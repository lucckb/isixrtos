#include <lwip/opt.h>
#include <lwip/def.h>
#include <lwip/mem.h>
#include <lwip/pbuf.h>
#include <lwip/sys.h>
#include <lwip/stats.h>
#include <lwip/snmp.h>
#include <lwip/tcpip.h>
#include <netif/etharp.h>
#include <netif/ppp_oe.h>
#include <stm32_eth.h>
#include <string.h>
#include <stdbool.h>
#include <isix.h>
#include <stm32system.h>
#include <stm32exti.h>
#include <stm32bitbang.h>
#include <eth/ethernetif.h>
#include <eth/phy.h>
#include "ethernetif_prv.h"

enum { ETH_RXBUFNB = 4 };
enum { ETH_TXBUFNB = 16 };


struct drv_rx_buff_s
{
	struct pbuf *pbuf;		//Receive pbuf
	bool used;				//Buffer is used
};

//
//Transmit and receive descriptors
static ETH_DMADESCTypeDef  dma_rx_ring[ETH_RXBUFNB];
static ETH_DMADESCTypeDef  dma_tx_ring[ETH_TXBUFNB];

//Global buffers for trx frames
static struct pbuf* 		tx_buff[ETH_TXBUFNB];
static struct drv_rx_buff_s rx_buff[ETH_RXBUFNB];

//Ring descriptor identifier
static volatile size_t dma_tx_idx;
static volatile size_t dma_rx_idx;

//Lock semaphore for the driver
static ossem_t netif_sem;

//TCPIP task event type
static unsigned ethif_events;

//Net interface copy task
static ostask_t netif_task_id;
//

//define phy ethernet driver pointer
define_phy_driver( ETH_PHY_DRIVER_NAME );


//Ethernet if task event
enum ethif_events_e
{
	ETHIF_EVENT_PHY_BIT,
	ETHIF_EVENT_EMAC_RX_BIT,
	ETHIF_EVENT_EMAC_TX_BIT
};


//
/**
  * @brief  Write to a PHY register
  * @param PHYAddress: PHY device address, is the index of one of supported 32 PHY devices.
  *   This parameter can be one of the following values: 0,..,31
  * @param PHYReg: PHY register address, is the index of one of the 32 PHY register.
  *   This parameter can be one of the following values:
  *     @arg PHY_BCR    : Tranceiver Control Register
  *     @arg More PHY register could be written depending on the used PHY
  * @param  PHYValue: the value to write
  * @retval ETH_ERROR: in case of timeout
  *         ETH_SUCCESS: for correct write
  */
int _ethernetif_write_phy_register_(uint16_t phy_addr, uint16_t phy_reg, uint16_t phy_value)
{
  static const uint32_t MACMIIAR_CR_MASK =0xFFFFFFE3;
	/* Get the ETHERNET MACMIIAR value */
  uint32_t tmpreg = ETH->MACMIIAR;
  /* Keep only the CSR Clock Range CR[2:0] bits value */
  tmpreg &= ~MACMIIAR_CR_MASK;
  /* Prepare the MII register address value */
  tmpreg |=(((uint32_t)phy_addr<<11) & ETH_MACMIIAR_PA); /* Set the PHY device address */
  tmpreg |=(((uint32_t)phy_reg<<6) 	 & ETH_MACMIIAR_MR);      /* Set the PHY register address */
  tmpreg |= ETH_MACMIIAR_MW;                               /* Set the write mode */
  tmpreg |= ETH_MACMIIAR_MB;                               /* Set the MII Busy bit */
  /* Give the value to the MII data register */
  ETH->MACMIIDR = phy_value;
  /* Write the result value into the MII Address register */
  ETH->MACMIIAR = tmpreg;
  /* Check for the Busy flag */
  for(int tout=0; tout<10000; tout++)
  {
    if( !(ETH->MACMIIAR & ETH_MACMIIAR_MB) )
    {
    	return ERR_OK;
    }
    isix_yield();
  }
  /* Return failed */
  return ERR_IF;
}
//
/**
  * @brief  Read a PHY register
  * @param PHYAddress: PHY device address, is the index of one of supported 32 PHY devices.
  *   This parameter can be one of the following values: 0,..,31
  * @param PHYReg: PHY register address, is the index of one of the 32 PHY register.
  *   This parameter can be one of the following values:
  *     @arg PHY_BCR: Tranceiver Basic Control Register
  *     @arg PHY_BSR: Tranceiver Basic Status Register
  *     @arg PHY_SR : Tranceiver Status Register
  *     @arg More PHY register could be read depending on the used PHY
  * @retval ETH_ERROR: in case of timeout
  *         MAC MIIDR register value: Data read from the selected PHY register (correct read )
  */
int _ethernetif_read_phy_register_(uint16_t phy_address, uint16_t phy_reg )
{
  static const uint32_t MACMIIAR_CR_MASK = 0xFFFFFFE3;
  /* Get the ETHERNET MACMIIAR value */
  uint32_t tmpreg = ETH->MACMIIAR;
  /* Keep only the CSR Clock Range CR[2:0] bits value */
  tmpreg &= ~MACMIIAR_CR_MASK;
  /* Prepare the MII address register value */
  tmpreg |=(((uint32_t)phy_address<<11) & ETH_MACMIIAR_PA); /* Set the PHY device address */
  tmpreg |=(((uint32_t)phy_reg<<6) & ETH_MACMIIAR_MR);      /* Set the PHY register address */
  tmpreg &= ~ETH_MACMIIAR_MW;                              /* Set the read mode */
  tmpreg |= ETH_MACMIIAR_MB;                               /* Set the MII Busy bit */
  /* Write the result value into the MII Address register */
  ETH->MACMIIAR = tmpreg;
  /* Check for the Busy flag */
  for(int tout=0; tout<10000; tout++)
  {
    if( !(ETH->MACMIIAR & ETH_MACMIIAR_MB) )
    {
    	return ETH->MACMIIDR;
    }
    isix_yield();
  }
  return ERR_IF;
}


