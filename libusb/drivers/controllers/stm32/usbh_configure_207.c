#include <usb/drivers/controllers/stm32/usb_config.h>
#include <usb/drivers/controllers/stm32/usb_interrupt.h>
#include <usb/drivers/controllers/stm32/usb_otg_regs.h>
#include <usb/core/usbh_configure.h>
#include <usb/core/usbh_core.h>
#include <usb/core/usbh_error.h>
#include <usb/core/usbh_interrupt.h>
#include <usb/core/xcat.h>
#include <stm32system.h>
#include <stm32gpio.h>
#include <stm32rcc.h>
#include <stm32exti.h>
#include <usb/drivers/controllers/stm32/timer.h>
#include <foundation/sys/dbglog.h>
#include <isix.h>


#define USBHOST_VBUS_PORT  usblib_xcat(GPIO, CONFIG_USBHOST_VBUS_GPIO_N)
#define USBHOST_VBUS_RCC   usblib_xcat(RCC_AHB1Periph_GPIO, CONFIG_USBHOST_VBUS_GPIO_N)
#define USBHOST_VBUS_OFF   (!(CONFIG_USBHOST_VBUS_ON))
#define USBHOST_VBUS_ON 	CONFIG_USBHOST_VBUS_ON

#ifdef CONFIG_USBHOST_OVRCURR_ENABLE
#define USBHOST_OVRCURR_PORT         usblib_xcat(GPIO, CONFIG_USBHOST_OVRCURR_GPIO_N)
#define USBHOST_OVRCURR_PIN          CONFIG_USBHOST_OVRCURR_PIN_N 
#define USBHOST_OVRCURR_RCC          usblib_xcat(RCC_AHB1Periph_GPIO, CONFIG_USBHOST_OVRCURR_GPIO_N)
#define USBHOST_OVRCURR_PORT_SOURCE  usblib_xcat(GPIO_PortSourceGPIO, CONFIG_USBHOST_OVRCURR_GPIO_N)
#define USBHOST_OVRCURR_PIN_SOURCE   usblib_xcat(GPIO_PinSource, CONFIG_USBHOST_OVRCURR_PIN_N)
#define USBHOST_OVRCURR_EXTI_LINE    usblib_xcat(EXTI_Line, CONFIG_USBHOST_OVRCURR_PIN_N)
#define USBHOST_OVRCURR_IRQn         usblib_xcat(CONFIG_USBHOST_OVRCURR_IRQ_N, _IRQn)
#endif

/** Low level USB host initialization for STM32F2xx and STM32F4xx **/



uint32_t _USB_OTG_BASE_ADDR;
unsigned _USB_EP_MAX_COUNT;
unsigned _USB_CHNNL_MAX_COUNT;

static usb_phy_t Phy;

/* Configure USB central components.
    prio - interrupt preemption priority */
