#include <lwip/opt.h>
#include <lwip/def.h>
#include <lwip/mem.h>
#include <lwip/pbuf.h>
#include <lwip/sys.h>
#include <lwip/stats.h>
#include <lwip/snmp.h>
#include <netif/etharp.h>
#include <netif/ppp_oe.h>
#include <stm32_eth.h>
#include <string.h>
#include <isix.h>
#include <stm32system.h>
#include <stm32rcc.h>
#include "ethernetif.h"

/* ------------------------------------------------------------------ */
enum {  ETH_DMARxDesc_FrameLengthShift   = 16 };

/* ------------------------------------------------------------------ */
enum { ETH_ERROR = 0, ETH_SUCCESS = 1 };

/* ------------------------------------------------------------------ */

/* Forward declarations. */
static err_t  ethernetif_input(struct netif *netif);

enum { ETH_RXBUFNB = 4 };
enum { ETH_TXBUFNB = 2 };
/* ------------------------------------------------------------------ */

//static uint8_t MACaddr[6];
static ETH_DMADESCTypeDef  DMARxDscrTab[ETH_RXBUFNB], DMATxDscrTab[ETH_TXBUFNB];/* Ethernet Rx & Tx DMA Descriptors */
static uint8_t Rx_Buff[ETH_RXBUFNB][ETH_MAX_PACKET_SIZE], Tx_Buff[ETH_TXBUFNB][ETH_MAX_PACKET_SIZE];/* Ethernet buffers */

extern ETH_DMADESCTypeDef  *DMATxDescToSet;
extern ETH_DMADESCTypeDef  *DMARxDescToGet;

/* ------------------------------------------------------------------ */
typedef struct
{
	u32 length;
	u32 buffer;
	ETH_DMADESCTypeDef *descriptor;
}
FrameTypeDef;
/* ------------------------------------------------------------------ */

static FrameTypeDef ETH_RxPkt_ChainMode(void);
static u32 ETH_GetCurrentTxBuffer(void);
static u32 ETH_TxPkt_ChainMode(u16 FrameLength);

/* ------------------------------------------------------------------ */
//Lock semaphore for the driver
static sem_t *netif_sem;
//Net interface copy task
static task_t *netif_task_id;

/* ------------------------------------------------------------------ */
#define ETH_DMA_IT_TST       ((uint32_t)0x20000000)  /*!< Time-stamp trigger interrupt (on DMA) */
#define ETH_DMA_IT_PMT       ((uint32_t)0x10000000)  /*!< PMT interrupt (on DMA) */
#define ETH_DMA_IT_MMC       ((uint32_t)0x08000000)  /*!< MMC interrupt (on DMA) */
#define ETH_DMA_IT_NIS       ((uint32_t)0x00010000)  /*!< Normal interrupt summary */
#define ETH_DMA_IT_AIS       ((uint32_t)0x00008000)  /*!< Abnormal interrupt summary */
#define ETH_DMA_IT_ER        ((uint32_t)0x00004000)  /*!< Early receive interrupt */
#define ETH_DMA_IT_FBE       ((uint32_t)0x00002000)  /*!< Fatal bus error interrupt */
#define ETH_DMA_IT_ET        ((uint32_t)0x00000400)  /*!< Early transmit interrupt */
#define ETH_DMA_IT_RWT       ((uint32_t)0x00000200)  /*!< Receive watchdog timeout interrupt */
#define ETH_DMA_IT_RPS       ((uint32_t)0x00000100)  /*!< Receive process stopped interrupt */
#define ETH_DMA_IT_RBU       ((uint32_t)0x00000080)  /*!< Receive buffer unavailable interrupt */
#define ETH_DMA_IT_R         ((uint32_t)0x00000040)  /*!< Receive interrupt */
#define ETH_DMA_IT_TU        ((uint32_t)0x00000020)  /*!< Underflow interrupt */
#define ETH_DMA_IT_RO        ((uint32_t)0x00000010)  /*!< Overflow interrupt */
#define ETH_DMA_IT_TJT       ((uint32_t)0x00000008)  /*!< Transmit jabber timeout interrupt */
#define ETH_DMA_IT_TBU       ((uint32_t)0x00000004)  /*!< Transmit buffer unavailable interrupt */
#define ETH_DMA_IT_TPS       ((uint32_t)0x00000002)  /*!< Transmit process stopped interrupt */
#define ETH_DMA_IT_T         ((uint32_t)0x00000001)  /*!< Transmit interrupt */