//
/**
  * @brief  Clears the ETHERNETs DMA IT pending bit.
  * @param  ETH_DMA_IT: specifies the interrupt pending bit to clear.
  *   This parameter can be any combination of the following values:
  *     @arg ETH_DMA_IT_NIS : Normal interrupt summary
  *     @arg ETH_DMA_IT_AIS : Abnormal interrupt summary
  *     @arg ETH_DMA_IT_ER  : Early receive interrupt
  *     @arg ETH_DMA_IT_FBE : Fatal bus error interrupt
  *     @arg ETH_DMA_IT_ETI : Early transmit interrupt
  *     @arg ETH_DMA_IT_RWT : Receive watchdog timeout interrupt
  *     @arg ETH_DMA_IT_RPS : Receive process stopped interrupt
  *     @arg ETH_DMA_IT_RBU : Receive buffer unavailable interrupt
  *     @arg ETH_DMA_IT_R   : Receive interrupt
  *     @arg ETH_DMA_IT_TU  : Transmit Underflow interrupt
  *     @arg ETH_DMA_IT_RO  : Receive Overflow interrupt
  *     @arg ETH_DMA_IT_TJT : Transmit jabber timeout interrupt
  *     @arg ETH_DMA_IT_TBU : Transmit buffer unavailable interrupt
  *     @arg ETH_DMA_IT_TPS : Transmit process stopped interrupt
  *     @arg ETH_DMA_IT_T   : Transmit interrupt
  * @retval None
  */
static inline void eth_dma_clear_it_pending_bit(uint32_t ETH_DMA_IT)
{

  /* Clear the selected ETHERNET DMA IT */
  ETH->DMASR = (uint32_t) ETH_DMA_IT;
}
//
/**
  * @brief  Enables or disables the MAC transmission.
  * @param  NewState: new state of the MAC transmission.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void eth_mac_transmission_cmd( bool enable )
{
  if ( enable )
  {
    /* Enable the MAC transmission */
    ETH->MACCR |= ETH_MACCR_TE;
  }
  else
  {
    /* Disable the MAC transmission */
    ETH->MACCR &= ~ETH_MACCR_TE;
  }
}
//
/**
  * @brief  Clears the ETHERNET transmit FIFO.
  * @param  None
  * @retval None
  */
static inline void eth_flush_transmit_fifo(void)
{
  /* Set the Flush Transmit FIFO bit */
  ETH->DMAOMR |= ETH_DMAOMR_FTF;
}
//
/**
  * @brief  Enables or disables the MAC reception.
  * @param  NewState: new state of the MAC reception.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void eth_mac_reception_cmd( bool enable )
{
  if ( enable )
  {
    /* Enable the MAC reception */
    ETH->MACCR |= ETH_MACCR_RE;
  }
  else
  {
    /* Disable the MAC reception */
    ETH->MACCR &= ~ETH_MACCR_RE;
  }
}
//
/**
  * @brief  Enables or disables the DMA transmission.
  * @param  NewState: new state of the DMA transmission.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void eth_dma_transmission_cmd( bool enable )
{
  if ( enable )
  {
    /* Enable the DMA transmission */
    ETH->DMAOMR |= ETH_DMAOMR_ST;
  }
  else
  {
    /* Disable the DMA transmission */
    ETH->DMAOMR &= ~ETH_DMAOMR_ST;
  }
}
//
/**
  * @brief  Enables or disables the DMA reception.
  * @param  NewState: new state of the DMA reception.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void eth_dma_reception_cmd( bool enable )
{
  if ( enable )
  {
    /* Enable the DMA reception */
    ETH->DMAOMR |= ETH_DMAOMR_SR;
  }
  else
  {
    /* Disable the DMA reception */
    ETH->DMAOMR &= ~ETH_DMAOMR_SR;
  }
}
//
/**
  * @brief  Enables ENET MAC and DMA reception/transmission
  * @param  None
  * @retval None
  */
static inline void eth_start(void)
{
  /* Enable transmit state machine of the MAC for transmission on the MII */
  eth_mac_transmission_cmd(true);
  /* Flush Transmit FIFO */
  eth_flush_transmit_fifo();
  /* Enable receive state machine of the MAC for reception from the MII */
  eth_mac_reception_cmd(true);

  /* Start DMA transmission */
  eth_dma_transmission_cmd(true);
  /* Start DMA reception */
  eth_dma_reception_cmd(true);
}

/**
  * @brief  Enables or disables the specified ETHERNET DMA interrupts.
  * @param  ETH_DMA_IT: specifies the ETHERNET DMA interrupt sources to be
  *   enabled or disabled.
  *   This parameter can be any combination of the following values:
  *     @arg ETH_DMA_IT_NIS : Normal interrupt summary
  *     @arg ETH_DMA_IT_AIS : Abnormal interrupt summary
  *     @arg ETH_DMA_IT_ER  : Early receive interrupt
  *     @arg ETH_DMA_IT_FBE : Fatal bus error interrupt
  *     @arg ETH_DMA_IT_ET  : Early transmit interrupt
  *     @arg ETH_DMA_IT_RWT : Receive watchdog timeout interrupt
  *     @arg ETH_DMA_IT_RPS : Receive process stopped interrupt
  *     @arg ETH_DMA_IT_RBU : Receive buffer unavailable interrupt
  *     @arg ETH_DMA_IT_R   : Receive interrupt
  *     @arg ETH_DMA_IT_TU  : Underflow interrupt
  *     @arg ETH_DMA_IT_RO  : Overflow interrupt
  *     @arg ETH_DMA_IT_TJT : Transmit jabber timeout interrupt
  *     @arg ETH_DMA_IT_TBU : Transmit buffer unavailable interrupt
  *     @arg ETH_DMA_IT_TPS : Transmit process stopped interrupt
  *     @arg ETH_DMA_IT_T   : Transmit interrupt
te  * @param  NewState: new state of the specified ETHERNET DMA interrupts.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void eth_dma_it_config(uint32_t ETH_DMA_IT, bool enable )
{
  if ( enable )
  {
    /* Enable the selected ETHERNET DMA interrupts */
    ETH->DMAIER |= ETH_DMA_IT;
  }
  else
  {
    /* Disable the selected ETHERNET DMA interrupts */
    ETH->DMAIER &=(~(uint32_t)ETH_DMA_IT);
  }
}
//
/**
  * @brief  Checks whether the specified ETHERNET DMA interrupt has occured or not.
  * @param  ETH_DMA_IT: specifies the interrupt source to check.
  *   This parameter can be one of the following values:
  *     @arg ETH_DMA_IT_TST : Time-stamp trigger interrupt
  *     @arg ETH_DMA_IT_PMT : PMT interrupt
  *     @arg ETH_DMA_IT_MMC : MMC interrupt
  *     @arg ETH_DMA_IT_NIS : Normal interrupt summary
  *     @arg ETH_DMA_IT_AIS : Abnormal interrupt summary
  *     @arg ETH_DMA_IT_ER  : Early receive interrupt
  *     @arg ETH_DMA_IT_FBE : Fatal bus error interrupt
  *     @arg ETH_DMA_IT_ET  : Early transmit interrupt
  *     @arg ETH_DMA_IT_RWT : Receive watchdog timeout interrupt
  *     @arg ETH_DMA_IT_RPS : Receive process stopped interrupt
  *     @arg ETH_DMA_IT_RBU : Receive buffer unavailable interrupt
  *     @arg ETH_DMA_IT_R   : Receive interrupt
  *     @arg ETH_DMA_IT_TU  : Underflow interrupt
  *     @arg ETH_DMA_IT_RO  : Overflow interrupt
  *     @arg ETH_DMA_IT_TJT : Transmit jabber timeout interrupt
  *     @arg ETH_DMA_IT_TBU : Transmit buffer unavailable interrupt
  *     @arg ETH_DMA_IT_TPS : Transmit process stopped interrupt
  *     @arg ETH_DMA_IT_T   : Transmit interrupt
  * @retval The new state of ETH_DMA_IT (SET or RESET).
  */
