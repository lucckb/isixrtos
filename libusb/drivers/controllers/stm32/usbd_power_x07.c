#include <board_usb_def.h>
#include <board_init.h>
#include <error.h>
#include <timer.h>
#include <usb_otg_regs.h>
#include <usbd_core.h>
#include <usbd_power.h>

/** USB device power management for STM32F105, STM32F107,
    STM32F2xx and STMF4xx **/

#define RW_TIME_MS  6

static int sysclkMHz = 0;
static remote_wakeup_t remoteWakeUp = RW_DISABLED;
static enum {RW_IDLE = 0, RW_INITIATED, RW_STARTED} state;

/* Configure power management. */
int PWRconfigure(unsigned prio, unsigned subprio, int clk) {
  NVIC_InitTypeDef NVIC_InitStruct;
  EXTI_InitTypeDef EXTI_InitStruct;

  sysclkMHz = clk;
  remoteWakeUp = RW_DISABLED;
  state = RW_IDLE;

  EXTI_ClearITPendingBit(EXTI_Line18);
  EXTI_InitStruct.EXTI_Line = EXTI_Line18; /* USB wakeup line */
  EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStruct.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStruct);

  NVIC_InitStruct.NVIC_IRQChannel = OTG_FS_WKUP_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = prio;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = subprio;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);

  TimerConfigure(prio, subprio);

  return 0;
}

void PWRmanagementEnable() {
  USB_OTG_GINTSTS_TypeDef gintsts;
  USB_OTG_GINTMSK_TypeDef gintmsk;

  /* Clear pending power management interrupts. */
  gintsts.d32 = 0;
  gintsts.b.usbsusp = 1;
  gintsts.b.wkuint = 1;
  P_USB_OTG_GREGS->GINTSTS = gintsts.d32;

  /* Enable power management interrupts. */
  gintmsk.d32 = P_USB_OTG_GREGS->GINTMSK;
  gintmsk.b.usbsuspm = 1;
  gintmsk.b.wkuim = 1;
  P_USB_OTG_GREGS->GINTMSK = gintmsk.d32;
}

static void StartRemoteHostWakeupSignalling(void) {
  USB_OTG_DCTL_TypeDef dctl;

  state = RW_STARTED;
  RedLEDon(); /* for diagnostic only */

  dctl.d32 = P_USB_OTG_DREGS->DCTL;
  dctl.b.rwusig = 1;
  P_USB_OTG_DREGS->DCTL = dctl.d32;
}

static void StopRemoteHostWakeupSignalling(void) {
  USB_OTG_DCTL_TypeDef dctl;

  dctl.d32 = P_USB_OTG_DREGS->DCTL;
  dctl.b.rwusig = 0;
  P_USB_OTG_DREGS->DCTL = dctl.d32;

  RedLEDoff(); /* for diagnostic only */
  state = RW_IDLE;
}

/* USB is going in the suspend mode. */
void PWRreduce() {
  USB_OTG_PCGCCTL_TypeDef pcgcctl;
  USB_OTG_DCTL_TypeDef    dctl;

  /* Enter the low-power mode. */
  pcgcctl.d32 = 0;
  pcgcctl.b.stppclk = 1;
  pcgcctl.b.gatehclk = 1;
  pcgcctl.b.physusp = 1;
  P_USB_OTG_PREGS->PCGCCTL = pcgcctl.d32;
  PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
  /* Now the device sleeps.
     NOTE for ZL29ARM: If USB cable is disconnected, the device snooze
     and then it is immediately woken up, because PA9 is connected to
     VBUS line of the host socket (A type). PA9 should be switched to
     BUS line of the device socket (B type), when USB operates in the
     device mode. */

  /* Now the device is woken up. Restore the full performance of the
     microcontroller. Switch on power and clocking of the USB core. */
  ErrorResetable(ClockConfigure(sysclkMHz), 2);
  pcgcctl.d32 = 0;
  P_USB_OTG_PREGS->PCGCCTL = pcgcctl.d32;

  /* Power up procedure is done. */
  dctl.d32 = P_USB_OTG_DREGS->DCTL;
  dctl.b.poprgdne = 1;
  P_USB_OTG_DREGS->DCTL = dctl.d32;

  /* Start remote host wakeup if desired. */
  if (state == RW_INITIATED) {
    StartRemoteHostWakeupSignalling();
    TimerStart(1, StopRemoteHostWakeupSignalling, RW_TIME_MS);
  }

  /* Finish the wakeup call sequence. */
  USBDwakeup();
}

/* USB is woken up. It seems to be never called. */
void PWRresume() {
}

/* Get the remote wakeup status. */
remote_wakeup_t PWRgetRemoteWakeUp() {
  return remoteWakeUp;
}

/* Set the remote wakeup status. */
void PWRsetRemoteWakeUp(remote_wakeup_t rw) {
  remoteWakeUp = rw;
}

/* Init remote wakeup of host. */
void PWRremoteWakeUp() {
  if (remoteWakeUp == RW_ENABLED && state == RW_IDLE)
    state = RW_INITIATED;
}