/* ------------------------------------------------------------------ */
/*
  * @brief  Called when a frame is received
  * @param  None
  * @retval None
  */
void eth_isr_vector(void) __attribute__((__interrupt__));
void eth_isr_vector(void)
{
	  isix_sem_signal_isr( netif_sem );
	  /* Clear the Eth DMA Rx IT pending bits */
	  ETH_DMAClearITPendingBit(ETH_DMA_IT_R);
	  ETH_DMAClearITPendingBit(ETH_DMA_IT_NIS);
}

/* ------------------------------------------------------------------ */
static ISIX_TASK_FUNC( netif_task, ifc)
{
	/* Enable MAC and DMA transmission and reception */
	ETH_Start();
	dbprintf("Start eth reception");
	struct netif *netif = (struct netif*)(ifc);
	for(;;)
	{
		if( isix_sem_wait( netif_sem, ISIX_TIME_INFINITE ) == ISIX_EOK )
		{
			//dbprintf("Got packet !!!");
			while( ETH_GetRxPktSize() !=0 )
			{
				ethernetif_input( netif );
			}
		}
	}
}

/* ------------------------------------------------------------------ */
/**
 * In this function, the hardware should be initialized.
 * Called from stm32_emac_if_init_callback().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void
low_level_init(struct netif *netif)
{
  /* Initialize Tx Descriptors list: Chain Mode */
  ETH_DMATxDescChainInit(DMATxDscrTab, &Tx_Buff[0][0], ETH_TXBUFNB);
  /* Initialize Rx Descriptors list: Chain Mode  */
  ETH_DMARxDescChainInit(DMARxDscrTab, &Rx_Buff[0][0], ETH_RXBUFNB);

  /* Enable the Ethernet Rx Interrupt */
  ETH_DMAITConfig(ETH_DMA_IT_NIS | ETH_DMA_IT_R, ENABLE);

	//Enable eth irq in nvic
	nvic_set_priority( ETH_IRQn ,1, 7 );
	nvic_irq_enable( ETH_IRQn, true );

  /* Enable Ethernet Rx interrrupt */
  { int i;
    for(i=0; i<ETH_RXBUFNB; i++)
    {
      ETH_DMARxDescReceiveITConfig(&DMARxDscrTab[i], ENABLE);
    }
  }

#ifdef ISIX_TCPIPLIB_CHECKSUM_BY_HARDWARE
  /* Enable the checksum insertion for the Tx frames */
  { int i;
    for(i=0; i<ETH_TXBUFNB; i++)
    {
      ETH_DMATxDescChecksumInsertionConfig(&DMATxDscrTab[i], ETH_DMATxDesc_ChecksumTCPUDPICMPFull);
    }
  }
#endif
  enum { C_netif_task_stack_size = 256 };
  netif_sem = isix_sem_create_limited( NULL, 0, 1 );
  LWIP_ASSERT("Unable to create netif semaphore", netif_sem);
  netif_task_id = isix_task_create( netif_task, netif, C_netif_task_stack_size, isix_get_min_priority() );
  LWIP_ASSERT("Unable to create netif task", netif_task_id);
}

/* ------------------------------------------------------------------ */
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

static err_t
low_level_output(struct netif *netif, struct pbuf *p)
{
  (void)netif;
  struct pbuf *q;
  int l = 0;
  u8 *buffer =  (u8 *)ETH_GetCurrentTxBuffer();
  
  for(q = p; q != NULL; q = q->next) 
  {
    memcpy((u8_t*)&buffer[l], q->payload, q->len);
	l = l + q->len;
  }
  ETH_TxPkt_ChainMode(l);

  return ERR_OK;
}