static inline bool eth_get_dma_it_status(uint32_t ETH_DMA_IT)
{
  return (ETH->DMASR & ETH_DMA_IT)?true:false;
}

//
/**
  * @brief  Returns the specified DMA Rx Desc frame length.
  * @param  DMARxDesc: pointer on a DMA Rx descriptor
  * @retval The Rx descriptor received frame length.
  */
static inline uint32_t eth_get_dma_rx_desc_frame_length(ETH_DMADESCTypeDef *DMARxDesc)
{
  enum  { ETH_DMARXDESC_FRAME_LENGTHSHIFT = 16 };
  /* Return the Receive descriptor frame length */
  return ((DMARxDesc->Status & ETH_DMARxDesc_FL) >> ETH_DMARXDESC_FRAME_LENGTHSHIFT);
}

//
static void eth_mac_address_config(uint32_t MacAddr, const uint8_t *Addr)
{
  static const uint32_t ETH_MAC_ADDR_HBASE  = (ETH_MAC_BASE + 0x40);  /* ETHERNET MAC address high offset */
  static const uint32_t ETH_MAC_ADDR_LBASE  =  (ETH_MAC_BASE + 0x44);  /* ETHERNET MAC address low offset */
  /* Check the parameters */
  /* Calculate the selectecd MAC address high register */
  uint32_t tmpreg = ((uint32_t)Addr[5] << 8) | (uint32_t)Addr[4];
  /* Load the selectecd MAC address high register */
  (*(__IO uint32_t *) (ETH_MAC_ADDR_HBASE + MacAddr)) = tmpreg;
  /* Calculate the selectecd MAC address low register */
  tmpreg = ((uint32_t)Addr[3] << 24) | ((uint32_t)Addr[2] << 16) | ((uint32_t)Addr[1] << 8) | Addr[0];

  /* Load the selectecd MAC address low register */
  (*(__IO uint32_t *) (ETH_MAC_ADDR_LBASE + MacAddr)) = tmpreg;
}

//
static int eth_init_0_no_autonegotiate(uint16_t phy_addr, uint32_t hclk )
{
	static const uint32_t MACMIIAR_CR_MASK  = 0xFFFFFFE3;
	/* Get the ETHERNET MACMIIAR value */
	uint32_t tmpreg = ETH->MACMIIAR;
	/* Clear CSR Clock Range CR[2:0] bits */
	tmpreg &= MACMIIAR_CR_MASK;
	/* Set CR bits depending on hclk value */
	if((hclk >= 20000000)&&(hclk < 35000000))
	{
	   /* CSR Clock Range between 20-35 MHz */
	   tmpreg |= (uint32_t)ETH_MACMIIAR_CR_Div16;
	}
	else if((hclk >= 35000000)&&(hclk < 60000000))
	{
	   /* CSR Clock Range between 35-60 MHz */
	   tmpreg |= (uint32_t)ETH_MACMIIAR_CR_Div26;
	}
	else /* ((hclk >= 60000000)&&(hclk <= 72000000)) */
	{
	  /* CSR Clock Range between 60-72 MHz */
	  tmpreg |= (uint32_t)ETH_MACMIIAR_CR_Div42;
	}
	/* Write to ETHERNET MAC MIIAR: Configure the ETHERNET CSR Clock Range */
	ETH->MACMIIAR = (uint32_t)tmpreg;
	/* PHY initialization and configuration */
	/* Put the PHY in reset mode */
	if( phy->reset( phy_addr ) )
	{
	  /* Return ERROR in case of write timeout */
	  return ERR_IF;
	}

	/* Delay to assure PHY reset */
	isix_wait_ms( 50 );
	return ERR_OK;
}

static inline void eth_init_1_mac_cr( uint32_t watchdog, uint32_t jabber, uint32_t inter_frame_gap,
		uint32_t carrier_sense, uint32_t speed,  uint32_t receive_own, uint32_t loopback_mode,
		uint32_t mode, uint32_t checksum_offload, uint32_t retry_transmission, uint32_t automatic_pad_crc_strip,
		uint32_t back_off_limit, uint32_t defferal_check )
{
	  static const uint32_t MACCR_CLEAR_MASK  = 0xFF20810F;
	  /* Get the ETHERNET MACCR value */
	  uint32_t tmpreg = ETH->MACCR;
	  /* Clear WD, PCE, PS, TE and RE bits */
	  tmpreg &= MACCR_CLEAR_MASK;
	  /* Set the WD bit according to ETH_Watchdog value */
	  /* Set the JD: bit according to ETH_Jabber value */
	  /* Set the IFG bit according to ETH_InterFrameGap value */
	  /* Set the DCRS bit according to ETH_CarrierSense value */
	  /* Set the FES bit according to ETH_Speed value */
	  /* Set the DO bit according to ETH_ReceiveOwn value */
	  /* Set the LM bit according to ETH_LoopbackMode value */
	  /* Set the DM bit according to ETH_Mode value */
	  /* Set the IPC bit according to ETH_ChecksumOffload value */
	  /* Set the DR bit according to ETH_RetryTransmission value */
	  /* Set the ACS bit according to ETH_AutomaticPadCRCStrip value */
	  /* Set the BL bit according to ETH_BackOffLimit value */
	  /* Set the DC bit according to ETH_DeferralCheck value */
	  tmpreg |= watchdog | jabber | inter_frame_gap | carrier_sense |
			    speed | receive_own | loopback_mode | mode |  checksum_offload |
			    retry_transmission | automatic_pad_crc_strip | back_off_limit | defferal_check;
	  /* Write to ETHERNET MACCR */
	  ETH->MACCR = (uint32_t)tmpreg;
}