static int USBHcentralConfigure(uint32_t prio) {

	if (Phy == USB_PHY_A) {
    rcc_ahb1_periph_clock_cmd(RCC_AHB1Periph_GPIOA, true);

    static const unsigned pins = /*(1<<9) | (1<<10) |*/ (1<<11) | (1<<12);
	gpio_config_ext(GPIOA, pins, GPIO_MODE_ALTERNATE, GPIO_PUPD_NONE, GPIO_SPEED_100MHZ, GPIO_OTYPE_PP);
   // gpio_pin_AF_config(GPIOA, GPIO_PinSource9,  GPIO_AF_OTG_FS); /* VBUS */
  //  gpio_pin_AF_config(GPIOA, GPIO_PinSource10, GPIO_AF_OTG_FS); /* ID */
    gpio_pin_AF_config(GPIOA, GPIO_PinSource11, GPIO_AF_OTG_FS); /* DM */
    gpio_pin_AF_config(GPIOA, GPIO_PinSource12, GPIO_AF_OTG_FS); /* DP */

    USE_OTG_FS_REGS();
    rcc_ahb2_periph_clock_cmd(RCC_AHB2Periph_OTG_FS, true);
  }
  else if (Phy == USB_PHY_B) { /* TODO: not tested */
    rcc_ahb1_periph_clock_cmd(RCC_AHB1Periph_GPIOB, true);

	static const unsigned pins = (1<<12) | (1<<13) | (1<<14) | (1<<15);
	gpio_config_ext(GPIOB, pins, GPIO_MODE_ALTERNATE, GPIO_PUPD_NONE, GPIO_SPEED_100MHZ, GPIO_OTYPE_PP);

	gpio_pin_AF_config(GPIOB, GPIO_PinSource12,GPIO_AF_OTG_HS_FS); /* ID */
	gpio_pin_AF_config(GPIOB, GPIO_PinSource13,GPIO_AF_OTG_HS_FS); /* VBUS */
	gpio_pin_AF_config(GPIOB, GPIO_PinSource14,GPIO_AF_OTG_HS_FS); /* DM */
	gpio_pin_AF_config(GPIOB, GPIO_PinSource15,GPIO_AF_OTG_HS_FS); /* DP */

    USE_OTG_HS_REGS();
    rcc_ahb1_periph_clock_cmd(RCC_AHB1Periph_OTG_HS, true);
  }
  else if (Phy == USB_PHY_ULPI) {
    rcc_ahb1_periph_clock_cmd(RCC_AHB1Periph_GPIOA |
                           RCC_AHB1Periph_GPIOB |
                           RCC_AHB1Periph_GPIOC |
                           RCC_AHB1Periph_GPIOH |
                           RCC_AHB1Periph_GPIOI,
                           true);

	{
		static const unsigned pins = (1<<3) | (1<<5);
		gpio_config_ext(GPIOA, pins, GPIO_MODE_ALTERNATE, GPIO_PUPD_NONE, GPIO_SPEED_100MHZ, GPIO_OTYPE_PP);
	}
	{
		static const unsigned pins = (1<<0)|(1<<1)|(1<<5)|(1<<10)|(1<<11)|(1<<12) |(1<<13);
		gpio_config_ext(GPIOB, pins, GPIO_MODE_ALTERNATE, GPIO_PUPD_NONE, GPIO_SPEED_100MHZ, GPIO_OTYPE_PP);
	}

	gpio_config(GPIOC, 0, GPIO_MODE_ALTERNATE, GPIO_PUPD_NONE, GPIO_SPEED_100MHZ, GPIO_OTYPE_PP);
	gpio_config(GPIOH, 4, GPIO_MODE_ALTERNATE, GPIO_PUPD_NONE, GPIO_SPEED_100MHZ, GPIO_OTYPE_PP);
	gpio_config(GPIOI, 11, GPIO_MODE_ALTERNATE, GPIO_PUPD_NONE, GPIO_SPEED_100MHZ, GPIO_OTYPE_PP);


    gpio_pin_AF_config(GPIOA, GPIO_PinSource3,  GPIO_AF_OTG2_HS) ; /* D0 */
    gpio_pin_AF_config(GPIOA, GPIO_PinSource5,  GPIO_AF_OTG2_HS) ; /* CLK */
    gpio_pin_AF_config(GPIOB, GPIO_PinSource0,  GPIO_AF_OTG2_HS) ; /* D1 */
    gpio_pin_AF_config(GPIOB, GPIO_PinSource1,  GPIO_AF_OTG2_HS) ; /* D2 */
    gpio_pin_AF_config(GPIOB, GPIO_PinSource5,  GPIO_AF_OTG2_HS) ; /* D7 */
    gpio_pin_AF_config(GPIOB, GPIO_PinSource10, GPIO_AF_OTG2_HS) ; /* D3 */
    gpio_pin_AF_config(GPIOB, GPIO_PinSource11, GPIO_AF_OTG2_HS) ; /* D4 */
    gpio_pin_AF_config(GPIOB, GPIO_PinSource12, GPIO_AF_OTG2_HS) ; /* D5 */
    gpio_pin_AF_config(GPIOB, GPIO_PinSource13, GPIO_AF_OTG2_HS) ; /* D6 */
    gpio_pin_AF_config(GPIOC, GPIO_PinSource0,  GPIO_AF_OTG2_HS) ; /* STP */
    gpio_pin_AF_config(GPIOH, GPIO_PinSource4,  GPIO_AF_OTG2_HS) ; /* NXT */
    gpio_pin_AF_config(GPIOI, GPIO_PinSource11, GPIO_AF_OTG2_HS) ; /* DIR */

    USE_OTG_HS_REGS();
    rcc_ahb1_periph_clock_cmd(RCC_AHB1Periph_OTG_HS |
                           RCC_AHB1Periph_OTG_HS_ULPI,
                           true);
  }
  else {
    return USBHLIB_ERROR_INVALID_PARAM;
  }

  /* Enable the main USB interrupt. */
  int nvic_chn;
  if (OTG_FS_REGS_USED)
    nvic_chn = OTG_FS_IRQn;
  else if (OTG_HS_REGS_USED)
    nvic_chn = OTG_HS_IRQn;
  else
    return USBHLIB_ERROR_NOT_SUPPORTED;

  nvic_set_priority( nvic_chn, prio, 2 );
  nvic_irq_enable( nvic_chn, true );

  if (Phy == USB_PHY_A || Phy == USB_PHY_B) {
    /* Enable clocks for the VBUS enable output and the overcurrent
       sensing input. RCC_APB2Periph_SYSCFG clock is required to
       activate EXTI line. */
#ifdef  CONFIG_USBHOST_OVRCURR_ENABLE
    rcc_ahb1_periph_clock_cmd(USBHOST_VBUS_RCC | USBHOST_OVRCURR_RCC, true );
#else
    rcc_ahb1_periph_clock_cmd(USBHOST_VBUS_RCC , true );
#endif
    rcc_apb2_periph_clock_cmd(RCC_APB2Periph_SYSCFG, true );

    /* Configure VBUS power supply enable output. */
    if( USBHOST_VBUS_OFF  ) {
		gpio_set( USBHOST_VBUS_PORT, CONFIG_USBHOST_VBUS_PIN );
	} else {
		gpio_clr( USBHOST_VBUS_PORT, CONFIG_USBHOST_VBUS_PIN );
	}
	gpio_config(USBHOST_VBUS_PORT, CONFIG_USBHOST_VBUS_PIN, GPIO_MODE_OUTPUT, 
			GPIO_PUPD_NONE, GPIO_SPEED_2MHZ, CONFIG_USBHOST_VBUS_OTYPE );

#ifdef CONFIG_USBHOST_OVRCURR_ENABLE
    /* Configure the overcurrent input and enable its interrupt. */
	gpio_config( USBHOST_OVRCURR_PORT, USBHOST_OVRCURR_PIN, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO_SPEED_2MHZ, GPIO_OTYPE_PP );
    gpio_exti_line_config(USBHOST_OVRCURR_PORT_SOURCE, USBHOST_OVRCURR_PIN_SOURCE);
    exti_clear_it_pending_bit(USBHOST_OVRCURR_EXTI_LINE);
	exti_init(USBHOST_OVRCURR_EXTI_LINE, EXTI_Mode_Interrupt, CONFIG_USBHOST_OVRCURR_EDGE, true );
	nvic_set_priority(USBHOST_OVRCURR_IRQn, prio, 0 );
	nvic_irq_enable( USBHOST_OVRCURR_IRQn, true );
#endif
  }

  return USBHLIB_SUCCESS;
}

