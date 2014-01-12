#include <board_usb_def.h>
#include <timer.h>
#include <usb_interrupt.h>
#include <usb_otg_regs.h>
#include <usbh_configure.h>
#include <usbh_core.h>
#include <usbh_error.h>
#include <usbh_interrupt.h>

#define HOST_VBUS_PORT  xcat(GPIO, HOST_VBUS_GPIO_N)
#define HOST_VBUS_PIN   xcat(GPIO_Pin_, HOST_VBUS_PIN_N)
#define HOST_VBUS_RCC   xcat(RCC_APB2Periph_GPIO, HOST_VBUS_GPIO_N)
#define HOST_VBUS_OFF   (!(HOST_VBUS_ON))

#define HOST_OVRCURR_PORT         xcat(GPIO, HOST_OVRCURR_GPIO_N)
#define HOST_OVRCURR_PIN          xcat(GPIO_Pin_, HOST_OVRCURR_PIN_N)
#define HOST_OVRCURR_RCC          xcat(RCC_APB2Periph_GPIO, HOST_OVRCURR_GPIO_N)
#define HOST_OVRCURR_PORT_SOURCE  xcat(GPIO_PortSourceGPIO, HOST_OVRCURR_GPIO_N)
#define HOST_OVRCURR_PIN_SOURCE   xcat(GPIO_PinSource, HOST_OVRCURR_PIN_N)
#define HOST_OVRCURR_EXTI_LINE    xcat(EXTI_Line, HOST_OVRCURR_PIN_N)
#define HOST_OVRCURR_IRQn         xcat(HOST_OVRCURR_IRQ_N, _IRQn);
#define HOST_OVRCURR_IRQ_HANDLER  xcat(HOST_OVRCURR_IRQ_N, _IRQHandler)

/** Low level USB host initialization for STM32F105 and STM32F107 **/

/* Configure USB central components.
    prio - interrupt preemption priority */
static int USBHcentralConfigure(uint32_t prio) {
  RCC_ClocksTypeDef RCC_ClocksStruct;
  EXTI_InitTypeDef  EXTI_InitStruct;
  GPIO_InitTypeDef  GPIO_InitStruct;
  NVIC_InitTypeDef  NVIC_InitStruct;

  /* USB needs 48 MHz clock. */
  RCC_GetClocksFreq(&RCC_ClocksStruct);
  if (RCC_ClocksStruct.SYSCLK_Frequency == 48000000)
    RCC_OTGFSCLKConfig(RCC_OTGFSCLKSource_PLLVCO_Div2);
  else if (RCC_ClocksStruct.SYSCLK_Frequency == 72000000)
    RCC_OTGFSCLKConfig(RCC_OTGFSCLKSource_PLLVCO_Div3);
  else
    return USBHLIB_ERROR_NOT_SUPPORTED;
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_OTG_FS, ENABLE);

  /* Enable clocks for the VBUS enable output and the overcurrent
     sensing input. RCC_APB2Periph_AFIO clock is required to activate
     EXTI line. */
  RCC_APB2PeriphClockCmd(HOST_VBUS_RCC |
                         HOST_OVRCURR_RCC |
                         RCC_APB2Periph_AFIO,
                         ENABLE);

  /* Configure VBUS power supply. */
  GPIO_WriteBit(HOST_VBUS_PORT, HOST_VBUS_PIN, HOST_VBUS_OFF);
  GPIO_InitStruct.GPIO_Pin = HOST_VBUS_PIN;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(HOST_VBUS_PORT, &GPIO_InitStruct);

  /* Configure the overcurrent input and enable its interrupt. */
  GPIO_InitStruct.GPIO_Pin = HOST_OVRCURR_PIN;
  GPIO_InitStruct.GPIO_Speed = 2;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(HOST_OVRCURR_PORT, &GPIO_InitStruct);

  GPIO_EXTILineConfig(HOST_OVRCURR_PORT_SOURCE,
                      HOST_OVRCURR_PIN_SOURCE);

  EXTI_ClearITPendingBit(HOST_OVRCURR_EXTI_LINE);

  EXTI_InitStruct.EXTI_Line = HOST_OVRCURR_EXTI_LINE;
  EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStruct.EXTI_Trigger = HOST_OVRCURR_EDGE;
  EXTI_InitStruct.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStruct);

  NVIC_InitStruct.NVIC_IRQChannel = HOST_OVRCURR_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = prio;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);

  /* Enable the main USB interrupt. */
  NVIC_InitStruct.NVIC_IRQChannel = OTG_FS_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = prio;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);

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
  ActiveWait(1, 50); /* If not wait, FIFO size registers are not written. */
  USBHvbus(1);  /* Switch VBUS power on. */
  gccfg.d32 = 0;
  gccfg.b.vbusasen = 1; /* Set VBUS sensing on A device. */
  gccfg.b.pwrdwn = 1; /* Deactivate power down. */
  P_USB_OTG_GREGS->GCCFG = gccfg.d32;
  P_USB_OTG_PREGS->PCGCCTL = 0;  /* Not reset by grstctl.b.csrst */
  ActiveWait(1, 50); /* If not wait, FIFO size registers are not written. */

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
int USBHconfigure(usb_phy_t phy) {
  int res;
  uint32_t prio;

  /* Only internal phy transceiver is supported. */
  if (phy != USB_PHY_A)
    return USBHLIB_ERROR_NOT_SUPPORTED;
  prio = USBHgetInterruptPriority();
  res = USBHcentralConfigure(prio);
  if (res < 0)
    return res;
  TimerConfigure(prio, 1);
  FineTimerConfigure(prio, 3);
  res = USBHcoreConfigure();
  if (res < 0)
    return res;
  res = USBHperipheralConfigure();
  if (res < 0)
    return res;
  return USBHLIB_SUCCESS;
}

void USBHvbus(int value) {
  USB_OTG_HPRT_TypeDef hprt;

  hprt.d32 = P_USB_OTG_HREGS->HPRT;
  if (value) {
    /* Switch the host port power on. */
    GPIO_WriteBit(HOST_VBUS_PORT, HOST_VBUS_PIN, HOST_VBUS_ON);
    hprt.b.ppwr = 1;
  }
  else {
    /* Switch the host port power off. */
    GPIO_WriteBit(HOST_VBUS_PORT, HOST_VBUS_PIN, HOST_VBUS_OFF);
    hprt.b.ppwr = 0;
  }
  /* Do not clear any interrupt. */
  hprt.b.pocchng = 0;
  hprt.b.penchng = 0;
  hprt.b.pena = 0;
  hprt.b.pcdet = 0;
  P_USB_OTG_HREGS->HPRT = hprt.d32;
}

void HOST_OVRCURR_IRQ_HANDLER(void) {
  if (EXTI_GetITStatus(HOST_OVRCURR_EXTI_LINE)) {
    EXTI_ClearITPendingBit(HOST_OVRCURR_EXTI_LINE);
    USBHovercurrentInterruptHandler();
  }
}