/* SET MAC speed according to the parameters */
static inline void eth_cr_set_speed_mode( bool is_100mbps, bool is_full_duplex )
{
	uint32_t tmpreg = ETH->MACCR;
	if( is_100mbps ) tmpreg |= ETH_Speed_100M;
	else tmpreg &= ~ETH_Speed_100M;
	if( is_full_duplex ) tmpreg |= ETH_Mode_FullDuplex;
	else  tmpreg &= ~ETH_Mode_FullDuplex;
	ETH->MACCR = tmpreg;
}

static inline void eth_init_2_mac_ffr( uint32_t receive_all, uint32_t source_addr_filter,
		uint32_t pass_control_frames, uint32_t broadcast_frames_reception,
		uint32_t destination_addr_filter, uint32_t promiscuous_mode,
		uint32_t multicast_frames_filter, uint32_t unicast_frames_filter )
{
	  /* Set the RA bit according to ETH_ReceiveAll value */
	  /* Set the SAF and SAIF bits according to ETH_SourceAddrFilter value */
	  /* Set the PCF bit according to ETH_PassControlFrames value */
	  /* Set the DBF bit according to ETH_BroadcastFramesReception value */
	  /* Set the DAIF bit according to ETH_DestinationAddrFilter value */
	  /* Set the PR bit according to ETH_PromiscuousMode value */
	  /* Set the PM, HMC and HPF bits according to ETH_MulticastFramesFilter value */
	  /* Set the HUC and HPF bits according to ETH_UnicastFramesFilter value */
	  /* Write to ETHERNET MACFFR */
	  ETH->MACFFR = receive_all | source_addr_filter | pass_control_frames |
	      broadcast_frames_reception | destination_addr_filter | promiscuous_mode |
	      multicast_frames_filter | unicast_frames_filter;
}

static inline void eth_init_3_hash_table_fltr(uint32_t table_hi, uint32_t table_lo )
{
	/* Write to ETHERNET MACHTHR */
	 ETH->MACHTHR = table_hi;
	 /* Write to ETHERNET MACHTLR */
	 ETH->MACHTLR = table_lo;
}


static inline void eth_init_4_mac_fcr(uint32_t pause_time, uint32_t zero_quanta_pause,
		uint32_t pause_low_treshold, uint32_t unicast_pause_frame_detect,
		uint32_t receive_flow_control, uint32_t transmit_flow_control )
{
	  static const uint32_t MACFCR_CLEAR_MASK = 0x0000FF41;
	 /* Get the ETHERNET MACFCR value */
	  uint32_t tmpreg = ETH->MACFCR;
	  /* Clear xx bits */
	  tmpreg &= MACFCR_CLEAR_MASK;

	  /* Set the PT bit according to ETH_PauseTime value */
	  /* Set the DZPQ bit according to ETH_ZeroQuantaPause value */
	  /* Set the PLT bit according to ETH_PauseLowThreshold value */
	  /* Set the UP bit according to ETH_UnicastPauseFrameDetect value */
	  /* Set the RFE bit according to ETH_ReceiveFlowControl value */
	  /* Set the TFE bit according to ETH_TransmitFlowControl value */
	  tmpreg |= (pause_time << 16) |  zero_quanta_pause | pause_low_treshold |
	            unicast_pause_frame_detect | receive_flow_control | transmit_flow_control;
	  /* Write to ETHERNET MACFCR */
	  ETH->MACFCR = tmpreg;
}

static inline void eth_init_5_mac_vlantr(uint32_t vlan_tag_comparision,
		uint32_t vlan_tag_identifier)
{
	  /* Set the VL bit according to ETH_VLANTagIdentifier value */
	  ETH->MACVLANTR = vlan_tag_comparision | vlan_tag_identifier;
}

static inline void eth_init_6_dma_omr( uint32_t drop_tcpip_checksum_error_frame,
		uint32_t receive_store_forward, uint32_t flush_received_frame,
		uint32_t transmit_store_forward, uint32_t transmit_treshold_control,
		uint32_t forward_error_frames, uint32_t forward_undersized_good_frames,
		uint32_t receive_treshold_control, uint32_t second_frame_operate )
{
	  static const uint32_t DMAOMR_CLEAR_MASK = 0xF8DE3F23;
	  uint32_t tmpreg = ETH->DMAOMR;
	  /* Clear xx bits */
	  tmpreg &= DMAOMR_CLEAR_MASK;

	  /* Set the DT bit according to ETH_DropTCPIPChecksumErrorFrame value */
	  /* Set the RSF bit according to ETH_ReceiveStoreForward value */
	  /* Set the DFF bit according to ETH_FlushReceivedFrame value */
	  /* Set the TSF bit according to ETH_TransmitStoreForward value */
	  /* Set the TTC bit according to ETH_TransmitThresholdControl value */
	  /* Set the FEF bit according to ETH_ForwardErrorFrames value */
	  /* Set the FUF bit according to ETH_ForwardUndersizedGoodFrames value */
	  /* Set the RTC bit according to ETH_ReceiveThresholdControl value */
	  /* Set the OSF bit according to ETH_SecondFrameOperate value */
	  tmpreg |= drop_tcpip_checksum_error_frame | receive_store_forward |
	     flush_received_frame | transmit_store_forward | transmit_treshold_control |
	     forward_error_frames | forward_undersized_good_frames |
	     receive_treshold_control | second_frame_operate;
	  /* Write to ETHERNET DMAOMR */
	  ETH->DMAOMR = tmpreg;
}


static inline void eth_init_7_dma_bmr(uint32_t address_aligned_beats , uint32_t fixed_burst,
		uint32_t rx_dma_burst_length, uint32_t tx_dma_burst_length,
		uint32_t descriptor_skip_length, uint32_t dma_arbitration )
{
	/* Set the AAL bit according to ETH_AddressAlignedBeats value */
	  /* Set the FB bit according to ETH_FixedBurst value */
	  /* Set the RPBL and 4*PBL bits according to ETH_RxDMABurstLength value */
	  /* Set the PBL and 4*PBL bits according to ETH_TxDMABurstLength value */
	  /* Set the DSL bit according to ETH_DesciptorSkipLength value */
	  /* Set the PR and DA bits according to ETH_DMAArbitration value */
	  ETH->DMABMR = address_aligned_beats | fixed_burst |
	       rx_dma_burst_length | /* !! if 4xPBL is selected for Tx or Rx it is applied for the other */
	       tx_dma_burst_length | (descriptor_skip_length << 2) |
	       dma_arbitration | ETH_DMABMR_USP; /* Enable use of separate PBL for Rx and Tx */
}

