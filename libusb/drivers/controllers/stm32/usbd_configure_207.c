#include <board_usb_def.h>
#include <delay.h>
#include <usb_otg_regs.h>
#include <usbd_configure.h>
#include <usbd_core.h>

/** Low level USB device initialization for STM32F205, STM32F207,
    STM32F215 and STM32F217, also applicable for STM32F405, STM32F407,
    STM32F415 and STM32F417. **/

uint32_t USB_OTG_BASE_ADDR;
unsigned EP_MAX_COUNT;
unsigned CHNNL_MAX_COUNT;

static usb_speed_t speed;
static usb_phy_t   phy;

/* Configure USB central components.
    prio    - interrupt preemption priority
    subprio - interrupt service order when the same priority */
static int USBDcentralConfigure(unsigned prio, unsigned subprio) {
  GPIO_InitTypeDef GPIO_InitStruct;
  NVIC_InitTypeDef NVIC_InitStruct;

  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;

  if (speed == FULL_SPEED && phy == USB_PHY_A) {
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
  else if (speed == FULL_SPEED && phy == USB_PHY_B) { /* TODO: not tested */
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
  else if (speed == FULL_SPEED && phy == USB_PHY_I2C) { /* TODO: not tested */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB , ENABLE);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1 |
                               GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource0,  GPIO_AF_OTG2_FS); /* RESET */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource1,  GPIO_AF_OTG2_FS); /* INTN */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_OTG2_FS); /* SCL */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_OTG2_FS); /* SDA */

    USE_OTG_HS_REGS();
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_OTG_HS, ENABLE);
  }
  else if (phy == USB_PHY_ULPI) {
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

  if (OTG_FS_REGS_USED)
    NVIC_InitStruct.NVIC_IRQChannel = OTG_FS_IRQn;
  else if (OTG_HS_REGS_USED)
    NVIC_InitStruct.NVIC_IRQChannel = OTG_HS_IRQn;
  else
    return -1;

  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = prio;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = subprio;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);

  return 0;
}

/* Reset USB peripheral and simulate cable disconnection. */
static int USBDperipheralReset(void) {
  USB_OTG_GRSTCTL_TypeDef grstctl;
  USB_OTG_DCTL_TypeDef    dctl;
  int timeout;

  /* Reset the USB core. */
  grstctl.d32 = 0;
  grstctl.b.csrst = 1;
  P_USB_OTG_GREGS->GRSTCTL = grstctl.d32;
  timeout = 20;
  do {
    if (--timeout < 0)
      return -1;
    grstctl.d32 = P_USB_OTG_GREGS->GRSTCTL;
  } while (grstctl.b.csrst == 1);

  /* Wait for the AHB master idle state. */
  timeout = 20;
  do {
    if (--timeout < 0)
      return -1;
    grstctl.d32 = P_USB_OTG_GREGS->GRSTCTL;
  } while (grstctl.b.ahbidl == 0);

  /* Disconnect as early as possible. */
  dctl.d32 = 0;
  dctl.b.sdis = 1;
  P_USB_OTG_DREGS->DCTL = dctl.d32;

  return 0;
}

/* Configure USB peripheral.
    clk - SYSCLK clock in MHz */
