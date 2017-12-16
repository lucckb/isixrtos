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
#define USBHOST_VBUS_RCC   usblib_xcat(RCC_APB2Periph_GPIO, CONFIG_USBHOST_VBUS_GPIO_N)
#define USBHOST_VBUS_OFF   (!(CONFIG_USBHOST_VBUS_ON))
#define USBHOST_VBUS_ON 	CONFIG_USBHOST_VBUS_ON

#ifdef CONFIG_USBHOST_OVRCURR_ENABLE
#define USBHOST_OVRCURR_PORT         usblib_xcat(GPIO, CONFIG_USBHOST_OVRCURR_GPIO_N)
#define USBHOST_OVRCURR_PIN          CONFIG_USBHOST_OVRCURR_PIN_N 
#define USBHOST_OVRCURR_RCC          usblib_xcat(RCC_APB2Periph_GPIO, CONFIG_USBHOST_OVRCURR_GPIO_N)
#define USBHOST_OVRCURR_PORT_SOURCE  usblib_xcat(GPIO_PortSourceGPIO, CONFIG_USBHOST_OVRCURR_GPIO_N)
#define USBHOST_OVRCURR_PIN_SOURCE   usblib_xcat(GPIO_PinSource, CONFIG_USBHOST_OVRCURR_PIN_N)
#define USBHOST_OVRCURR_EXTI_LINE    usblib_xcat(EXTI_Line, CONFIG_USBHOST_OVRCURR_PIN_N)
#define USBHOST_OVRCURR_IRQn         usblib_xcat(CONFIG_USBHOST_OVRCURR_IRQ_N, _IRQn)
#else
#define USBHOST_OVRCURR_RCC 0
#endif
/** Low level USB host initialization for STM32F105 and STM32F107 **/

/* Configure USB central components.
    prio - interrupt preemption priority */
static int USBHcentralConfigure(uint32_t prio) {

  rcc_ahb_periph_clock_cmd( RCC_AHBPeriph_OTG_FS, true );

  /* Enable clocks for the VBUS enable output and the overcurrent
     sensing input. RCC_APB2Periph_AFIO clock is required to activate
     EXTI line. */
  rcc_apb2_periph_clock_cmd( USBHOST_VBUS_RCC | USBHOST_OVRCURR_RCC |
                         RCC_APB2Periph_AFIO, true );

  /* Configure VBUS power supply. */
  gpio_clr( USBHOST_VBUS_PORT, CONFIG_USBHOST_VBUS_PIN );
  gpio_config( USBHOST_VBUS_PORT, CONFIG_USBHOST_VBUS_PIN, GPIO_MODE_2MHZ, CONFIG_USBHOST_VBUS_OTYPE );
#ifdef CONFIG_USBHOST_OVRCURR_ENABLE
  /* Configure the overcurrent input and enable its interrupt. */
  gpio_config( USBHOST_OVRCURR_PORT, USBHOST_OVRCURR_PIN, GPIO_MODE_INPUT, GPIO_CNF_IN_FLOAT );
  gpio_exti_line_config( USBHOST_OVRCURR_PORT_SOURCE, USBHOST_OVRCURR_PIN_SOURCE );
  exti_clr_pending_bit(USBHOST_OVRCURR_EXTI_LINE);
  exti_init( USBHOST_OVRCUR_EXTI_LINE, EXTI_Mode_Interrupt, USBHOST_OVRCURR_EDGE , true );


  NVIC_InitStruct.NVIC_IRQChannel = HOST_OVRCURR_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = prio;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);
  nvic_set_priority( USBHOST_OVRCURR_IRQn, prio, 0 );
  nvic_irq_enable( USBHOST_OVRCURR_IRQn, true );
#endif
  /* Enable the main USB interrupt. */
  nvic_set_priority( OTG_FS_IRQn, prio, 2 );
  nvic_irq_enable( OTG_FS_IRQn, true );
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
  /* gusbcfg.b.physel = 1; This bit is read-only and always 1. */
  P_USB_OTG_GREGS->GUSBCFG = gusbcfg.d32;
  isix_wait_ms( 50 ); /* If not wait, FIFO size registers are not written. */
  USBHvbus(1);  /* Switch VBUS power on. */
  gccfg.d32 = 0;
  gccfg.b.vbusasen = 1; /* Set VBUS sensing on A device. */
  gccfg.b.pwrdwn = 1; /* Deactivate power down. */
  P_USB_OTG_GREGS->GCCFG = gccfg.d32;
  P_USB_OTG_PREGS->PCGCCTL = 0;  /* Not reset by grstctl.b.csrst */
  isix_wait_ms(50); /* If not wait, FIFO size registers are not written. */

  /* Calculate the frame interval based on the PHY clock selected in
     the fslspcd field of the HCFG register.
  P_USB_OTG_HREGS->HFIR = 0; */

  /* Configure FIFOs. Values in 4-byte words. Totally max 320 words. */
  rx_fifosize             = 128;
  tx_nonperiodic_fifosize = 96;
  tx_periodic_fifosize    = 96;

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
  /* We do not use TS FIFO interrupt.
  gahbcfg.b.ptxfelvl = 1;
  gahbcfg.b.txfelvl = 1; */
  gahbcfg.b.gintmsk = 1; /* Enable global interrupt. */
  P_USB_OTG_GREGS->GAHBCFG = gahbcfg.d32;

  return USBHLIB_SUCCESS;
}

/* Configure USB host interface.
    phy   - used phy transceiver */
int usbh_configure(usb_phy_t phy) {
  int res;
  uint32_t prio;

  /* Only internal phy transceiver is supported. */
  if (phy != USB_PHY_A)
    return USBHLIB_ERROR_NOT_SUPPORTED;
  prio = usbhp_get_interrupt_priority();
  res = USBHcentralConfigure(prio);
  if (res < 0)
    return res;
  usblibp_timer_configure();
  usblibp_fine_timer_configure(prio, 3, CONFIG_PCLK1_HZ );
  res = USBHcoreConfigure();
  if (res < 0)
    return res;
  res = USBHperipheralConfigure();
  if (res < 0)
    return res;
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
    GPIO_WriteBit(USBHOST_VBUS_PORT, CONFIG_USBHOST_VBUS_PIN, USBHOST_VBUS_ON);
    hprt.b.ppwr = 1;
  }
  else {
    /* Switch the host port power off. */
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