//
/* Setup DMA RX buffer */
static void dma_rx_buffer_set(ETH_DMADESCTypeDef *rx_desc, struct pbuf *p)
{
  /* We assume that buffer is created unsing single segment
   */
  rx_desc->ControlBufferSize &= ~(ETH_DMARxDesc_RBS1 |
                                 ETH_DMARxDesc_RBS2);
  rx_desc->ControlBufferSize |= p->len & ETH_DMARxDesc_RBS1;
  rx_desc->Buffer1Addr = (uint32_t)p->payload;
  rx_desc->Buffer2NextDescAddr = 0;
  rx_desc->Status = ETH_DMARxDesc_OWN;
}

//
/**
  * @brief  Resets all MAC subsystem internal registers and logic.
  * @param  None
  * @retval None
  */
static inline void eth_software_reset(void)
{
  /* Set the SWR bit: resets all MAC subsystem internal registers and logic */
  /* After reset all the registers holds their respective reset values */
  ETH->DMABMR |= ETH_DMABMR_SR;
}

//
/**
  * @brief  Checks whether the ETHERNET software reset bit is set or not.
  * @param  None
  * @retval The new state of DMA Bus Mode register SR bit (SET or RESET).
  */
static inline bool eth_get_software_reset_status(void)
{
  return (ETH->DMABMR & ETH_DMABMR_SR)?(true):(false);
}

//
/** Reallocat receive buffers */
static int realloc_rx_pbufs(void)
{
  static int idx = 0;

  /* check buffers after last returned to the DMA */
  while ( rx_buff[idx].used )
  {
    if ( rx_buff[idx].pbuf == NULL )
    {
      /*
       *	Allocate PBUF RX buffer PBUF_POOL as single part
       */
    	rx_buff[idx].pbuf = pbuf_alloc(PBUF_RAW, PBUF_POOL_BUFSIZE,PBUF_POOL );
    	if ( rx_buff[idx].pbuf == NULL )
    		return ERR_MEM;
    }
    rx_buff[idx].used = false;
    dma_rx_buffer_set(&dma_rx_ring[idx], rx_buff[idx].pbuf);
    if (dma_rx_ring[idx].ControlBufferSize & ETH_DMARxDesc_RER)
      idx = 0;
    else
      ++idx;
  }
  return ERR_OK;
}

//
/* Initialize allocation of RX buffers in the ethernet driver */
static int alloc_rx_pbufs(void)
{
  for (int i = 0; i < ETH_RXBUFNB; ++i)
  {
    rx_buff[i].used = false;
    /*
     *  Initial allocation of rx buff
     */
    rx_buff[i].pbuf = pbuf_alloc(PBUF_RAW, PBUF_POOL_BUFSIZE, PBUF_POOL);
    if (rx_buff[i].pbuf == NULL)
    {
      for (int j = i - 1; j >= 0; --j)
        pbuf_free(rx_buff[i].pbuf);
      return ERR_MEM;
    }
  }
  return ERR_OK;
}
//
/* Free Tx buffers */
static void free_tx_pbufs(void)
{
  for (int i = 0; i < ETH_TXBUFNB; ++i)
  {
    if (tx_buff[i] && (dma_tx_ring[i].Status & ETH_DMATxDesc_OWN) == 0)
    {
      //if (DMAtxRing[i].Status & ETH_DMATxDesc_ES)
       // ((struct ethnetif *)netif->state)->TX_errors++;
      pbuf_free(tx_buff[i]);
      tx_buff[i] = NULL;
    }
  }
}
//
//DMA pbuf set
static struct pbuf* dma_tx_pbuf_set(ETH_DMADESCTypeDef *tx_desc, struct pbuf *p, bool fs)
{

  tx_desc->ControlBufferSize = (uint32_t)p->len & ETH_DMATxDesc_TBS1;
  tx_desc->Buffer1Addr = (uint32_t)p->payload;
  if (p->next)
  {
    p = p->next;
    tx_desc->ControlBufferSize |= ((uint32_t)p->len << 16) & ETH_DMATxDesc_TBS2;
    tx_desc->Buffer2NextDescAddr = (uint32_t)(p->payload);
  }
  else
      tx_desc->Buffer2NextDescAddr = 0;

  tx_desc->Status |= ETH_DMATxDesc_IC | (fs?ETH_DMATxDesc_FS:0) |
          (p->next?0:ETH_DMATxDesc_LS) | ETH_DMATxDesc_OWN;
  return p->next;
}
//
/* Forward declarations. */
static err_t  ethernetif_input(struct netif *netif);

//
/*
  * @brief  Called when a frame is received
  * @param  None
  * @retval None
  */
void eth_isr_vector(void) __attribute__((__interrupt__));
void eth_isr_vector(void)
{
	if( eth_get_dma_it_status(ETH_DMA_IT_R ) )
	{
		/* Signal thread driver */
		setBit_BB( &ethif_events, ETHIF_EVENT_EMAC_RX_BIT );
		isix_sem_signal_isr( netif_sem );
		/* Clear the Eth DMA Rx IT pending bits */
		eth_dma_clear_it_pending_bit(ETH_DMA_IT_R);
	}
	if( eth_get_dma_it_status(ETH_DMA_IT_T ) )
	{
		/* Signal thread driver */
		setBit_BB( &ethif_events, ETHIF_EVENT_EMAC_TX_BIT );
		isix_sem_signal_isr( netif_sem );
		/* Clear the Eth DMA Rx IT pending bits */
		eth_dma_clear_it_pending_bit(ETH_DMA_IT_T);
	}
	eth_dma_clear_it_pending_bit(ETH_DMA_IT_NIS);
}

//
/**
 *  Interrupt handler for MAC dervice
 */
#if PHY_INT_USE_INTERRUPT
#define DO_EXTI_HANLDLER(x) void  __attribute__((__interrupt__)) exti ## x ## _isr_vector(void)
#define EXTI_HANDLER(x) DO_EXTI_HANLDLER( x )

