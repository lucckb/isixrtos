#include <board_usb_def.h>
#include <usb_regs.h>
#include <usbd_configure.h>
#include <usbd_core.h>

#define USB_PULLUP_GPIO  xcat(GPIO, USB_PULLUP_GPIO_N)
#define USB_PULLUP_PIN   xcat(GPIO_Pin_, USB_PULLUP_PIN_N)
#define USB_PULLUP_RCC   xcat(RCC_APB2Periph_GPIO, USB_PULLUP_GPIO_N)
#define USB_PULLUP_OFF   (!(USB_PULLUP_ON))

/** Low level USB device initialization for STM32F102 and STM32F103 **/

/* Pull-up resistor control configuration
+-----------------------------------+--------------------------+
|   Constants in board_usb_def.h    |      Output state        |
+-------------------+---------------+------------+-------------+
| USB_PULLUP_DIRECT | USB_PULLUP_ON | Pull-up on | Pull-up off |
+-------------------+---------------+------------+-------------+
|         0         |       0       |     0      |      1      |
+-------------------+---------------+------------+-------------+
|         0         |       1       |     1      |      0      |
+-------------------+---------------+------------+-------------+
|         1         |       X       |     1      |      Z      |
+-------------------+---------------+------------+-------------+
*/

/* Control USB pull-up resistor.
    newState != 0 - pull-up resistor is on
    newState == 0 - pull-up resistor is off */
static void PullUpResistor(int newState) {
  if (USB_PULLUP_DIRECT) {
    if (newState) {  /* Output in high state (1), push-pull, 2 MHz */
      if (USB_PULLUP_PIN_N < 8)
        USB_PULLUP_GPIO->CRL &= ~(4U << (4 * USB_PULLUP_PIN_N));
      else
        USB_PULLUP_GPIO->CRH &= ~(4U << (4 * USB_PULLUP_PIN_N - 32));
    }
    else { /* Output in high impedance state (Z), open-drain, 2 MHz */
      if (USB_PULLUP_PIN_N < 8)
        USB_PULLUP_GPIO->CRL |= 4U << (4 * USB_PULLUP_PIN_N);
      else
        USB_PULLUP_GPIO->CRH |= 4U << (4 * USB_PULLUP_PIN_N - 32);
    }
  }
  else {
    if (newState)
      GPIO_WriteBit(USB_PULLUP_GPIO, USB_PULLUP_PIN, USB_PULLUP_ON);
    else
      GPIO_WriteBit(USB_PULLUP_GPIO, USB_PULLUP_PIN, USB_PULLUP_OFF);
  }
}

/* Configure USB pull-up resistor and set it off. Only full speed
   device is supported. */
static int PullUpConfigure(usb_speed_t speed) {
  if (speed != FULL_SPEED)
    return -1;

  RCC_APB2PeriphClockCmd(USB_PULLUP_RCC, ENABLE);

  if (USB_PULLUP_DIRECT) {
    /* Pull-up resistor must stay off in the configuration phase.
       First, configure pin in the analog input mode. */
    if (USB_PULLUP_PIN_N < 8)
      USB_PULLUP_GPIO->CRL &= ~(15U << (4 * USB_PULLUP_PIN_N));
    else
      USB_PULLUP_GPIO->CRH &= ~(15U << (4 * USB_PULLUP_PIN_N - 32));
    /* Next, set 1 in the output register - irrelevant in the analog
       input mode. */
    USB_PULLUP_GPIO->BSRR = USB_PULLUP_PIN;
    /* Finally, switch pin to open drain 2MHz output - pin goes to
       high impedance state (Z). */
    if (USB_PULLUP_PIN_N < 8)
      USB_PULLUP_GPIO->CRL |= 6U << (4 * USB_PULLUP_PIN_N);
    else
      USB_PULLUP_GPIO->CRH |= 6U << (4 * USB_PULLUP_PIN_N - 32);
  }
  else {
    GPIO_InitTypeDef GPIO_InitStruct;

    /* Pull-up resistor must stay off in the configuration phase. */
    PullUpResistor(0);

    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin = USB_PULLUP_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(USB_PULLUP_GPIO, &GPIO_InitStruct);
  }
  return 0;
}

/* USB core needs 48 MHz clock.
    clk - SYSCLK clock in MHz */
static int USBDclockConfigure(int clk) {
  if (clk == 48)
    RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_Div1);
  else if (clk == 72)
    RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);
  else
    return -1;
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
  return 0;
}

/* Configure interrupt controler.
    prio    - preemption priority
    subprio - service priority when the same prio */
static void USBDinterruptConfigure(unsigned prio, unsigned subprio) {
  NVIC_InitTypeDef NVIC_InitStruct;

  NVIC_InitStruct.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
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
  if (USBDclockConfigure(clk) < 0)
    return -1;
  USBDinterruptConfigure(prio, subprio);
  USBDperipheralConfigure();
  return 0;
}
