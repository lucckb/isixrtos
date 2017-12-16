#include <usb/drivers/controllers/stm32/usb_otg_regs.h>
#include <usb/drivers/controllers/stm32/usb_interrupt.h>
#include <usb/drivers/controllers/stm32/timer.h>
#include <usb/core/usbh_api.h>
#include <usb/core/usbh_core.h>
#include <usb/core/usbh_interrupt.h>
#include <usb/core/usbh_io.h>
#include <usb/core/usbh_lib.h>
#include <foundation/sys/dbglog.h>
#include <isix.h>
/* TrResult table is initialized with TR_UNDEF values. */
static usbh_transaction_result_t TrResult[CONST_CHNNL_MAX_COUNT];
static uint16_t DeadScheduleClocks = 65535;
static uint16_t CoreScheduleTime = 65535;

/** Diagnostic functions **/

/* Green LED is blinking when a device is connected and
   the host is working. */
static void Blink(int enable) {
	(void)enable;
}

/* DATA PID toggle error -- for diagnostic purpose only */
static inline void ToggleError(void) {

}

/** USB host interrupt handlers for STM32F105, STM32F107, STM32F205,
    STM32F207, STM32F215 and STM32F217 **/

static void StopSignallingPortReset(void* p) {
  (void)p;
  USB_OTG_HPRT_TypeDef hprt;
  hprt.d32 = P_USB_OTG_HREGS->HPRT;
  hprt.b.prst = 0;
  /* Do not clear any interrupt. */
  hprt.b.pocchng = 0;
  hprt.b.penchng = 0;
  hprt.b.pena = 0;
  hprt.b.pcdet = 0;
  P_USB_OTG_HREGS->HPRT = hprt.d32;
}

static void StartSignallingPortReset(void* p) {
  (void)p;
  USB_OTG_HPRT_TypeDef hprt;
  hprt.d32 = P_USB_OTG_HREGS->HPRT;
  hprt.b.prst = 1;
  /* Do not clear any interrupt. */
  hprt.b.pocchng = 0;
  hprt.b.penchng = 0;
  hprt.b.pena = 0;
  hprt.b.pcdet = 0;
  P_USB_OTG_HREGS->HPRT = hprt.d32;
  usblibp_timer_start(3, StopSignallingPortReset, RESET_TIME_MS);
}

static void HostDisconnectHandler(void* p) {
  (void)p;
#ifdef CONFIG_USBLIB_US_TIM_N 
  usblibp_fine_timer_stop(1);
#endif
  usblibp_timer_stop(1);
  StopSignallingPortReset(p);
  USBHdeviceDisconnected();
  Blink(0);
  //RedLEDon(); /* Red LED is on when there is no device connected. */
  DeadScheduleClocks = 65535;
  CoreScheduleTime = 65535;
}

static void HostPortHandler(void) {
  USB_OTG_HPRT_TypeDef hprt;
  USB_OTG_HCFG_TypeDef hcfg;

  hprt.d32 = P_USB_OTG_HREGS->HPRT;

  /* A device is attached to the host port.
     Configure bus clock and reset the device. */
  if (hprt.b.pcdet) {
    hcfg.d32 = 0;
    if (OTG_FS_REGS_USED)
      hcfg.b.fslspcs = hprt.b.pspd;
    else if (OTG_HS_REGS_USED)
      hcfg.b.fslspcs = 1;
    P_USB_OTG_HREGS->HCFG = hcfg.d32;
    USBHdeviceAttached();
    usblibp_timer_start(1, StartSignallingPortReset, STARTUP_TIME_MS);
    ///RedLEDoff(); /* Red LED id off when a device is connected. */
  }

  /* Reset of the device is finished. */
  if (hprt.b.penchng && hprt.b.pena && hprt.b.pcsts) {
    if (hprt.b.pspd == LOW_SPEED) {
      DeadScheduleClocks = 2 * HOST_LS_SCHEDULE_US * 6;
      CoreScheduleTime = HOST_LS_SCHEDULE_US;
    }
    else if (hprt.b.pspd == FULL_SPEED) {
      DeadScheduleClocks = 2 * HOST_FS_SCHEDULE_US * 48;
      CoreScheduleTime = HOST_FS_SCHEDULE_US;
    }
    else if (hprt.b.pspd == HIGH_SPEED) {
      DeadScheduleClocks = 2 * HOST_HS_SCHEDULE_US * 60;
      CoreScheduleTime = HOST_HS_SCHEDULE_US;
    }
    else {
      DeadScheduleClocks = 65535;
      CoreScheduleTime = 65535;
    }
    USBHdeviceSpeed(hprt.b.pspd);
    usblibp_timer_start(1, USBHdeviceResetDone, RECOVERY_TIME_MS);
  }

  /* The overcurrent on VBUS is detected -- never signalled here,
     because an external VBUS supply is used. */
  if (hprt.b.pocchng && hprt.b.poca) {
    USBHovercurrentInterruptHandler();
  }

  /* Clear interrupts. */
  hprt.b.pena = 0;
  P_USB_OTG_HREGS->HPRT = hprt.d32;
}