EXTI_HANDLER(PHY_INT_EXTI_NUM)
{
	setBit_BB( &ethif_events, ETHIF_EVENT_PHY_BIT );
	isix_sem_signal_isr( netif_sem );
	exti_clear_it_pending_bit( PHY_INT_EXTI_LINENUM(PHY_INT_EXTI_NUM) );
}

#undef DO_EXTI_HANLDLER
#undef EXTI_HANDLER
#endif
//
//Check the ethernet link status
static void check_link_status( uint8_t addr, struct netif *netif )
{
	const int ls = phy->read_status(addr);
	if( ls & phy_stat_autoneg_compl )
	{
		eth_cr_set_speed_mode( (ls&phy_stat_100m_full)||(ls&phy_stat_100m_half),
				(ls&phy_stat_10m_full)||(ls&phy_stat_100m_full) );
		tcpip_callback_with_block( (tcpip_callback_fn)netif_set_link_up, netif, 0 );
	}
	else
	{
		tcpip_callback_with_block( (tcpip_callback_fn)netif_set_link_down, netif, 0 );
	}
}
//
/* Network interrupt bottom half Linux concept of bottom halfes */
static void  __attribute__((__noreturn__)) netif_task( void *ifc )
{
	/* Enable MAC and DMA transmission and reception */
	eth_start();
	struct netif *netif = (struct netif*)(ifc);
#if !PHY_INT_USE_INTERRUPT
	ostick_t link_last_call_time = 0;
#endif
	for(;;)
	{
#if PHY_INT_USE_INTERRUPT
		const int rcode = isix_sem_wait( netif_sem, ISIX_TIME_INFINITE );
#else
		enum { LINK_CHECK_INTERVAL = 2000 };
		const int rcode = isix_sem_wait( netif_sem, isix_ms2tick(LINK_CHECK_INTERVAL) );
#endif
		if( rcode == ISIX_EOK )
		{
#if PHY_INT_USE_INTERRUPT
			if( getBit_BB(&ethif_events, ETHIF_EVENT_PHY_BIT) )
			{
				check_link_status( ETH_DRV_PHY_ADDR, netif );
			}
#endif
			//If TX int
			if( getBit_BB(&ethif_events, ETHIF_EVENT_EMAC_TX_BIT) )
			{
				free_tx_pbufs();
			}
			//If RX int
			if( getBit_BB(&ethif_events, ETHIF_EVENT_EMAC_RX_BIT) )
			while (!(dma_rx_ring[dma_rx_idx].Status & ETH_DMARxDesc_OWN))
			{
				ethernetif_input( netif );
				while( realloc_rx_pbufs() )
				{
					//If no more pbufs wait for  release it by stack
					//LWIP doesn't provide any signaling in this case
					isix_wait_ms( 10 );
				}
			}
			//Clear ethif events
			resetBitsAll_BB( &ethif_events );
		}
#if !PHY_INT_USE_INTERRUPT
		if( link_last_call_time - isix_get_jiffies() > LINK_CHECK_INTERVAL )
		{
			check_link_status( ETH_DRV_PHY_ADDR, netif );
			link_last_call_time = isix_get_jiffies();
		}
#endif
	}
}
//

/**
 * In this function, the hardware should be initialized.
 * Called from stm32_emac_if_init_callback().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static int low_level_init(struct netif *netif)
{

  if( !phy ) {
	  return ERR_IF;
  }
  dma_tx_idx = 0;
  for (int i = 0; i < ETH_TXBUFNB; ++i)
  {
#ifdef ISIX_TCPIPLIB_CHECKSUM_BY_HARDWARE
    dma_tx_ring[i].Status = ETH_DMATxDesc_ChecksumTCPUDPICMPFull;
#else
    dma_tx_ring[i].Status = 0;
#endif
    dma_tx_ring[i].ControlBufferSize = 0;
    dma_tx_ring[i].Buffer1Addr = 0;
    dma_tx_ring[i].Buffer2NextDescAddr = 0;
  }
  dma_tx_ring[ETH_TXBUFNB - 1].Status |= ETH_DMATxDesc_TER;
  ETH->DMATDLAR = (uint32_t)dma_tx_ring;

  /*-------------> Initialize Rx Descriptors list: Chain Mode  */
  for (int i = 0; i < ETH_RXBUFNB; ++i)
  {
     /* Zeruj bit ETH_DMARxDesc_DIC, aby uaktywnić przerwanie.
       DMArxRing[i].ControlBufferSize &= ~ETH_DMARxDesc_DIC; */
     dma_rx_ring[i].ControlBufferSize = 0;
     dma_rx_buffer_set(&dma_rx_ring[i], rx_buff[i].pbuf);
  }
  dma_rx_ring[ETH_RXBUFNB - 1].ControlBufferSize |= ETH_DMARxDesc_RER;
  ETH->DMARDLAR = (uint32_t)dma_rx_ring;

  /* Enable the Ethernet Rx Tx Interrupt */
  eth_dma_it_config(ETH_DMA_IT_NIS | ETH_DMA_IT_R | ETH_DMA_IT_T, true);
  /* Enable interrupt in NVIC */
  nvic_irq_enable( ETH_IRQn, true );
#if PHY_INT_USE_INTERRUPT
  gpio_exti_line_config( PHY_INT_EXTI_PORT_SOURCE(PHY_INT_GPIO_PORT), PHY_INT_EXTI_PIN_SOURCE(PHY_INT_EXTI_NUM) );
  exti_init( PHY_INT_EXTI_LINENUM(PHY_INT_EXTI_NUM), EXTI_Mode_Interrupt, EXTI_Trigger_Falling, true );
  exti_clear_it_pending_bit( PHY_INT_EXTI_LINENUM(PHY_INT_EXTI_NUM) );
  nvic_irq_enable( PHY_INT_EXTI_LINE_IRQ_N, true );
  //eth_read_phy_register( ETH_DRV_PHY_ADDR, PHYMICSR );
  phy->read_status( ETH_DRV_PHY_ADDR );
