#ifndef __ASM_LPC214X_VIC__
#define __ASM_LPC214X_VIC__

/*-----------------------------------------------------------------------*/
//Definition of priority interrupt
#define INTERRUPT_PRIO_DEFAULT -1
#define INTERRUPT_PRIO(n) n
#define INTERRUPT_PRIO_FIRST 0
#define INTERRUPT_PRIO_LAST 15
/*-----------------------------------------------------------------------*/
//LPC 214x Interrupt mask
#define INTERRUPT_NUM_WDT      0   /* Watchdog timer */
#define INTERRUPT_NUM_ICERX    2   /* ICE Rx  */
#define INTERRUPT_NUM_ICETX    3   /* ICE Tx  */
#define INTERRUPT_NUM_TIMER0   4   /* Timer 0 */
#define INTERRUPT_NUM_TIMER1   5   /* Timer 1 */
#define INTERRUPT_NUM_UART0    6   /* Uart 0  */
#define INTERRUPT_NUM_UART1    7   /* Uart 1  */
#define INTERRUPT_NUM_PWM0     8   /* PWM0    */
#define INTERRUPT_NUM_I2C0     9   /* I2C Controller 0   */
#define INTERRUPT_NUM_SPI0    10   /* SPI Controller 0   */
#define INTERRUPT_NUM_SPI1    11   /* SPI Controller 1   */
#define INTERRUPT_NUM_PLL     12   /* PLL loop */
#define INTERRUPT_NUM_RTC     13   /* Real Time Clock    */
#define INTERRUPT_NUM_EINT0   14   /* External Int0      */
#define INTERRUPT_NUM_EINT1   15   /* External Int1      */
#define INTERRUPT_NUM_EINT2   16   /* External Int2      */
#define INTERRUPT_NUM_EINT3   17   /* External Int3      */
#define INTERRUPT_NUM_ADC0    18   /* A/D 0    */
#define INTERRUPT_NUM_I2C1    19   /* I2C Controller 1   */
#define INTERRUPT_NUM_BOD     20   /* BOD detector       */
#define INTERRUPT_NUM_ADC1    21   /* A/D 1    */
#define INTERRUPT_NUM_USB     22   /* USB controller     */

#define VIC_CNTLREG_MASK 0x1f       /* Cntl register MASK */
#define VIC_CNTLREG_SLOTEN  (1<<5)  /* VIC IRQ Slot EN */

#endif