static int USBDperipheralConfigure(int clk) {
  USB_OTG_DCFG_TypeDef    dcfg;
  USB_OTG_GUSBCFG_TypeDef gusbcfg;
  USB_OTG_GCCFG_TypeDef   gccfg;
  USB_OTG_GI2CCTL_TypeDef gi2cctl;
  USB_OTG_GINTMSK_TypeDef gintmsk;
  USB_OTG_GAHBCFG_TypeDef gahbcfg;

  /* Configure USB in the device mode.
     DCFG OTG HS register is badly documented in RM0033.
      Reset value is 0x00200200.
      If the nzlsohsk field is 1 then the STALL handshake is sent on
      a non-zero-length status OUT transaction and the received OUT
      packet is not sent to the application.
      The dspd field has the same coding as usb_speed_t and
      the XcvrSelect field in USB3300 PHY:
       0 == HS transceiver,
       1 == FS transceiver,
       2 == LS transceiver,
       3 == FS transceiver for LS packets (FS preamble automatically
            pre-ended).
     GUSBCFG OTG HS register is badly documented in RM0033.
      Reset value is 0x00001410.
      Undocumented or badly documented fields:
       phyif (bit 3) - 0 == 8 bit interface,
       ulpisel (bit 4) - read-only, always 1,
       fsintf (bit 5) - function unknown,
       physel (bit 6) - perhaps select internal PHY,
       ddrsel (bit 7) - 0 == single data rate, 1 == double data rate,
       trdt (bits 10:13) - reset value 5, for 120 MHz should be 3.
    For OTG FS registers see comment is usbd_configure_107.c. */
  dcfg.d32 = 0;
  dcfg.b.nzlsohsk = 1;
  dcfg.b.dspd = speed;
  P_USB_OTG_DREGS->DCFG = dcfg.d32;
  gusbcfg.d32 = 0;
  gusbcfg.b.fdmod = 1; /* Force the device mode. */
  if (clk >= 96)
    gusbcfg.b.trdt = 3;
  else if (clk >= 64)
    gusbcfg.b.trdt = 4;
  else
    gusbcfg.b.trdt = 5;
  if (phy == USB_PHY_B) /* TODO: not tested */
    gusbcfg.b.physel = 1;
  else if (phy == USB_PHY_ULPI)
    gusbcfg.b.ulpievbusd = 1; /* Use an external VBUS supply. */
  else if (phy == USB_PHY_I2C) /* TODO: not tested */
    gusbcfg.b.otgutmifssel = 1;
  P_USB_OTG_GREGS->GUSBCFG = gusbcfg.d32;
  gccfg.d32 = 0;
  gccfg.b.vbusbsen = 1; /* Set VBUS sensing on B device. */
  if (phy == USB_PHY_A || phy == USB_PHY_B)
    gccfg.b.pwrdwn = 1; /* Deactivate PHY power down. */
  else if (phy == USB_PHY_I2C) { /* TODO: not tested */
    gccfg.b.pwrdwn = 1;
    gccfg.b.i2cpaden = 1;
  }
  P_USB_OTG_GREGS->GCCFG = gccfg.d32;
  if (phy == USB_PHY_I2C) { /* TODO: not tested */
    gi2cctl.d32 = 0;
    gi2cctl.b.addr = 0x2d;
    gi2cctl.b.i2cdevaddr = 1;
    gi2cctl.b.i2cdatse0 = 1;
    P_USB_OTG_GREGS->GI2CCTL = gi2cctl.d32;
    Delay(200);
    gi2cctl.b.i2cen = 1;
    P_USB_OTG_GREGS->GI2CCTL = gi2cctl.d32;
  }
  P_USB_OTG_PREGS->PCGCCTL = 0;  /* Not reset by grstctl.b.csrst */

  /* Configure the global interrupt. */
  gintmsk.d32 = 0;
  gintmsk.b.usbrstm = 1;
  gintmsk.b.enumdnem = 1;
  gintmsk.b.rxflvlm = 1;
  gintmsk.b.oepim = 1;
  gintmsk.b.iepim = 1;
  gintmsk.b.sofm = 1; /* Required for isochronous transfers */
  P_USB_OTG_GREGS->GINTMSK = gintmsk.d32;
  gahbcfg.d32 = 0;
  gahbcfg.b.txfelvl = 1; /* Interrupt when TX FIFO completely empty */
  gahbcfg.b.gintmsk = 1; /* Enable the global interrupt. */
  P_USB_OTG_GREGS->GAHBCFG = gahbcfg.d32;

  /* Simulate cable reconnection. Disconnet time for high speed must
     be longer than 125 us - add some delay. */
  Delay(400);
  P_USB_OTG_DREGS->DCTL = 0;

  return 0;
}

/** USB device initialization API **/

/* This function is provided only for compatibility with STM32F103. */
int USBDpreConfigure(usb_speed_t s, usb_phy_t p) {
  speed = s;
  phy = p;
  return s == FULL_SPEED || s == HIGH_SPEED ? 0 : -1;
}

/* Configure USB interface.
    prio    - interrupt preemption priority
    subprio - interrupt service order when the same priority
    clk     - SYSCLK clock in MHz */
int USBDconfigure(unsigned prio, unsigned subprio, int clk) {
  if (USBDcoreConfigure() < 0)
    return -1;
  if (USBDcentralConfigure(prio, subprio) < 0)
    return -1;
  if (USBDperipheralReset() < 0)
    return -1;
  if (USBDperipheralConfigure(clk) < 0)
    return -1;
  return 0;
}