#endif
  /* Create task and sem */
  enum { C_netif_task_stack_size = 384 };
  netif_sem = isix_sem_create_limited( NULL, 0, 1 );
  if(!netif_sem)
  {
	  return ERR_MEM;
  }
  netif_task_id = isix_task_create( netif_task, netif, C_netif_task_stack_size, ETH_DRV_ISIX_THREAD_PRIORITY , 0 );
  if(!netif_task_id)
  {
	  isix_sem_destroy(netif_sem);
	  return ERR_MEM;
  }
  /* Enable Auto-Negotiation */
  if(phy->config_speed(ETH_DRV_PHY_ADDR, phy_link_auto ) != ERR_OK)
  {
	  /* Return ERROR in case of write timeout */
	  isix_task_kill(netif_task_id);
	  isix_sem_destroy(netif_sem);
	  return ERR_IF;
  }
  return ERR_OK;
}
//
//Check if pbuf packet is DMA safe
static bool is_pbuf_dma_safe( const struct pbuf *p )
{
	enum {SRAM_BEGIN=0x20000000, SRAM_END=0x3fffffff };
	for ( ;p != NULL; p = p->next)
	{
		if(p->payload<(void*)SRAM_BEGIN || p->payload>(void*)SRAM_END )
		{
			return false;
		}
	}
	return true;
}
//
//Convert puf to req desc
static inline size_t pbuf_chains_to_dma_descs_num(int pbuf_chains)
{

	return pbuf_chains / 2 + pbuf_chains % 2;
}
//
/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become availale since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */

static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
	  (void)netif;
	  if (p == NULL)
	  {
	    return ERR_ARG;
	  }
	  const int req_descs = pbuf_chains_to_dma_descs_num( pbuf_clen( p ) );
	  for(int s=0,idx=dma_tx_idx; s<req_descs; s++,idx++)
	  {
	     if( tx_buff[idx] || dma_tx_ring[idx].Status & ETH_DMATxDesc_OWN)
	     {
	    	 return ERR_IF;
	     }
	     if(dma_tx_ring[idx].Status & ETH_DMATxDesc_TER)
	         idx = 0;
	  }
	  /* This is the flash safe version. DMA can access only RAM memory
	   * so if the PBUF packet is in ROM memory, PBUF must be copied to
	   * the RAM
	   */
	  if( is_pbuf_dma_safe(p) )
	  {
		  pbuf_ref(p);
	  }
	  else
	  {
		  struct pbuf* np = pbuf_alloc(PBUF_RAW, p->tot_len, PBUF_RAM);
		  pbuf_copy( np, p );
		  p = np;
	  }
	  for( bool first=true; p; first=false )
	  {
	      tx_buff[dma_tx_idx] = p;
	      p = dma_tx_pbuf_set(&dma_tx_ring[dma_tx_idx], p, first );
	      /* Zmień deskryptor nadawczy DMA na następny w pierścieniu. */
	      if (dma_tx_ring[dma_tx_idx].Status & ETH_DMATxDesc_TER)
	         dma_tx_idx = 0;
	      else
	         ++dma_tx_idx;
	  }
	  /* Start trasmission again if it is interrupted */
	 if (ETH->DMASR & ETH_DMASR_TBUS)
	 {
	     ETH->DMASR = ETH_DMASR_TBUS;
	     ETH->DMATPDR = 0;
	 }
	 return ERR_OK;
}

//
/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
static struct pbuf* low_level_input(struct netif *netif)
{
	(void)netif;
	struct pbuf *p = NULL;
	  uint32_t len;
	  /* ETH_DMARxDesc_ES == 0 oznacza, że nie wystąpił błąd.
	     ETH_DMARxDesc_LS == 1 && ETH_DMARxDesc_FS == 1 oznacza, że
	     cała ramka jest opisana pojedynczym deskryptorem. */
	  if ( !(dma_rx_ring[dma_rx_idx].Status & ETH_DMARxDesc_ES) &&
	       (dma_rx_ring[dma_rx_idx].Status & ETH_DMARxDesc_LS) &&
	        (dma_rx_ring[dma_rx_idx].Status & ETH_DMARxDesc_FS)
	      )
	  {
	    len = eth_get_dma_rx_desc_frame_length(&dma_rx_ring[dma_rx_idx]);
	    if (len >= ETH_HEADER + MIN_ETH_PAYLOAD + ETH_CRC)
	    {
	      len -= ETH_CRC;
	      /* Zakładamy, że rxBuffer[DMArxIdx] != NULL. */
	      p = rx_buff[dma_rx_idx].pbuf;
	      /* Dla buforów typu PBUF_POOL nie jest wykonywane kopiowane. */
	      pbuf_realloc(p, len);
	      /* Usuń bufor z kolejki odbiorczej. Bufor zostanie przekazany
	         bibliotece lwIP. */
	      rx_buff[dma_rx_idx].pbuf = NULL;
	    }
	  }
	  rx_buff[dma_rx_idx].used = true;

	  /* Zmień deskryptor odbiorczy DMA na następny w pierścieniu. */
	  if (dma_rx_ring[dma_rx_idx].ControlBufferSize & ETH_DMARxDesc_RER)
	    dma_rx_idx = 0;
	  else
	    ++dma_rx_idx;

	  /* Jeśli odbieranie DMA zostało wstrzymane, wznów je. */
	  if (ETH->DMASR & ETH_DMASR_RBUS)
	  {
	    ETH->DMASR = ETH_DMASR_RBUS;
	    ETH->DMARPDR = 0;
	  }
	  return p;
}

//
/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
err_t ethernetif_input(struct netif *netif)
{
  struct pbuf *p;

  /* move received packet into a new pbuf */
  p = low_level_input(netif);

  /* no packet could be read, silently ignore this */
  if (p == NULL) return ERR_MEM;

   err_t err = netif->input(p, netif);
   if (err != ERR_OK)
   {
     LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
     pbuf_free(p);
     p = NULL;
  }
  return err;
}

//
/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network iETHconfigureDMAnterface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t stm32_emac_if_init_callback(struct netif *netif)
{

 LWIP_ASSERT("netif != NULL", (netif != NULL));


#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

  /*
   * Initialize the snmp variables and counters inside the struct netif.
   * The last argument should be replaced with your link speed, in units
   * of bits per second.
   */
  NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, 100000000);

  /* Hold private data here */
  //netif->state = NULL
  netif->name[0] = 's';
  netif->name[1] = 't';
  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
  netif->output = etharp_output;
  netif->linkoutput = low_level_output;
  netif->mtu = MAX_ETH_PAYLOAD;
  /* hardware address length */
  netif->hwaddr_len  = ETHARP_HWADDR_LEN;

  /* initialize the hardware */
  if( low_level_init(netif) )
  {
	  return ERR_IF;
  }
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;
  return ERR_OK;
}


//
/**
  * @brief  Deinitializes the ETHERNET peripheral registers to their default reset values.
  * @param  None
  * @retval None
  */