/* ------------------------------------------------------------------ */
/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
static struct pbuf *
low_level_input(struct netif *netif)
{
  (void)netif;
  struct pbuf *p, *q;
  u16_t len;
  int l =0;
  FrameTypeDef frame;
  u8 *buffer;
  
  p = NULL;
  frame = ETH_RxPkt_ChainMode();
  /* Obtain the size of the packet and put it into the "len"
     variable. */
  len = frame.length;
  
  buffer = (u8 *)frame.buffer;

  /* We allocate a pbuf chain of pbufs from the pool. */
  p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);

  if (p != NULL)
  {
    for (q = p; q != NULL; q = q->next)
    {
	  memcpy((u8_t*)q->payload, (u8_t*)&buffer[l], q->len);
      l = l + q->len;
    }    
  }


  /* Set Own bit of the Rx descriptor Status: gives the buffer back to ETHERNET DMA */
  frame.descriptor->Status = ETH_DMARxDesc_OWN; 
 
  /* When Rx Buffer unavailable flag is set: clear it and resume reception */
  if ((ETH->DMASR & ETH_DMASR_RBUS) != (u32)RESET)  
  {
    /* Clear RBUS ETHERNET DMA flag */
    ETH->DMASR = ETH_DMASR_RBUS;
    /* Resume DMA reception */
    ETH->DMARPDR = 0;
  }


  return p;
}

/* ------------------------------------------------------------------ */
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
  err_t err = ERR_OK;
  struct pbuf *p;

  /* move received packet into a new pbuf */
  p = low_level_input(netif);

  /* no packet could be read, silently ignore this */
  if (p == NULL) return ERR_MEM;

  struct eth_hdr *ethhdr = p->payload;
  switch( htons( ethhdr->type) )
  {
  case ETHTYPE_IP:
  case ETHTYPE_ARP:
	  /* Update arp table */
	  err = netif->input(p, netif);
	  break;
  default:
	  break;
  }

  if (err != ERR_OK)
  {
    LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
    pbuf_free(p);
    p = NULL;
  }

  return err;
}

/* ------------------------------------------------------------------ */
/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t
stm32_emac_if_init_callback(struct netif *netif)
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
  low_level_init(netif);

  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

  return ERR_OK;
}

/*******************************************************************************
* Function Name  : ETH_RxPkt_ChainMode
* Description    : Receives a packet.
* Input          : None
* Output         : None
* Return         : frame: farme size and location
*******************************************************************************/
FrameTypeDef ETH_RxPkt_ChainMode(void)
{ 
  u32 framelength = 0;
  FrameTypeDef frame = {0,0, NULL};

  /* Check if the descriptor is owned by the ETHERNET DMA (when set) or CPU (when reset) */
  if((DMARxDescToGet->Status & ETH_DMARxDesc_OWN) != (u32)RESET)
  {	
	frame.length = ETH_ERROR;

    if ((ETH->DMASR & ETH_DMASR_RBUS) != (u32)RESET)  
    {
      /* Clear RBUS ETHERNET DMA flag */
      ETH->DMASR = ETH_DMASR_RBUS;
      /* Resume DMA reception */
      ETH->DMARPDR = 0;
    }

	/* Return error: OWN bit set */
    return frame; 
  }
  
  if(((DMARxDescToGet->Status & ETH_DMARxDesc_ES) == (u32)RESET) && 
     ((DMARxDescToGet->Status & ETH_DMARxDesc_LS) != (u32)RESET) &&  
     ((DMARxDescToGet->Status & ETH_DMARxDesc_FS) != (u32)RESET))  
  {      
    /* Get the Frame Length of the received packet: substruct 4 bytes of the CRC */
    framelength = ((DMARxDescToGet->Status & ETH_DMARxDesc_FL) >> ETH_DMARxDesc_FrameLengthShift) - 4;
	
	/* Get the addrees of the actual buffer */
	frame.buffer = DMARxDescToGet->Buffer1Addr;	
  }
  else
  {
    /* Return ERROR */
    framelength = ETH_ERROR;
  }

  frame.length = framelength;


  frame.descriptor = DMARxDescToGet;
  
  /* Update the ETHERNET DMA global Rx descriptor with next Rx decriptor */      
  /* Chained Mode */    
  /* Selects the next DMA Rx descriptor list for next buffer to read */ 
  DMARxDescToGet = (ETH_DMADESCTypeDef*) (DMARxDescToGet->Buffer2NextDescAddr);    
  
  /* Return Frame */
  return (frame);  
}

