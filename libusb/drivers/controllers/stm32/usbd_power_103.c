#include <board_def.h>
#include <board_init.h>
#include <error.h>
#include <irq.h>
#include <timer.h>
#include <usb_regs.h>
#include <usbd_core.h>
#include <usbd_power.h>

/* NOTE: STM32F103 limitation: If clk == 72, the device cannot be
   woken up or reset and hangs when connected during OS boot. */

/** USB device power management for STM32F103 **/

#define RW_TIME_MS  6

static remote_wakeup_t remoteWakeUp = RW_DISABLED;
static enum {RW_IDLE = 0, RW_STARTED} state;

/* Configure power management. */
int PWRconfigure(unsigned prio, unsigned subprio, int clk) {
  EXTI_InitTypeDef EXTI_InitStruct;

  if (clk != 48)
    return -1;

  remoteWakeUp = RW_DISABLED;
  state = RW_IDLE;

  EXTI_ClearITPendingBit(EXTI_Line18);
  EXTI_InitStruct.EXTI_Line = EXTI_Line18; /* USB wakeup line */
  EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStruct.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStruct);

  IRQsetPriority(USBWakeUp_IRQn, prio, subprio);

  TimerConfigure(prio, subprio);

  return 0;
}

void PWRmanagementEnable() {
  uint16_t interruptMask;

  /* Clear pending power management interrupts. */
  _SetISTR(CLR_WKUP | CLR_SUSP);
  EXTI_ClearITPendingBit(EXTI_Line18);
  NVIC_ClearPendingIRQ(USBWakeUp_IRQn);

  /* Enable power management interrupts. */
  interruptMask = _GetCNTR();
  interruptMask |= CNTR_WKUPM | CNTR_SUSPM;
  _SetCNTR(interruptMask);
  NVIC_EnableIRQ(USBWakeUp_IRQn);
}

/* USB is going into the suspend mode, enter the power-saving mode. */
void PWRreduce() {
  uint16_t w;

  w = _GetCNTR();

  /* Enter the suspend mode in the USB macrocell. */
  w |= CNTR_FSUSP;
  _SetCNTR(w);

  /* Enter the low-power mode in the USB macrocell. */
  w |= CNTR_LPMODE;
  _SetCNTR(w);

  /* Enter the low-power mode in the microcontroller core. */
  do {
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
    /* Core is now clocked with HSI 8 MHz. */
  } while ((_GetFNR() & FNR_RXDP) && state == RW_IDLE);
  /* RXDP == 0, RXDM == 0 -- root reset
     RXDP == 0, RXDM == 1 -- root resume
     RDXP == 1            -- noise on bus */
}

/* USB is woken up, resume from the low-power mode. */
void PWRresume() {
  /* Leave the low-power mode in the USB macrocell. According to
     RM0008 the LP_MODE bit is cleared by hardware.
  _SetCNTR(_GetCNTR() & ~CNTR_LPMODE); */

  /* Restore the full performance of the microcontroller core. */
  ErrorResetable(ClockReenable(), 2);

  /* Leave the suspend mode in the USB macrocell. */
  _SetCNTR(_GetCNTR() & ~CNTR_FSUSP);
}

/* Get the remote wakeup status. */
remote_wakeup_t PWRgetRemoteWakeUp() {
  return remoteWakeUp;
}

/* Set the remote wakeup status. */
void PWRsetRemoteWakeUp(remote_wakeup_t rw) {
  remoteWakeUp = rw;
}

static void StartRemoteHostWakeupSignalling(void) {
  state = RW_STARTED;
  RedLEDon(); /* for diagnostic only */
  _SetCNTR(_GetCNTR() | CNTR_RESUME);
}

static void StopRemoteHostWakeupSignalling(void) {
  _SetCNTR(_GetCNTR() & ~CNTR_RESUME);
  RedLEDoff(); /* for diagnostic only */
  state = RW_IDLE;
}

/* Init remote wakeup of host. */
void PWRremoteWakeUp() {
  if (remoteWakeUp == RW_ENABLED && state == RW_IDLE) {
    /* Wake up the USB peripheral. */
    _SetCNTR(_GetCNTR() & ~CNTR_LPMODE);
    PWRresume();
    StartRemoteHostWakeupSignalling();
    TimerStart(1, StopRemoteHostWakeupSignalling, RW_TIME_MS);
    /* The same wakeup sequence should be called as in
       the USB_LP_CAN1_RX0_IRQHandler. */
    USBDwakeup();
  }
}
