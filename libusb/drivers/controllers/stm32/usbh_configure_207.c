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
#define HOST_VBUS_RCC   xcat(RCC_AHB1Periph_GPIO, HOST_VBUS_GPIO_N)
#define HOST_VBUS_OFF   (!(HOST_VBUS_ON))

#define HOST_OVRCURR_PORT         xcat(GPIO, HOST_OVRCURR_GPIO_N)
#define HOST_OVRCURR_PIN          xcat(GPIO_Pin_, HOST_OVRCURR_PIN_N)
#define HOST_OVRCURR_RCC          xcat(RCC_AHB1Periph_GPIO, HOST_OVRCURR_GPIO_N)
#define HOST_OVRCURR_PORT_SOURCE  xcat(EXTI_PortSourceGPIO, HOST_OVRCURR_GPIO_N)
#define HOST_OVRCURR_PIN_SOURCE   xcat(EXTI_PinSource, HOST_OVRCURR_PIN_N)
#define HOST_OVRCURR_EXTI_LINE    xcat(EXTI_Line, HOST_OVRCURR_PIN_N)
#define HOST_OVRCURR_IRQn         xcat(HOST_OVRCURR_IRQ_N, _IRQn);
#define HOST_OVRCURR_IRQ_HANDLER  xcat(HOST_OVRCURR_IRQ_N, _IRQHandler)

/** Low level USB host initialization for STM32F2xx and STM32F4xx **/

uint32_t USB_OTG_BASE_ADDR;
unsigned EP_MAX_COUNT;
unsigned CHNNL_MAX_COUNT;

static usb_phy_t Phy;

/* Configure USB central components.
    prio - interrupt preemption priority */
static int USBHcentralConfigure(uint32_t prio) {
  GPIO_InitTypeDef GPIO_InitStruct;
  EXTI_InitTypeDef EXTI_InitStruct;
  NVIC_InitTypeDef NVIC_InitStruct;

  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;

  if (Phy == USB_PHY_A) {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10 |
                               GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9,  GPIO_AF_OTG1_FS); /* VBUS */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_OTG1_FS); /* ID */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_OTG1_FS); /* DM */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_OTG1_FS); /* DP */

    USE_OTG_FS_REGS();
    RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_OTG_FS, ENABLE);
  }
  else if (Phy == USB_PHY_B) { /* TODO: not tested */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 |
                               GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_OTG2_FS); /* ID */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_OTG2_FS); /* VBUS */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_OTG2_FS); /* DM */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_OTG2_FS); /* DP */

    USE_OTG_HS_REGS();
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_OTG_HS, ENABLE);
  }
  else if (Phy == USB_PHY_ULPI) {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA |
                           RCC_AHB1Periph_GPIOB |
                           RCC_AHB1Periph_GPIOC |
                           RCC_AHB1Periph_GPIOH |
                           RCC_AHB1Periph_GPIOI,
                           ENABLE);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_5;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1  |
                               GPIO_Pin_5  | GPIO_Pin_10 |
                               GPIO_Pin_11 | GPIO_Pin_12 |
                               GPIO_Pin_13;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
    GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
    GPIO_Init(GPIOH, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
    GPIO_Init(GPIOI, &GPIO_InitStruct);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3,  GPIO_AF_OTG2_HS) ; /* D0 */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource5,  GPIO_AF_OTG2_HS) ; /* CLK */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource0,  GPIO_AF_OTG2_HS) ; /* D1 */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource1,  GPIO_AF_OTG2_HS) ; /* D2 */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource5,  GPIO_AF_OTG2_HS) ; /* D7 */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_OTG2_HS) ; /* D3 */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_OTG2_HS) ; /* D4 */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_OTG2_HS) ; /* D5 */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_OTG2_HS) ; /* D6 */
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource0,  GPIO_AF_OTG2_HS) ; /* STP */
    GPIO_PinAFConfig(GPIOH, GPIO_PinSource4,  GPIO_AF_OTG2_HS) ; /* NXT */
    GPIO_PinAFConfig(GPIOI, GPIO_PinSource11, GPIO_AF_OTG2_HS) ; /* DIR */

    USE_OTG_HS_REGS();
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_OTG_HS |
                           RCC_AHB1Periph_OTG_HS_ULPI,
                           ENABLE);
  }
  else {
    return USBHLIB_ERROR_INVALID_PARAM;
  }

  /* Enable the main USB interrupt. */
  if (OTG_FS_REGS_USED)
    NVIC_InitStruct.NVIC_IRQChannel = OTG_FS_IRQn;
  else if (OTG_HS_REGS_USED)
    NVIC_InitStruct.NVIC_IRQChannel = OTG_HS_IRQn;
  else
    return USBHLIB_ERROR_NOT_SUPPORTED;

  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = prio;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);

  if (Phy == USB_PHY_A || Phy == USB_PHY_B) {
    /* Enable clocks for the VBUS enable output and the overcurrent
       sensing input. RCC_APB2Periph_SYSCFG clock is required to
       activate EXTI line. */
    RCC_AHB1PeriphClockCmd(HOST_VBUS_RCC | HOST_OVRCURR_RCC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    /* Configure VBUS power supply enable output. */
    GPIO_WriteBit(HOST_VBUS_PORT, HOST_VBUS_PIN, HOST_VBUS_OFF);
    GPIO_InitStruct.GPIO_Pin = HOST_VBUS_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(HOST_VBUS_PORT, &GPIO_InitStruct);

    /* Configure the overcurrent input and enable its interrupt. */
    GPIO_InitStruct.GPIO_Pin = HOST_OVRCURR_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(HOST_OVRCURR_PORT, &GPIO_InitStruct);

    SYSCFG_EXTILineConfig(HOST_OVRCURR_PORT_SOURCE,
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
  ActiveWait(1, 50); /* If not wait, FIFO size registers are not written. */
  USBHvbus(1);  /* Switch VBUS power on. */
  gccfg.d32 = 0;
  gccfg.b.vbusasen = 1; /* Set VBUS sensing on A device. */
  if (Phy == USB_PHY_A || Phy == USB_PHY_B)
    gccfg.b.pwrdwn = 1; /* Deactivate power down. */
  P_USB_OTG_GREGS->GCCFG = gccfg.d32;
  P_USB_OTG_PREGS->PCGCCTL = 0;  /* Not reset by grstctl.b.csrst */
  ActiveWait(1, 50); /* If not wait, FIFO size registers are not written. */

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
int USBHconfigure(usb_phy_t phy) {
  int res;
  uint32_t prio;

  Phy = phy;
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
    if (Phy == USB_PHY_A || Phy == USB_PHY_B)
      GPIO_WriteBit(HOST_VBUS_PORT, HOST_VBUS_PIN, HOST_VBUS_ON);
    hprt.b.ppwr = 1;
  }
  else {
    /* Switch the host port power off. */
    if (Phy == USB_PHY_A || Phy == USB_PHY_B)
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