/*******************************************************************************
* Function Name  : ETH_TxPkt_ChainMode
* Description    : Transmits a packet, from application buffer, pointed by ppkt.
* Input          : - FrameLength: Tx Packet size.
* Output         : None
* Return         : ETH_ERROR: in case of Tx desc owned by DMA
*                  ETH_SUCCESS: for correct transmission
*******************************************************************************/
u32 ETH_TxPkt_ChainMode(u16 FrameLength)
{   
  /* Check if the descriptor is owned by the ETHERNET DMA (when set) or CPU (when reset) */
  if((DMATxDescToSet->Status & ETH_DMATxDesc_OWN) != (u32)RESET)
  {  
	/* Return ERROR: OWN bit set */
    return ETH_ERROR;
  }
        
  /* Setting the Frame Length: bits[12:0] */
  DMATxDescToSet->ControlBufferSize = (FrameLength & ETH_DMATxDesc_TBS1);

  /* Setting the last segment and first segment bits (in this case a frame is transmitted in one descriptor) */    
  DMATxDescToSet->Status |= ETH_DMATxDesc_LS | ETH_DMATxDesc_FS;

  /* Set Own bit of the Tx descriptor Status: gives the buffer back to ETHERNET DMA */
  DMATxDescToSet->Status |= ETH_DMATxDesc_OWN;

  /* When Tx Buffer unavailable flag is set: clear it and resume transmission */
  if ((ETH->DMASR & ETH_DMASR_TBUS) != (u32)RESET)
  {
    /* Clear TBUS ETHERNET DMA flag */
    ETH->DMASR = ETH_DMASR_TBUS;
    /* Resume DMA transmission*/
    ETH->DMATPDR = 0;
  }
  
  /* Update the ETHERNET DMA global Tx descriptor with next Tx decriptor */  
  /* Chained Mode */
  /* Selects the next DMA Tx descriptor list for next buffer to send */ 
  DMATxDescToSet = (ETH_DMADESCTypeDef*) (DMATxDescToSet->Buffer2NextDescAddr);    


  /* Return SUCCESS */
  return ETH_SUCCESS;   
}


/*******************************************************************************
* Function Name  : ETH_GetCurrentTxBuffer
* Description    : Return the address of the buffer pointed by the current descritor.
* Input          : None
* Output         : None
* Return         : Buffer address
*******************************************************************************/
u32 ETH_GetCurrentTxBuffer(void)
{ 
  /* Return Buffer address */
  return (DMATxDescToSet->Buffer1Addr);   
}


/* ------------------------------------------------------------------ */
/**
  * @brief  Deinitializes the ETHERNET peripheral registers to their default reset values.
  * @param  None
  * @retval None
  */
static void eth_deinit(void)
{

  rcc_ahb_periph_reset_cmd( RCC_AHBPeriph_ETH_MAC, true );
  nop();
  rcc_ahb_periph_reset_cmd( RCC_AHBPeriph_ETH_MAC, false );
  nop();
}

/* ------------------------------------------------------------------ */

