/**
  ******************************************************************************
  * @file    stm32f10x_exti.h
  * @author  mcd application team
  * @version v3.5.0
  * @date    11-march-2011
  * @brief   this file contains all the functions prototypes for the exti firmware
  *          library.
  ******************************************************************************/
#pragma once


#define _STM32_EXTI_IMR_DEFVAL 0x1F800000
#define _STM32_EXTI_PR_DEFVAL  0x01FFFFFF


#define EXTI_Line0       ((uint32_t)0)  /*!< External interrupt line 0  */
#define EXTI_Line1       ((uint32_t)1)  /*!< External interrupt line 1  */
#define EXTI_Line2       ((uint32_t)2)  /*!< External interrupt line 2  */
#define EXTI_Line3       ((uint32_t)3)  /*!< External interrupt line 3  */
#define EXTI_Line4       ((uint32_t)4)  /*!< External interrupt line 4  */
#define EXTI_Line5       ((uint32_t)5)  /*!< External interrupt line 5  */
#define EXTI_Line6       ((uint32_t)6)  /*!< External interrupt line 6  */
#define EXTI_Line7       ((uint32_t)7)  /*!< External interrupt line 7  */
#define EXTI_Line8       ((uint32_t)8)  /*!< External interrupt line 8  */
#define EXTI_Line9       ((uint32_t)9)  /*!< External interrupt line 9  */
#define EXTI_Line10      ((uint32_t)10)  /*!< External interrupt line 10 */
#define EXTI_Line11      ((uint32_t)11)  /*!< External interrupt line 11 */
#define EXTI_Line12      ((uint32_t)12)  /*!< External interrupt line 12 */
#define EXTI_Line13      ((uint32_t)13)  /*!< External interrupt line 13 */
#define EXTI_Line14      ((uint32_t)14)  /*!< External interrupt line 14 */
#define EXTI_Line15      ((uint32_t)15)  /*!< External interrupt line 15 */
#define EXTI_Line16      ((uint32_t)16)  /*!< External interrupt line 16 
                                                      Connected to the PVD Output */
#define EXTI_Line17      ((uint32_t)17)  /*!< Internal interrupt line 17 
                                                      Connected to the RTC Alarm 
                                                      event */
#define EXTI_Line18      ((uint32_t)18)  /*!< Internal interrupt line 18 
                                                      Connected to the USB Device
                                                      Wakeup from suspend event */
#define EXTI_Line19      ((uint32_t)19)  /*!< Internal interrupt line 19
                                                      Connected to the RTC Tamper
                                                      and Time Stamp events */
#define EXTI_Line20      ((uint32_t)20)  /*!< Internal interrupt line 20
                                                      Connected to the RTC wakeup
                                                      event */
#define EXTI_Line21      ((uint32_t)21)  /*!< Internal interrupt line 21
                                                      Connected to the Comparator 1
                                                      event */
#define EXTI_Line22      ((uint32_t)22)  /*!< Internal interrupt line 22
                                                      Connected to the Comparator 2
                                                      event */
#define EXTI_Line23      ((uint32_t)23)  /*!< Internal interrupt line 23
                                                      Connected to the I2C1 wakeup
                                                      event */
#define EXTI_Line24      ((uint32_t)24)  /*!< Internal interrupt line 24
                                                      Connected to the I2C2 wakeup
                                                      event */
#define EXTI_Line25      ((uint32_t)25)  /*!< Internal interrupt line 25
                                                      Connected to the USART1 wakeup
                                                      event */
#define EXTI_Line26      ((uint32_t)26)  /*!< Internal interrupt line 26
                                                      Connected to the USART2 wakeup
                                                      event */
#define EXTI_Line27      ((uint32_t)27)  /*!< Internal interrupt line 27
                                                      Connected to the CEC wakeup
                                                      event */
#define EXTI_Line28      ((uint32_t)28)  /*!< Internal interrupt line 28
												   Connected to the USART3 wakeup
                                                      event */