/* Configure USB peripheral. */
	static int USBHperipheralConfigure(void) {
	USB_OTG_GRSTCTL_TypeDef  grstctl;
	USB_OTG_GUSBCFG_TypeDef  gusbcfg;
	USB_OTG_GCCFG_TypeDef    gccfg;
  USB_OTG_GINTMSK_TypeDef  gintmsk;
  USB_OTG_GAHBCFG_TypeDef  gahbcfg;
  USB_OTG_FIFOSIZE_TypeDef fifosize;
  uint32_t rx_fifosize, tx_nonperiodic_fifosize, tx_periodic_fifosize;
  int timeout;

  /* Reset the USB core. */
  grstctl.d32 = 0;
  grstctl.b.csrst = 1;
  P_USB_OTG_GREGS->GRSTCTL = grstctl.d32;
  timeout = 20;
  do {
    if (--timeout < 0)
      return USBHLIB_ERROR_TIMEOUT;
    grstctl.d32 = P_USB_OTG_GREGS->GRSTCTL;
  } while (grstctl.b.csrst == 1);

  /* Wait for the AHB master idle state. */
  grstctl.d32 = 0;
  timeout = 20;
  do {
    if (--timeout < 0)
      return USBHLIB_ERROR_TIMEOUT;
    grstctl.d32 = P_USB_OTG_GREGS->GRSTCTL;
  } while (grstctl.b.ahbidl == 0);

  /* Configure USB in the host mode. */
  gusbcfg.d32 = 0;
  gusbcfg.b.fhmod = 1; /* Force the host mode. */
  if (Phy == USB_PHY_B) { /* TODO: not tested */
    gusbcfg.b.physel = 1;
  }
  else if (Phy == USB_PHY_ULPI) {
    gusbcfg.b.ulpievbusd = 1; /* Use an external VBUS supply. */
    gusbcfg.b.ulpievbusi = 1; /* Use an external VBUS comparator. */
  }
  P_USB_OTG_GREGS->GUSBCFG = gusbcfg.d32;
  isix_wait_ms(50); /* If not wait, FIFO size registers are not written. */
  USBHvbus(1);  /* Switch VBUS power on. */
  gccfg.d32 = 0;
  gccfg.b.vbusasen = 1; /* Set VBUS sensing on A device. */
  if (Phy == USB_PHY_A || Phy == USB_PHY_B)
    gccfg.b.pwrdwn = 1; /* Deactivate power down. */
  P_USB_OTG_GREGS->GCCFG = gccfg.d32;
  P_USB_OTG_PREGS->PCGCCTL = 0;  /* Not reset by grstctl.b.csrst */
  isix_wait_ms(50); /* If not wait, FIFO size registers are not written. */

  /* Calculate the frame interval based on the PHY clock selected in
     the fslspcd field of the HCFG register.
  P_USB_OTG_HREGS->HFIR = 0; */

  /* Configure FIFOs. Values are in 4-byte words. */
  if (OTG_FS_REGS_USED) { /* totally max 320 words */
    rx_fifosize             = 128;
    tx_nonperiodic_fifosize = 96;
    tx_periodic_fifosize    = 96;
  }
  else if (OTG_HS_REGS_USED) { /* totally max 1024 words */
    rx_fifosize             = 400;
    tx_nonperiodic_fifosize = 312;
    tx_periodic_fifosize    = 312;
  }
  else {
    return USBHLIB_ERROR_NOT_SUPPORTED;
  }

  P_USB_OTG_GREGS->GRXFSIZ = rx_fifosize;
  fifosize.b.startaddr = rx_fifosize;
  fifosize.b.depth = tx_nonperiodic_fifosize;
  P_USB_OTG_GREGS->HNPTXFSIZ = fifosize.d32;
  fifosize.b.startaddr = rx_fifosize + tx_nonperiodic_fifosize;
  fifosize.b.depth = tx_periodic_fifosize;
  P_USB_OTG_GREGS->HPTXFSIZ = fifosize.d32;

  /* Configure the global interrupt. */
  gintmsk.d32 = 0;
  gintmsk.b.sofm = 1;
  gintmsk.b.rxflvlm = 1;
  gintmsk.b.hprtim = 1;
  gintmsk.b.hcim = 1;
  gintmsk.b.discim = 1;
  P_USB_OTG_GREGS->GINTMSK = gintmsk.d32;
  gahbcfg.d32 = 0;
  /* We do not use TX FIFO interrupt.
  gahbcfg.b.ptxfelvl = 1;
  gahbcfg.b.txfelvl = 1; */
  gahbcfg.b.gintmsk = 1; /* Enable global interrupt. */
  P_USB_OTG_GREGS->GAHBCFG = gahbcfg.d32;

  return USBHLIB_SUCCESS;
}