//GPIO initialize
static void eth_gpio_mii_init(bool provide_mco)
{
    //Enable gpios
	rcc_apb2_periph_clock_cmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
		RCC_APB2Periph_GPIOC |	RCC_APB2Periph_GPIOD |RCC_APB2Periph_AFIO, true);
	/* ETHERNET pins configuration */
	  /* AF Output Push Pull:
	  - ETH_MII_MDIO / ETH_RMII_MDIO: PA2
	  - ETH_MII_MDC / ETH_RMII_MDC: PC1
	  - ETH_MII_TXD2: PC2
	  - ETH_MII_TX_EN / ETH_RMII_TX_EN: PB11
	  - ETH_MII_TXD0 / ETH_RMII_TXD0: PB12
	  - ETH_MII_TXD1 / ETH_RMII_TXD1: PB13
	  - ETH_MII_PPS_OUT / ETH_RMII_PPS_OUT: PB5
	  - ETH_MII_TXD3: PB8 */

	  /* Configure PA2 as alternate function push-pull */
	  gpio_config( GPIOA, 2,  GPIO_MODE_50MHZ, GPIO_CNF_ALT_PP );
	  /* Configure PC1, PC2 and PC3 as alternate function push-pull */
	  gpio_config_ext( GPIOC, (1<<1)| (1<<2), GPIO_MODE_50MHZ, GPIO_CNF_ALT_PP );
	  /* Configure PB5, PB8, PB11, PB12 and PB13 as alternate function push-pull */
	  gpio_config_ext( GPIOB, (1<<5)|(1<<8)|(1<<11)|(1<<12)|(1<<13), GPIO_MODE_50MHZ, GPIO_CNF_ALT_PP );
	  /*               For Remapped Ethernet pins                   */
	  /* Input (Reset Value):
	  - ETH_MII_CRS CRS: PA0
	  - ETH_MII_RX_CLK / ETH_RMII_REF_CLK: PA1
	  - ETH_MII_COL: PA3
	  - ETH_MII_RX_DV / ETH_RMII_CRS_DV: PD8
	  - ETH_MII_TX_CLK: PC3
	  - ETH_MII_RXD0 / ETH_RMII_RXD0: PD9
	  - ETH_MII_RXD1 / ETH_RMII_RXD1: PD10
	  - ETH_MII_RXD2: PD11
	  - ETH_MII_RXD3: PD12
	  - ETH_MII_RX_ER: PB10 */

	  /* ETHERNET pins remapp in STM3210C-EVAL board: RX_DV and RxD[3:0] */
	  gpio_pin_remap_config(GPIO_Remap_ETH, true);

	  /* Configure PA0, PA1 and PA3 as input */
	  gpio_config_ext(GPIOA, (1<<0)|(1<<1)|(1<<3), GPIO_MODE_INPUT, GPIO_CNF_IN_FLOAT );

	  /* Configure PB10 as input */
	  gpio_config( GPIOB, 10, GPIO_MODE_INPUT, GPIO_CNF_IN_FLOAT );

	  /* Configure PC3 as input */
	  gpio_config( GPIOC, 3, GPIO_MODE_INPUT, GPIO_CNF_IN_FLOAT );

	  /* Configure PD8, PD9, PD10, PD11 and PD12 as input */
	  gpio_config_ext(GPIOD, (1<<8)|(1<<9)|(1<<10)|(1<<11)|(1<<12), GPIO_MODE_INPUT, GPIO_CNF_IN_FLOAT );

	  /* MCO pin configuration */
	  /* Configure MCO (PA8) as alternate function push-pull */
	  if(provide_mco)
		  gpio_config( GPIOA, 8 , GPIO_MODE_50MHZ, GPIO_CNF_ALT_PP);
}

/* ------------------------------------------------------------------ */
/**
  * @brief  Configures the Ethernet Interface
  */
