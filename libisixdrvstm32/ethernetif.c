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
#include <stdbool.h>
#include <isix.h>
#include <stm32system.h>
#include <stm32rcc.h>
#include "ethernetif.h"

/* ------------------------------------------------------------------ */
enum {  ETH_DMARxDesc_FrameLengthShift   = 16 };

/* ------------------------------------------------------------------ */
enum { ETH_ERROR = 0, ETH_SUCCESS = 1 };

/* ------------------------------------------------------------------ */

enum { ETH_RXBUFNB = 4 };
enum { ETH_TXBUFNB = 2 };
enum { TX_BUFFER_SIZE = ETH_MAX_PACKET_SIZE + VLAN_TAG - ETH_CRC };

/* ------------------------------------------------------------------ */
struct drv_rx_buff_s
{
	struct pbuf *pbuf;		//Receive pbuf
	bool used;				//Buffer is used
};
/* ------------------------------------------------------------------ */
//Transmit and receive descriptors
static ETH_DMADESCTypeDef  dma_rx_ring[ETH_RXBUFNB];
static ETH_DMADESCTypeDef  dma_tx_ring[ETH_TXBUFNB];

//Global buffers for trx frames
static uint8_t tx_buff[ETH_TXBUFNB][TX_BUFFER_SIZE];
static struct drv_rx_buff_s rx_buff[ETH_RXBUFNB];

//Ring descriptor identifier
static volatile size_t dma_tx_idx;
static volatile size_t dma_rx_idx;


/* ------------------------------------------------------------------ */
//Lock semaphore for the driver
static sem_t *netif_sem;
//Net interface copy task
static task_t *netif_task_id;

/* ------------------------------------------------------------------ */
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
/* ------------------------------------------------------------------ */
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

/* ------------------------------------------------------------------ */
/** Reallocat receive buffers */
static void realloc_rx_pbufs(void)
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
        return;
    }
    rx_buff[idx].used = false;
    dma_rx_buffer_set(&dma_rx_ring[idx], rx_buff[idx].pbuf);
    if (dma_rx_ring[idx].ControlBufferSize & ETH_DMARxDesc_RER)
      idx = 0;
    else
      ++idx;
  }
}


/* ------------------------------------------------------------------ */
/* Forward declarations. */
static err_t  ethernetif_input(struct netif *netif);

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
/* Network interrupt bottom half Linux concept of bottom halfes */
static ISIX_TASK_FUNC( netif_task, ifc)
{
	/* Enable MAC and DMA transmission and reception */
	ETH_Start();
	struct netif *netif = (struct netif*)(ifc);
	for(;;)
	{
		if( isix_sem_wait( netif_sem, ISIX_TIME_INFINITE ) == ISIX_EOK )
		{
			while (!(dma_rx_ring[dma_rx_idx].Status & ETH_DMARxDesc_OWN))
			{
				ethernetif_input( netif );
				realloc_rx_pbufs();
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
static void low_level_init(struct netif *netif)
{
  /* Initialize Tx Descriptors list: Chain Mode */
  //ETH_DMATxDescChainInit(DMATxDscrTab, &Tx_Buff[0][0], ETH_TXBUFNB);

  /* Configure TX descriptors
   */
  dma_tx_idx = 0;
  for (int i = 0; i < ETH_TXBUFNB; ++i)
  {
#ifdef ISIX_TCPIPLIB_CHECKSUM_BY_HARDWARE
    dma_tx_ring[i].Status = ETH_DMATxDesc_ChecksumTCPUDPICMPFull;
#else
    dma_tx_ring[i].Status = 0;
#endif
    dma_tx_ring[i].ControlBufferSize = 0;
    dma_tx_ring[i].Buffer1Addr = (uint32_t)tx_buff[i];
    dma_tx_ring[i].Buffer2NextDescAddr = 0;
  }
  dma_tx_ring[ETH_TXBUFNB - 1].Status |= ETH_DMATxDesc_TER;
  ETH->DMATDLAR = (uint32_t)dma_tx_ring;

  /* Initialize Rx Descriptors list: Chain Mode  */
  for (int i = 0; i < ETH_RXBUFNB; ++i)
  {
     /* Zeruj bit ETH_DMARxDesc_DIC, aby uaktywnić przerwanie.
       DMArxRing[i].ControlBufferSize &= ~ETH_DMARxDesc_DIC; */
     dma_rx_ring[i].ControlBufferSize = 0;
     dma_rx_buffer_set(&dma_rx_ring[i], rx_buff[i].pbuf);
  }
  dma_rx_ring[ETH_RXBUFNB - 1].ControlBufferSize |= ETH_DMARxDesc_RER;
  ETH->DMARDLAR = (uint32_t)dma_rx_ring;

  /* Enable the Ethernet Rx Interrupt */
  ETH_DMAITConfig(ETH_DMA_IT_NIS | ETH_DMA_IT_R, ENABLE);

	//Enable eth irq in nvic
	nvic_set_priority( ETH_IRQn ,1, 7 );
	nvic_irq_enable( ETH_IRQn, true );

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

static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
	  (void)netif;
	  if (p == NULL)
	  {
	    return ERR_ARG;
	  }
	  if (p->tot_len > TX_BUFFER_SIZE)
	  {
	    return ERR_BUF; /* Frame not fit in dma buff */
	  }

	  if ( dma_tx_ring[dma_tx_idx].Status & ETH_DMATxDesc_OWN )
	  {
	    return ERR_IF; /* Descriptor is busy */
	  }
#if STM32_ETHERNET_USE_PARTIAL_COPY
	  pbuf_copy_partial(p, (void *)dma_tx_ring[dma_tx_idx].Buffer1Addr, TX_BUFFER_SIZE, 0);
#else
	  {
		  struct pbuf *q;
		  int l = 0;
		  u8 *buffer = (u8*)dma_tx_ring[dma_tx_idx].Buffer1Addr;
		  for(q = p; q != NULL; q = q->next)
		  {
			  memcpy((u8_t*)&buffer[l], q->payload, q->len);
			  l = l + q->len;
		  }
	  }
#endif
	  dma_tx_ring[dma_tx_idx].ControlBufferSize = p->tot_len & ETH_DMATxDesc_TBS1;
	  dma_tx_ring[dma_tx_idx].Status |= ETH_DMATxDesc_FS | ETH_DMATxDesc_LS | ETH_DMATxDesc_OWN;
	  /* Start trasmission again if it is interrupted */
	  if (ETH->DMASR & ETH_DMASR_TBUS)
	  {
	    ETH->DMASR = ETH_DMASR_TBUS;
	    ETH->DMATPDR = 0;
	  }

	  /* Zmień deskryptor nadawczy DMA na następny w pierścieniu. */
	  if (dma_tx_ring[dma_tx_idx].Status & ETH_DMATxDesc_TER)
	    dma_tx_idx = 0;
	  else
	    ++dma_tx_idx;

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
	    len = ETH_GetDMARxDescFrameLength(&dma_rx_ring[dma_rx_idx]);
	    if (len >= ETH_HEADER + MIN_ETH_PAYLOAD + ETH_CRC) {
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

/* ------------------------------------------------------------------ */
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
  low_level_init(netif);

  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

  return ERR_OK;
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
	if ( alloc_rx_pbufs() != ERR_OK )
	{
		mem_free( nifc );
	}
	return nifc;
}
