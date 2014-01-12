#include <error.h>
#include <usb_interrupt.h>
#include <usb_otg_regs.h>
#include <usbd_api.h>
#include <usbd_core.h>
#include <usbd_power.h>

/** USB device interrupt handlers for STM32F105, STM32F107, STM32F205,
    STM32F207, STM32F215, STM32F217, STM32F405, STM32F407, STM32F415
    and STM32F417 **/

static void DevResetHandler(void) {
  USB_OTG_DOEPMSK_TypeDef doepmsk;
  USB_OTG_DIEPMSK_TypeDef diepmsk;
  int i;

  /* Disable endpoint interrupts. */
  doepmsk.d32 = 0;
  P_USB_OTG_DREGS->DOEPMSK = doepmsk.d32;
  diepmsk.d32 = 0;
  P_USB_OTG_DREGS->DIEPMSK = diepmsk.d32;
  P_USB_OTG_DREGS->DAINTMSK = 0;
  P_USB_OTG_DREGS->DIEPEMPMSK = 0;

  /* Clear pending endpoint interrupts. */
  for (i = 0; i < EP_MAX_COUNT ; ++i) {
    P_USB_OTG_DINEPS[i].DIEPINTx = 0xff;
    P_USB_OTG_DOUTEPS[i].DOEPINTx = 0xff;
  }

  /* Enable endpoint interrupts. */
  doepmsk.b.stupm = 1;
  doepmsk.b.xfrcm = 1;
  P_USB_OTG_DREGS->DOEPMSK = doepmsk.d32;
  diepmsk.b.xfrcm = 1;
  P_USB_OTG_DREGS->DIEPMSK = diepmsk.d32;
}

static void DevEnumerationDoneHandler(void) {
  USB_OTG_DSTS_TypeDef dsts;

  /* The enumspd field seems to have the same coding as the dspd field
     in the DCFG register. See also comment in usbd_configure_207.c.
     It is not clear, what to do, when enumspd == 3. */
  dsts.d32 = P_USB_OTG_DREGS->DSTS;
  USBDreset(dsts.b.enumspd);
}

/* This interrupt does not need to be cleared.
   Corresponding pending interrupt flags are read only bits. */
static void DevRxFifoNonEmptylHandler(void) {
  USB_OTG_GRXSTS_TypeDef status;

  /* Get the status from the top of the FIFO.
     If a packet is received, read them form the FIFO. */
  status.d32 = P_USB_OTG_GREGS->GRXSTSP;
  if (status.b.pktsts == GRXSTS_PKTSTS_SETUP_RECEIVED ||
      status.b.pktsts == GRXSTS_PKTSTS_OUT_RECEIVED)
    USBDdataReceived(status.b.ch_ep_num, status.b.bcnt);
}

static void DevOutEndPointTransactionHandler(void) {
  USB_OTG_DAINT_TypeDef    daint;
  USB_OTG_DOEPINTx_TypeDef doepint;
  uint32_t oepint, ep;

  daint.d32 = P_USB_OTG_DREGS->DAINT;
  daint.d32 &= P_USB_OTG_DREGS->DAINTMSK;
  for (oepint = daint.b.oepint, ep = 0; oepint; oepint >>= 1, ++ep) {
    if (oepint & 1) {
      doepint.d32 = P_USB_OTG_DOUTEPS[ep].DOEPINTx;
      doepint.d32 &= P_USB_OTG_DREGS->DOEPMSK;

      /* Clear pending interrupt flags. */
      P_USB_OTG_DOUTEPS[ep].DOEPINTx = doepint.d32;

      /* We assume that there is only one interrupt source for
         one endpoint at one time. */
      if (doepint.b.stup)
        USBDtransfer(ep, PID_SETUP);
      if (doepint.b.xfrc)
        USBDtransfer(ep, PID_OUT);
    }
  }
}

static void DevInEndPointTransactionHandler(void) {
  USB_OTG_DAINT_TypeDef    daint;
  USB_OTG_DIEPINTx_TypeDef diepint;
  uint32_t iepint, ep, mask;

  daint.d32 = P_USB_OTG_DREGS->DAINT;
  daint.d32 &= P_USB_OTG_DREGS->DAINTMSK;
  for (iepint = daint.b.iepint, ep = 0; iepint; iepint >>= 1, ++ep) {
    if (iepint & 1) {
      mask = P_USB_OTG_DREGS->DIEPMSK;
      mask |= ((P_USB_OTG_DREGS->DIEPEMPMSK >> ep) & 1) << 7;
      diepint.d32 = P_USB_OTG_DINEPS[ep].DIEPINTx & mask;

      /* Clear pending interrupt flags. */
      P_USB_OTG_DINEPS[ep].DIEPINTx = diepint.d32;

      /* We assume that there is only one interrupt source for
         one endpoint at one time. */
      if (diepint.b.txfe) /* Disabled in USBDcontinueInTransfer */
        USBDcontinueInTransfer(ep);
      if (diepint.b.xfrc)
        USBDtransfer(ep, PID_IN);
    }
  }
}

static void DevSuspendHandler(void) {
  USB_OTG_GINTSTS_TypeDef gintsts;

  USBDsuspend();
  PWRreduce();

  /* Clear interrupt again. */
  gintsts.d32 = 0;
  gintsts.b.usbsusp = 1;
  P_USB_OTG_GREGS->GINTSTS = gintsts.d32;
}

static void DevWakeupHandler() {
  /* This function seems to be never called.
  PWRresume();
  USBDwakeup();
  */
}

static void DevSofHandler(void) {
  USB_OTG_DSTS_TypeDef    dsts;
  USB_OTG_DEPCTLx_TypeDef depctl;
  int i;

  dsts.d32 = P_USB_OTG_DREGS->DSTS;
  for (i = 1; i < EP_MAX_COUNT; ++i) {
    depctl.d32 = P_USB_OTG_DOUTEPS[i].DOEPCTLx;
    if (depctl.b.eptyp == ISOCHRONOUS_TRANSFER) {
      if (dsts.b.fnsof & 1)
        depctl.b.sd1pid_soddfrm = 1;
      else
        depctl.b.sd0pid_sevnfrm = 1;
      P_USB_OTG_DOUTEPS[i].DOEPCTLx = depctl.d32;
    }
  }

  USBDsof(dsts.b.fnsof);
}

void USBglobalInterruptHandler() {
  USB_OTG_GINTSTS_TypeDef interrupt_status;

  interrupt_status.d32 = P_USB_OTG_GREGS->GINTSTS;
  /* Ensure that we are in the device mode. */
  if (interrupt_status.b.cmod == 0) {
    interrupt_status.d32 &= P_USB_OTG_GREGS->GINTMSK;
    /* Clear pending interrupt flags. */
    P_USB_OTG_GREGS->GINTSTS = interrupt_status.d32;

    if (interrupt_status.b.usbrst)
      DevResetHandler();
    if (interrupt_status.b.enumdne)
      DevEnumerationDoneHandler();

    if (interrupt_status.b.rxflvl)
      DevRxFifoNonEmptylHandler();
    if (interrupt_status.b.oepint)
      DevOutEndPointTransactionHandler();
    if (interrupt_status.b.iepint)
      DevInEndPointTransactionHandler();

    if (interrupt_status.b.usbsusp)
      DevSuspendHandler();
    if (interrupt_status.b.wkuint)
      DevWakeupHandler();

    if (interrupt_status.b.sof)
      DevSofHandler();
  }
  else {
    /* We are in the host mode. */
    ErrorResetable(-1, 8);
  }
}