static int ethernet_init(uint32_t hclk, uint8_t phy_addr, bool is_rmii ,bool configure_mco)
{
    //Enable eth stuff
	rcc_ahb_periph_clock_cmd( RCC_AHBPeriph_ETH_MAC | RCC_AHBPeriph_ETH_MAC_Tx |
			RCC_AHBPeriph_ETH_MAC_Rx, true );

  ETH_InitTypeDef ETH_InitStructure;

  /* MII/RMII Media interface selection */
  if( !is_rmii )
  {
	 eth_gpio_mii_init(configure_mco);
	 gpio_eth_media_interface_config(GPIO_ETH_MediaInterface_MII);
  }
  else
  {
	 gpio_eth_media_interface_config(GPIO_ETH_MediaInterface_RMII);
  }

  /* Reset ETHERNET on AHB Bus */
  eth_deinit();

  /* Software reset */
  ETH_SoftwareReset();

  /* Wait for software reset */
  while (ETH_GetSoftwareResetStatus() == SET) nop();

  /* ETHERNET Configuration */
  /* Call ETH_StructInit if you don't like to configure all ETH_InitStructure parameter */
  ETH_StructInit(&ETH_InitStructure);
  /* Fill ETH_InitStructure parametrs */
  /*  MAC  */
  ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable;
  ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;
  ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable;
  ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable;
  ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Disable;
  ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable;
  ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;
  ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;
  ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;
#ifdef ISIX_TCPIPLIB_CHECKSUM_BY_HARDWARE
  ETH_InitStructure.ETH_ChecksumOffload = ETH_ChecksumOffload_Enable;
#endif

  /*  DMA */

  /* When we use the Checksum offload feature, we need to enable the Store and Forward mode:
  the store and forward guarantee that a whole frame is stored in the FIFO, so the MAC can insert/verify the checksum,
  if the checksum is OK the DMA can handle the frame otherwise the frame is dropped */
  ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable;
  ETH_InitStructure.ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Enable;
  ETH_InitStructure.ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;

  ETH_InitStructure.ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Disable;
  ETH_InitStructure.ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Disable;
  ETH_InitStructure.ETH_SecondFrameOperate = ETH_SecondFrameOperate_Enable;
  ETH_InitStructure.ETH_AddressAlignedBeats = ETH_AddressAlignedBeats_Enable;
  ETH_InitStructure.ETH_FixedBurst = ETH_FixedBurst_Enable;
  ETH_InitStructure.ETH_RxDMABurstLength = ETH_RxDMABurstLength_32Beat;
  ETH_InitStructure.ETH_TxDMABurstLength = ETH_TxDMABurstLength_32Beat;
  ETH_InitStructure.ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_2_1;
  /* Configure Ethernet */
  if ( !ETH_Init(&ETH_InitStructure, phy_addr, hclk) )
  {
	  return ERR_IF;
  }
  /* konfiguracja diod świecących na ZL3ETH
	 zielona - link status: on = good link, off = no link, blink = activity
	pomarańczowa - speed: on = 100 Mb/s, off = 10 Mb/s
   */
  {
	  enum { PHYIDR1 = 0x02, PHYIDR2 = 0x03, PHYCR =  0x19 };
	  enum { LED_CNFG0 = 0x0020 };
	  enum { LED_CNFG1 = 0x0040 };
	  enum { DP83848_ID = 0x080017 };
	  uint32_t phy_idcode = (((uint32_t)ETH_ReadPHYRegister( phy_addr, PHYIDR1)<<16) |
			  ETH_ReadPHYRegister( phy_addr, PHYIDR2)) >> 10;
	  if( phy_idcode == DP83848_ID )
	  {
		  uint16_t phyreg = ETH_ReadPHYRegister( phy_addr, PHYCR);
		  phyreg &= ~(LED_CNFG0 | LED_CNFG1);
		  ETH_WritePHYRegister( phy_addr, PHYCR, phyreg);
	  }
  }
  return ERR_OK;
}
/* ------------------------------------------------------------------ */

/** Input packet handling */
struct netif* stm32_emac_if_setup( const uint8_t *hw_addr, uint8_t phy_addr, uint32_t hclk,
        bool is_rmii, bool configure_mco )
{
	//Create NETIF interface
	ethernet_init( hclk, phy_addr, is_rmii ,configure_mco );
	struct netif* nifc = (struct netif*)mem_malloc(sizeof(struct netif));
	if (nifc == NULL)
	{
	    LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
	    return NULL;
	}
	memset(nifc, 0, sizeof(struct netif));
	ETH_MACAddressConfig(ETH_MAC_Address0, hw_addr);
	/* set MAC hardware address length */
	nifc->hwaddr_len = ETHARP_HWADDR_LEN;
	/* set MAC hardware address */
	memcpy(nifc->hwaddr, hw_addr, ETHARP_HWADDR_LEN);
	return nifc;
}
