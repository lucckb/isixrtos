#include <board_usb_def.h>
#include <usb_regs.h>
#include <usbd_configure.h>
#include <usbd_core.h>

#if USB_PULLUP_DIRECT == 0
  /* Dummy definitions to avoid compiler error */
  #ifndef USB_PULLUP_GPIO_N
    #define USB_PULLUP_GPIO_N  D
  #endif
  #ifndef USB_PULLUP_PIN_N
    #define USB_PULLUP_PIN_N  15
  #endif
#endif

#define USB_PULLUP_GPIO  xcat(GPIO, USB_PULLUP_GPIO_N)
#define USB_PULLUP_PIN   xcat(GPIO_Pin_, USB_PULLUP_PIN_N)
#define USB_PULLUP_RCC   xcat(RCC_AHBPeriph_GPIO, USB_PULLUP_GPIO_N)

/** Low level USB device initialization for STM32L151 and STM32L152 **/

/* Control USB pull-up resistor.
    newState != 0 - pull-up resistor is on
    newState == 0 - pull-up resistor is off */
static void PullUpResistor(int newState) {
  if (USB_PULLUP_DIRECT) {
    if (newState)  /* Output in high state (1) */
      USB_PULLUP_GPIO->OTYPER &= ~(1 << USB_PULLUP_PIN_N);
    else /* Output in high impedance state (Z) */
      USB_PULLUP_GPIO->OTYPER |= 1 << USB_PULLUP_PIN_N;
  }
  else {
    /* Use internal pull-up resistor. According to "STM32L151xx and
       STM32L152xx Errata sheet" it has a value between 0.81 and 0.95
       kiloohm. */
    SYSCFG_USBPuCmd(newState ? ENABLE : DISABLE);
  }
}

/* Configure USB pull-up resistor and set it off. Only full speed
   device is supported. */
static int PullUpConfigure(usb_speed_t speed) {
  if (speed != FULL_SPEED)
    return -1;

  /* Pull-up resistor must stay off in the configuration phase. */
  if (USB_PULLUP_DIRECT) {
    GPIO_InitTypeDef GPIO_InitStruct;

    RCC_AHBPeriphClockCmd(USB_PULLUP_RCC, ENABLE);

    GPIO_WriteBit(USB_PULLUP_GPIO, USB_PULLUP_PIN, 1);

    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin = USB_PULLUP_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_400KHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(USB_PULLUP_GPIO, &GPIO_InitStruct);
  }
  else { /* Use internal pull-up resistor. */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    PullUpResistor(0);
  }

  return 0;
}

/* Configure PA11 (DM) and PA12 (DP) for USB. Not needed. */
static void USBDioConfigure(void) {
  /*
  GPIO_InitTypeDef GPIO_InitStruct;

  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_40MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_USB);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_USB);
  */
}

/* USB core needs 48 MHz clock.
    clk - SYSCLK clock in MHz */
static int USBDclockConfigure(int clk) {
  if (clk != 24 && clk != 32)
    return -1;

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);

  return 0;
}

/* Configure interrupt controler.
    prio    - preemption priority
    subprio - service priority when the same prio */
static void USBDinterruptConfigure(unsigned prio, unsigned subprio) {
  NVIC_InitTypeDef NVIC_InitStruct;

  NVIC_InitStruct.NVIC_IRQChannel = USB_LP_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = prio;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = subprio;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);
}

/* Switch USB hardware on and configure interrupt mask. */
static void USBDperipheralConfigure(void) {
  /* Disable all interrupts and reset USB device. */
  _SetCNTR(CNTR_FRES);
  /* Bits PDWN and FRES are set after microcontroller reset - the CNTR
     register is initialized with the value (CNTR_PDWN | CNTR_FRES) ==
     0x0003. The device is in the reset state as long as the CNTR_FRES
     bit is set. It is necessary to wait t_STARTUP time, before
     clearing the PDWN and FRES bits. Assume, that this time is
     elapsed, e.g., during clock and PLLs initialization. */
  _SetCNTR(0);
  /* Clear pending interrupts. */
  _SetISTR(0);
  /* Set disired interrupt mask. */
  _SetCNTR(CNTR_CTRM | CNTR_RESETM | CNTR_SOFM);
  /* Simulate cable plug in. */
  PullUpResistor(1);
}

/** USB device initialization API **/

/* Call the USBpreConfigure function as early as possible after
   microcontroller reset. The main configuration is provided later
   in the USBconfigure function. Phy transceiver is integrated. */
int USBDpreConfigure(usb_speed_t speed, usb_phy_t phy) {
  return PullUpConfigure(speed);
}

/* Configure USB interface.
    prio    - preemption priority
    subprio - service priority when the same prio
    clk     - SYSCLK clock in MHz */
int USBDconfigure(unsigned prio, unsigned subprio, int clk) {
  if (USBDcoreConfigure() < 0)
    return -1;
  USBDioConfigure();
  if (USBDclockConfigure(clk) < 0)
    return -1;
  USBDinterruptConfigure(prio, subprio);
  USBDperipheralConfigure();
  return 0;
}
