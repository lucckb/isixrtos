#include <usb_interrupt.h>
#include <usb_regs.h>
#include <usbd_api.h>
#include <usbd_core.h>
#include <usbd_power.h>

/** USB device interrupt handlers for STM32F102, STM32F103 and
    STM32L1xx **/

void USBglobalInterruptHandler() {
  uint16_t pending;

  pending = _GetCNTR() & _GetISTR();
  if (pending & ISTR_CTR) {
    uint16_t EPindex, EPflags, EPnum;

    EPindex = _GetISTR() & ISTR_EP_ID;
    EPflags = _GetENDPOINT(EPindex);
    EPnum   = EPflags & EPADDR_FIELD;
    /* The EP_SETUP bit implies the EP_CTR_RX bit. */
    if (EPflags & EP_SETUP) {
      _ClearEP_CTR_RX(EPindex);
      USBDtransfer(EPnum, PID_SETUP);
    }
    else if (EPflags & EP_CTR_RX) {
      _ClearEP_CTR_RX(EPindex);
      USBDtransfer(EPnum, PID_OUT);
    }
    else if (EPflags & EP_CTR_TX) {
      _ClearEP_CTR_TX(EPindex);
      if (EPnum != 0)
        USBDcontinueInTransfer(EPnum);
      USBDtransfer(EPnum, PID_IN);
    }
  }
  if (pending & ISTR_DOVR) {
    _SetISTR(CLR_DOVR);
  }
  if (pending & ISTR_ERR) {
    _SetISTR(CLR_ERR);
  }
  if (pending & ISTR_SUSP) {
    USBDsuspend();
    PWRreduce();
    /* The ISTR_SUSP bit must be cleared after
       setting of the CNTR_FSUSP bit. */
    _SetISTR(CLR_SUSP);
  }
  if (pending & ISTR_WKUP) {
    _SetISTR(CLR_WKUP);
    PWRresume();
    USBDwakeup();
  }
  if (pending & ISTR_RESET) {
    _SetISTR(CLR_RESET);
    USBDreset(FULL_SPEED);
  }
  if (pending & ISTR_SOF) {
    _SetISTR(CLR_SOF);
    USBDsof(_GetFNR() & FNR_FN);
  }
  if (pending & ISTR_ESOF) {
    _SetISTR(CLR_ESOF);
  }
}