static void HostOneChannelHandler(int ch_num, int in) {
  USB_OTG_HCINTx_TypeDef    hcint;
  USB_OTG_HCINTMSKx_TypeDef hcintmsk;

  hcint.d32 = P_USB_OTG_HCHNNLS[ch_num].HCINTx;
  hcintmsk.d32 = P_USB_OTG_HCHNNLS[ch_num].HCINTMSKx;
  hcint.d32 &= hcintmsk.d32;

  /* Clear all signaled interrupts. */
  P_USB_OTG_HCHNNLS[ch_num].HCINTx = hcint.d32;

  /* The ack bit can be set only for OUT transaction.
     Bits ack and xfrc can be set simultaneously.
     The ack bit should be processed first. */
  if (!in && hcint.b.ack) {
    USBHpacketSent(ch_num);
  }
  /* Only one transfer result can be considered at the same time. */
  if (hcint.b.xfrc) {
    USBHhaltChannel(ch_num);
    TrResult[ch_num] = TR_DONE;
  }
  else if (hcint.b.stall) {
    USBHhaltChannel(ch_num);
    TrResult[ch_num] = TR_STALL;
  }
  else if (hcint.b.nak) {
    /* The channel can be reenabled immediately for control an bulk IN
       transfer. Isochronous endpoint never naks.
    USB_OTG_HCCHARx_TypeDef hcchar;

    hcchar.d32 = P_USB_OTG_HCHNNLS[ch_num].HCCHARx;
    if (in && (hcchar.b.eptyp == CONTROL_TRANSFER ||
               hcchar.b.eptyp == BULK_TRANSFER) {
      hcchar.b.chena = 1;
      hcchar.b.chdis = 0;
      P_USB_OTG_HCHNNLS[ch_num].HCCHARx = hcchar.d32;
    }
    else {
      USBHhaltChannel(ch_num);
      TrResult[ch_num] = TR_NAK;
    } */
    USBHhaltChannel(ch_num);
    TrResult[ch_num] = TR_NAK;
  }
  else if (hcint.b.txerr || hcint.b.bberr || hcint.b.frmor) {
    /* The bberr bit can be set only for IN transaction. */
    USBHhaltChannel(ch_num);
    TrResult[ch_num] = TR_ERROR;
  }
  else if (hcint.b.dterr) {
    ToggleError(); /* for diagnostic */
  }
  if (hcint.b.chh) {
    hcintmsk.b.chhm = 0;
    P_USB_OTG_HCHNNLS[ch_num].HCINTMSKx = hcintmsk.d32;
    /* Transfer is really finished only when channel is disabled. */
    USBHtransferFinished(ch_num, TrResult[ch_num]);
    TrResult[ch_num] = TR_UNDEF;
  }
}

