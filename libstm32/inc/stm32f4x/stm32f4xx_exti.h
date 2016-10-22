/**
  ******************************************************************************
  * @file    stm32f4xx_exti.h
  * @author  MCD Application Team
  * @version V1.0.2
  * @date    05-March-2012
  * @brief   This file contains all the functions prototypes for the EXTI firmware
  *          library.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */
#pragma once


#define _STM32_EXTI_IMR_DEFVAL 0x00000000
#define _STM32_EXTI_PR_DEFVAL  0x007FFFFF


#define EXTI_Line0       ((uint32_t)0)     /*!< External interrupt line 0 */
#define EXTI_Line1       ((uint32_t)1)     /*!< External interrupt line 1 */
#define EXTI_Line2       ((uint32_t)2)     /*!< External interrupt line 2 */
#define EXTI_Line3       ((uint32_t)3)     /*!< External interrupt line 3 */
#define EXTI_Line4       ((uint32_t)4)     /*!< External interrupt line 4 */
#define EXTI_Line5       ((uint32_t)5)     /*!< External interrupt line 5 */
#define EXTI_Line6       ((uint32_t)6)     /*!< External interrupt line 6 */
#define EXTI_Line7       ((uint32_t)7)     /*!< External interrupt line 7 */
#define EXTI_Line8       ((uint32_t)8)     /*!< External interrupt line 8 */
#define EXTI_Line9       ((uint32_t)9)     /*!< External interrupt line 9 */
#define EXTI_Line10      ((uint32_t)10)     /*!< External interrupt line 10 */
#define EXTI_Line11      ((uint32_t)11)     /*!< External interrupt line 11 */
#define EXTI_Line12      ((uint32_t)12)     /*!< External interrupt line 12 */
#define EXTI_Line13      ((uint32_t)13)     /*!< External interrupt line 13 */
#define EXTI_Line14      ((uint32_t)14)     /*!< External interrupt line 14 */
#define EXTI_Line15      ((uint32_t)15)     /*!< External interrupt line 15 */
#define EXTI_Line16      ((uint32_t)16)     /*!< External interrupt line 16 Connected to the PVD Output */
#define EXTI_Line17      ((uint32_t)17)     /*!< External interrupt line 17 Connected to the RTC Alarm event */
#define EXTI_Line18      ((uint32_t)18)     /*!< External interrupt line 18 Connected to the USB OTG FS Wakeup from suspend event */
#define EXTI_Line19      ((uint32_t)19)     /*!< External interrupt line 19 Connected to the Ethernet Wakeup event */
#define EXTI_Line20      ((uint32_t)20)  /*!< External interrupt line 20 Connected to the USB OTG HS (configured in FS) Wakeup event  */
#define EXTI_Line21      ((uint32_t)21)  /*!< External interrupt line 21 Connected to the RTC Tamper and Time Stamp events */
#define EXTI_Line22      ((uint32_t)22)  /*!< External interrupt line 22 Connected to the RTC Wakeup event */

