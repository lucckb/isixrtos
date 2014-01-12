#include <board_usb_def.h>
#include <usb_otg_regs.h>
#include <usbd_configure.h>
#include <usbd_core.h>

/** Low level USB device initialization for STM32F105 and STM32F107 **/

/* USB core needs 48 MHz clock.
    clk - SYSCLK clock in MHz */
static int USBDclockConfigure(int clk) {
  if (clk == 48)
    RCC_OTGFSCLKConfig(RCC_OTGFSCLKSource_PLLVCO_Div2);
  else if (clk == 72)
    RCC_OTGFSCLKConfig(RCC_OTGFSCLKSource_PLLVCO_Div3);
  else
    return -1;
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_OTG_FS, ENABLE);
  return 0;
}

/* Configure interrupt controler.
    prio    - interrupt preemption priority
    subprio - interrupt service order when the same priority */
static int USBDinterruptConfigure(unsigned prio, unsigned subprio) {
  NVIC_InitTypeDef NVIC_InitStruct;

  NVIC_InitStruct.NVIC_IRQChannel = OTG_FS_IRQn;
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
  USB_OTG_GINTMSK_TypeDef gintmsk;
  USB_OTG_GAHBCFG_TypeDef gahbcfg;

  /* Configure USB in the full speed device mode.
     DCFG OTG FS register is badly documented in RM0008 and RM0033.
      Reset value is 0x00200000.
      If the nzlsohsk field is 1 then the STALL handshake is sent on
      a non-zero-length status OUT transaction and the received OUT
      packet is not sent to the application.
      The dspd field has no meaning for the internal full speed PHY.
     GUSBCFG OTG FS register is badly documented in RM0008 and RM0033.
      Reset value is 0x00001440.
      The trdt field reset value is 5. For 72 MHz it should be 4.
      The physel field is bit 6. It is read-only and always 1. */
  dcfg.d32 = 0;
  dcfg.b.nzlsohsk = 1;
  P_USB_OTG_DREGS->DCFG = dcfg.d32;
  gusbcfg.d32 = 0;
  gusbcfg.b.fdmod = 1; /* Force the device mode. */
  gusbcfg.b.trdt = clk >= 64 ? 4 : 5;
  P_USB_OTG_GREGS->GUSBCFG = gusbcfg.d32;
  gccfg.d32 = 0;
  gccfg.b.vbusbsen = 1; /* Set VBUS sensing on B device. */
  gccfg.b.pwrdwn = 1; /* Deactivate power down. */
  P_USB_OTG_GREGS->GCCFG = gccfg.d32;
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

  /* Simulate cable reconnection. */
  P_USB_OTG_DREGS->DCTL = 0;

  return 0;
}

/** USB device initialization API **/

/* Call USBpreConfigure before USBconfigure. STM32F105 and STM32F107
   support only full speed devices and internal phy transceiver.
   This function is provided only for compatibility with STM32F103. */
int USBDpreConfigure(usb_speed_t speed, usb_phy_t phy) {
  return speed == FULL_SPEED && phy == USB_PHY_A ? 0 : -1;
}

/* Configure USB interface.
    prio    - interrupt preemption priority
    subprio - interrupt service order when the same priority
    clk     - SYSCLK clock in MHz */
int USBDconfigure(unsigned prio, unsigned subprio, int clk) {
  if (USBDclockConfigure(clk) < 0)
    return -1;
  if (USBDcoreConfigure() < 0)
    return -1;
  if (USBDinterruptConfigure(prio, subprio) < 0)
    return -1;
  if (USBDperipheralReset() < 0)
    return -1;
  if (USBDperipheralConfigure(clk) < 0)
    return -1;
  return 0;
}
