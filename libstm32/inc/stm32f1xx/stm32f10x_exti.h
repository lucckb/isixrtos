/**
  ******************************************************************************
  * @file    stm32f10x_exti.h
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    11-March-2011
  * @brief   This file contains all the functions prototypes for the EXTI firmware
  *          library.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */
#pragma once

#define _STM32_EXTI_IMR_DEFVAL 0x00000000
#define _STM32_EXTI_PR_DEFVAL  0x000FFFFF

#define EXTI_Line0       ((uint32_t)0)  /*!< External interrupt line 0 */
#define EXTI_Line1       ((uint32_t)1)  /*!< External interrupt line 1 */
#define EXTI_Line2       ((uint32_t)2)  /*!< External interrupt line 2 */
#define EXTI_Line3       ((uint32_t)3)  /*!< External interrupt line 3 */
#define EXTI_Line4       ((uint32_t)4)  /*!< External interrupt line 4 */
#define EXTI_Line5       ((uint32_t)5)  /*!< External interrupt line 5 */
#define EXTI_Line6       ((uint32_t)6)  /*!< External interrupt line 6 */
#define EXTI_Line7       ((uint32_t)7)  /*!< External interrupt line 7 */
#define EXTI_Line8       ((uint32_t)8)  /*!< External interrupt line 8 */
#define EXTI_Line9       ((uint32_t)9)  /*!< External interrupt line 9 */
#define EXTI_Line10      ((uint32_t)10)  /*!< External interrupt line 10 */
#define EXTI_Line11      ((uint32_t)11)  /*!< External interrupt line 11 */
#define EXTI_Line12      ((uint32_t)12)  /*!< External interrupt line 12 */
#define EXTI_Line13      ((uint32_t)13)  /*!< External interrupt line 13 */
#define EXTI_Line14      ((uint32_t)14)  /*!< External interrupt line 14 */
#define EXTI_Line15      ((uint32_t)15)  /*!< External interrupt line 15 */
#define EXTI_Line16      ((uint32_t)16)  /*!< External interrupt line 16 Connected to the PVD Output */
#define EXTI_Line17      ((uint32_t)17)  /*!< External interrupt line 17 Connected to the RTC Alarm event */
#define EXTI_Line18      ((uint32_t)18)  /*!< External interrupt line 18 Connected to the USB Device/USB OTG FS
                                                   Wakeup from suspend event */
#define EXTI_Line19      ((uint32_t)19)  /*!< External interrupt line 19 Connected to the Ethernet Wakeup event */