/* Configure USB host interface.
    phy  - used phy transceiver */
int usbh_configure(usb_phy_t phy) {
  int res;
  uint32_t prio;
  Phy = phy;
  prio = usbhp_get_interrupt_priority();
  res = USBHcentralConfigure(prio);
  if (res < 0) {
	 dbprintf( "Central configfure err %i", res );
	 return res;
  }
  usblibp_timer_configure();
#ifdef CONFIG_USBLIB_US_TIM_N
  usblibp_fine_timer_configure(prio, 3, CONFIG_PCLK1_HZ );
#endif
  res = USBHcoreConfigure();
  if (res < 0) {
	  dbprintf( "Core configfure err %i", res );
	  return res;
  }
  res = USBHperipheralConfigure();
  if (res < 0) {
	  dbprintf( "Periph configfure err %i", res );
	  return res;
  }
  return USBHLIB_SUCCESS;
}


static inline void GPIO_WriteBit( GPIO_TypeDef* port, uint16_t pin, bool en ) {
	if( en ) {
		gpio_set( port, pin );
	} else {
		gpio_clr( port, pin );
	}
}

void USBHvbus(int value) {
  USB_OTG_HPRT_TypeDef hprt;

  hprt.d32 = P_USB_OTG_HREGS->HPRT;
  if (value) {
    /* Switch the host port power on. */
    if (Phy == USB_PHY_A || Phy == USB_PHY_B)
      GPIO_WriteBit(USBHOST_VBUS_PORT, CONFIG_USBHOST_VBUS_PIN, USBHOST_VBUS_ON);
    hprt.b.ppwr = 1;
  }
  else {
    /* Switch the host port power off. */
    if (Phy == USB_PHY_A || Phy == USB_PHY_B)
      GPIO_WriteBit(USBHOST_VBUS_PORT, CONFIG_USBHOST_VBUS_PIN, USBHOST_VBUS_OFF);
    hprt.b.ppwr = 0;
  }
  /* Do not clear any interrupt. */
  hprt.b.pocchng = 0;
  hprt.b.penchng = 0;
  hprt.b.pena = 0;
  hprt.b.pcdet = 0;
  P_USB_OTG_HREGS->HPRT = hprt.d32;
}