//
/**
  * @brief  Configures the Ethernet Interface
  */
static int ethernet_init(uint32_t hclk, uint8_t phy_addr)
{
	if( !phy ) {
		return ERR_IF;
	}
  //Enable eth stuff
  _ethernetif_clock_setup_arch_( true );

  /* MII/RMII Media interface selection */
#if ETH_DRV_USE_RMII
	 _ethernetif_gpio_rmii_init_arch_();
#else
	 _ethernetif_gpio_mii_init_arch_();
#endif

  /* Reset ETHERNET on AHB Bus */
   _ethernetif_deinit_arch_();

  /* Software reset */
  eth_software_reset();

  /* Wait for software reset */
  while (eth_get_software_reset_status() ) {
	isix_wait_ms(10);
  }
  if( phy->probe( phy_addr, phy_link_auto ) ) {
	  return ERR_IF;
  }

  /* Configure Ethernet */
  if( eth_init_0_no_autonegotiate( phy_addr, hclk ) )
  {
	  return ERR_IF;
  }
  eth_init_1_mac_cr(ETH_Watchdog_Enable, ETH_Jabber_Enable, ETH_InterFrameGap_96Bit,
		  ETH_CarrierSense_Enable, ETH_Speed_10M, ETH_ReceiveOwn_Enable,
		  ETH_LoopbackMode_Disable, ETH_Mode_HalfDuplex,
#ifdef ISIX_TCPIPLIB_CHECKSUM_BY_HARDWARE
		  ETH_ChecksumOffload_Enable,
#else
		  ETH_ChecksumOffload_Disable,
#endif
		  ETH_RetryTransmission_Enable, ETH_AutomaticPadCRCStrip_Disable,
		  ETH_BackOffLimit_10, ETH_DeferralCheck_Disable
	);

  eth_init_2_mac_ffr( ETH_ReceiveAll_Disable, ETH_SourceAddrFilter_Disable,
		  ETH_PassControlFrames_BlockAll, ETH_BroadcastFramesReception_Enable,
		  ETH_DestinationAddrFilter_Normal, ETH_PromiscuousMode_Disable,
		  ETH_MulticastFramesFilter_Perfect, ETH_UnicastFramesFilter_Perfect );

  eth_init_3_hash_table_fltr( 0, 0 );

  eth_init_4_mac_fcr( 0, ETH_ZeroQuantaPause_Disable, ETH_PauseLowThreshold_Minus4,
		  ETH_UnicastPauseFrameDetect_Disable, ETH_ReceiveFlowControl_Enable, ETH_TransmitFlowControl_Enable );

  eth_init_5_mac_vlantr(ETH_VLANTagComparison_16Bit, 0 );

  eth_init_6_dma_omr( ETH_DropTCPIPChecksumErrorFrame_Enable, ETH_ReceiveStoreForward_Enable,
		  ETH_FlushReceivedFrame_Disable, ETH_TransmitStoreForward_Enable, ETH_TransmitThresholdControl_64Bytes,
		  ETH_ForwardErrorFrames_Disable, ETH_ForwardUndersizedGoodFrames_Disable,
		  ETH_ReceiveThresholdControl_64Bytes, ETH_SecondFrameOperate_Enable );

  eth_init_7_dma_bmr( ETH_AddressAlignedBeats_Enable, ETH_FixedBurst_Enable, ETH_RxDMABurstLength_32Beat,
		  ETH_TxDMABurstLength_32Beat, 0, ETH_DMAArbitration_RoundRobin_RxTx_2_1 );
  if( phy->config_intr( phy_addr, true ) ) {
	  return ERR_IF;
  }
  return ERR_OK;
}
//

/** Input packet handling */
struct netif* stm32_emac_netif_create( const uint8_t *hw_addr )
{

	_ethernetif_dma_setup_arch_();
	//Create NETIF interface
	if( ethernet_init( ETH_DRV_HCLK_HZ, ETH_DRV_PHY_ADDR ) )
	{
		return NULL;
	}
	struct netif* nifc = (struct netif*)mem_malloc(sizeof(struct netif));
	if (nifc == NULL)
	{
	    LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
	    return NULL;
	}
	memset(nifc, 0, sizeof(struct netif));
	eth_mac_address_config(ETH_MAC_Address0, hw_addr);
	/* set MAC hardware address length */
	nifc->hwaddr_len = ETHARP_HWADDR_LEN;
	/* set MAC hardware address */
	memcpy(nifc->hwaddr, hw_addr, ETHARP_HWADDR_LEN);
	if ( alloc_rx_pbufs() != ERR_OK )
	{
		mem_free( nifc );
	}
	//Set NVIC priority
	nvic_set_priority( ETH_IRQn ,ETH_DRV_IRQ_PRIO, ETH_DRV_IRQ_SUBPRIO );
	nvic_set_priority( PHY_INT_EXTI_LINE_IRQ_N, ETH_DRV_IRQ_PRIO, ETH_DRV_IRQ_SUBPRIO  );
	return nifc;
}
//
/* Destroy the netif stm32 interface */
int stm32_emac_netif_shutdown( struct netif *netif )
{
	if( !netif )
	{
		return ERR_MEM;
	}
	//Stop the hardware first
	/* Enable the Ethernet Rx Interrupt */
	eth_dma_it_config(ETH_DMA_IT_NIS | ETH_DMA_IT_R | ETH_DMA_IT_T, DISABLE);
	/* Enable interrupt in NVIC */
	nvic_irq_enable( ETH_IRQn, false );
#if PHY_INT_USE_INTERRUPT
	 //Disable only phy line NVIC IRQ can be shared with other pins
	 exti_init( PHY_INT_EXTI_LINENUM(PHY_INT_EXTI_NUM), EXTI_Mode_Interrupt, EXTI_Trigger_Falling, false );
#endif
	 int ret_code = ERR_OK;
	 if( netif_task_id )
	 {
		 isix_task_kill( netif_task_id );
		 netif_task_id = NULL;
	 }
	 else
	 {
		 ret_code =  ERR_IF;
	 }
	 if( netif_sem )
	 {
		 if( isix_sem_destroy( netif_sem ) )
		 {
			 ret_code = ERR_IF;
		 }
		 netif_sem = NULL;
	 }
	 //Reset the ethernet controler using RST signal
	 _ethernetif_deinit_arch_();
	 //Disable ETH clocks
	 _ethernetif_clock_setup_arch_(false);
	 return ret_code;
}
