/*
 * =====================================================================================
 *
 *       Filename:  stm32f30x_exti.h
 *
 *    Description:  EXTI defs
 *
 *        Version:  1.0
 *        Created:  09.12.2017 21:34:55
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once


#define _STM32_EXTI_IMR_DEFVAL 0x1F800000
#define _STM32_EXTI_IMR2_DEFVAL 0x0000000C
#define _STM32_EXTI_PR_DEFVAL 0xE07FFFFF
#define _STM32_EXTI_PR2_DEFVAL 0x00000003

#define EXTI_Line0       ((uint32_t)0x00)  /*!< External interrupt line 0  */
#define EXTI_Line1       ((uint32_t)0x01)  /*!< External interrupt line 1  */
#define EXTI_Line2       ((uint32_t)0x02)  /*!< External interrupt line 2  */
#define EXTI_Line3       ((uint32_t)0x03)  /*!< External interrupt line 3  */
#define EXTI_Line4       ((uint32_t)0x04)  /*!< External interrupt line 4  */
#define EXTI_Line5       ((uint32_t)0x05)  /*!< External interrupt line 5  */
#define EXTI_Line6       ((uint32_t)0x06)  /*!< External interrupt line 6  */
#define EXTI_Line7       ((uint32_t)0x07)  /*!< External interrupt line 7  */
#define EXTI_Line8       ((uint32_t)0x08)  /*!< External interrupt line 8  */
#define EXTI_Line9       ((uint32_t)0x09)  /*!< External interrupt line 9  */
#define EXTI_Line10      ((uint32_t)0x0A)  /*!< External interrupt line 10 */
#define EXTI_Line11      ((uint32_t)0x0B)  /*!< External interrupt line 11 */
#define EXTI_Line12      ((uint32_t)0x0C)  /*!< External interrupt line 12 */
#define EXTI_Line13      ((uint32_t)0x0D)  /*!< External interrupt line 13 */
#define EXTI_Line14      ((uint32_t)0x0E)  /*!< External interrupt line 14 */
#define EXTI_Line15      ((uint32_t)0x0F)  /*!< External interrupt line 15 */
#define EXTI_Line16      ((uint32_t)0x10)  /*!< External interrupt line 16 
                                                      Connected to the PVD Output */
#define EXTI_Line17      ((uint32_t)0x11)  /*!< Internal interrupt line 17 
                                                      Connected to the RTC Alarm 
                                                      event */
#define EXTI_Line18      ((uint32_t)0x12)  /*!< Internal interrupt line 18 
                                                      Connected to the USB Device
                                                      Wakeup from suspend event */
#define EXTI_Line19      ((uint32_t)0x13)  /*!< Internal interrupt line 19
                                                      Connected to the RTC Tamper
                                                      and Time Stamp events */
#define EXTI_Line20      ((uint32_t)0x14)  /*!< Internal interrupt line 20
                                                      Connected to the RTC wakeup
                                                      event */ 
#define EXTI_Line21      ((uint32_t)0x15)  /*!< Internal interrupt line 21
                                                      Connected to the Comparator 1
                                                      event */
#define EXTI_Line22      ((uint32_t)0x16)  /*!< Internal interrupt line 22
                                                      Connected to the Comparator 2
                                                      event */
#define EXTI_Line23      ((uint32_t)0x17)  /*!< Internal interrupt line 23
                                                      Connected to the I2C1 wakeup
                                                      event */
#define EXTI_Line24      ((uint32_t)0x18)  /*!< Internal interrupt line 24
                                                      Connected to the I2C2 wakeup
                                                      event */
#define EXTI_Line25      ((uint32_t)0x19)  /*!< Internal interrupt line 25
                                                      Connected to the USART1 wakeup
                                                      event */
#define EXTI_Line26      ((uint32_t)0x1A)  /*!< Internal interrupt line 26
                                                      Connected to the USART2 wakeup
                                                      event */
#define EXTI_Line27      ((uint32_t)0x1B)  /*!< Internal interrupt line 27
                                                       reserved */
#define EXTI_Line28      ((uint32_t)0x1C)  /*!< Internal interrupt line 28
                                                      Connected to the USART3 wakeup
                                                      event */
#define EXTI_Line29      ((uint32_t)0x1D)  /*!< Internal interrupt line 29
                                                      Connected to the Comparator 3 
                                                      event */
#define EXTI_Line30      ((uint32_t)0x1E)  /*!< Internal interrupt line 30
                                                      Connected to the Comparator 4 
                                                      event */
#define EXTI_Line31      ((uint32_t)0x1F)  /*!< Internal interrupt line 31
                                                      Connected to the Comparator 5 
                                                      event */
#define EXTI_Line32      ((uint32_t)0x20)  /*!< Internal interrupt line 32
                                                      Connected to the Comparator 6 
                                                      event */
#define EXTI_Line33      ((uint32_t)0x21)  /*!< Internal interrupt line 33
                                                      Connected to the Comparator 7 
                                                      event */
#define EXTI_Line34      ((uint32_t)0x22)  /*!< Internal interrupt line 34
                                                      Connected to the USART4 wakeup
                                                      event */
#define EXTI_Line35      ((uint32_t)0x23)  /*!< Internal interrupt line 35
                                                      Connected to the USART5 wakeup
                                                      event */
                                                                       