static void HostChannelsHandler(void) {
  USB_OTG_HAINT_TypeDef   haint;
  USB_OTG_HCCHARx_TypeDef hcchar;
  int                     i;

  haint.d32 = P_USB_OTG_HREGS->HAINT;
  for (i = 0; i < (int)_USB_CHNNL_MAX_COUNT; ++i) {
    if (haint.b.haint & (1 << i)) {
      hcchar.d32 = P_USB_OTG_HCHNNLS[i].HCCHARx;
      HostOneChannelHandler(i, hcchar.b.epdir);
    }
  }
}

static void HostRxFifoNonEmptyHandler(void) {
  USB_OTG_GRXSTS_TypeDef  grxsts;
  USB_OTG_HCTSIZx_TypeDef hctsiz;
  USB_OTG_HCCHARx_TypeDef hcchar;
  uint32_t                ch_num;

  grxsts.d32 = P_USB_OTG_GREGS->GRXSTSP;
  ch_num = grxsts.b.ch_ep_num;

  if (grxsts.b.pktsts == GRXSTS_PKTSTS_IN_RECEIVED) {
    /* Read received packet to the host memory. */
    USBHpacketReceived(ch_num, grxsts.b.bcnt);

    /* Reactivate the channel when more packets are expected. */
    hctsiz.d32 = P_USB_OTG_HCHNNLS[ch_num].HCTSIZx;
    if (hctsiz.b.pktcnt > 0) {
      hcchar.d32 = P_USB_OTG_HCHNNLS[ch_num].HCCHARx;
      hcchar.b.chena = 1;
      hcchar.b.chdis = 0;
      P_USB_OTG_HCHNNLS[ch_num].HCCHARx = hcchar.d32;
    }
  }
  else if (grxsts.b.pktsts == GRXSTS_PKTSTS_DATA_TOGGLE_ERROR)
    ToggleError(); /* for diagnostic */
}

static void HostSofHandler(void) {
  USBHsof(USBHgetCurrentFrame());
  Blink(1); /* for diagnostic purpose */
}


#ifdef CONFIG_USBLIB_US_TIM_N
/* The USB global interrupt has higher subpriority
   than the fine-timer interrupt. */
static void CoreProcessHandler(void) {
  USBHcoreProcess();
  /* Recall the core process even if there is no pending USB interrupt,
     but do not call it just before the next start of frame. */
  if (USBHgetFrameClocksRemaining() >= DeadScheduleClocks)
    usblibp_fine_timer_start(1, CoreProcessHandler, CoreScheduleTime);
  else
    usblibp_fine_timer_stop(1);
}
#endif

void USBglobalInterruptHandler() {
  USB_OTG_GINTSTS_TypeDef interrupt_status;

  interrupt_status.d32 = P_USB_OTG_GREGS->GINTSTS;
  if (interrupt_status.b.cmod == 0) {
    /* We are in the device mode. */
	isix_bug("OTG controller in device mode");
  }
  else {
    /* We are in the host mode. */
    interrupt_status.d32 &= P_USB_OTG_GREGS->GINTMSK;
    /* Clear pending interrupt flags. */
    P_USB_OTG_GREGS->GINTSTS = interrupt_status.d32;

    /* Very important - serve discint before hprtint. */
    if (interrupt_status.b.discint)
      HostDisconnectHandler(NULL); /* device disconnected */
    if (interrupt_status.b.hprtint)
      HostPortHandler(); /* device connected */

    if (interrupt_status.b.hcint)
      HostChannelsHandler();
    if (interrupt_status.b.rxflvl)
      HostRxFifoNonEmptyHandler();

    if (interrupt_status.b.sof)
      HostSofHandler();
  }
#ifdef CONFIG_USBLIB_US_TIM_N
  /* Call the core process just after the interrupt is handled. */
  usblibp_fine_timer_start(1, CoreProcessHandler, 2);
#else
  USBHcoreProcess();
#endif 
}

/* Overcurrent is detected on VBUS. */
void USBHovercurrentInterruptHandler() {
  usbh_device_hard_reset(10 * DEVICE_RESET_TIME_MS);
}
