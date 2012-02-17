/**
  ******************************************************************************
  * @file    stm32f10x_rcc.h
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    11-March-2011
  * @brief   This file contains all the functions prototypes for the RCC firmware 
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F10x_RCC_H
#define __STM32F10x_RCC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"


/** @defgroup RCC_Exported_Constants
  * @{
  */

/** @defgroup HSE_configuration 
  * @{
  */

#define RCC_HSE_OFF                      ((uint32_t)0x00000000)
#define RCC_HSE_ON                       ((uint32_t)0x00010000)
#define RCC_HSE_Bypass                   ((uint32_t)0x00040000)

/**
  * @}
  */ 

/** @defgroup PLL_entry_clock_source 
  * @{
  */

#define RCC_PLLSource_HSI_Div2           ((uint32_t)0x00000000)

#if !defined (STM32F10X_LD_VL) && !defined (STM32F10X_MD_VL) && !defined (STM32F10X_HD_VL) && !defined (STM32F10X_CL)
 #define RCC_PLLSource_HSE_Div1           ((uint32_t)0x00010000)
 #define RCC_PLLSource_HSE_Div2           ((uint32_t)0x00030000)
 #define IS_RCC_PLL_SOURCE(SOURCE) (((SOURCE) == RCC_PLLSource_HSI_Div2) || \
                                   ((SOURCE) == RCC_PLLSource_HSE_Div1) || \
                                   ((SOURCE) == RCC_PLLSource_HSE_Div2))
#else
 #define RCC_PLLSource_PREDIV1            ((uint32_t)0x00010000)

#endif /* STM32F10X_CL */ 

/**
  * @}
  */ 

/** @defgroup PLL_multiplication_factor 
  * @{
  */
#ifndef STM32F10X_CL
 #define RCC_PLLMul_2                    ((uint32_t)0x00000000)
 #define RCC_PLLMul_3                    ((uint32_t)0x00040000)
 #define RCC_PLLMul_4                    ((uint32_t)0x00080000)
 #define RCC_PLLMul_5                    ((uint32_t)0x000C0000)
 #define RCC_PLLMul_6                    ((uint32_t)0x00100000)
 #define RCC_PLLMul_7                    ((uint32_t)0x00140000)
 #define RCC_PLLMul_8                    ((uint32_t)0x00180000)
 #define RCC_PLLMul_9                    ((uint32_t)0x001C0000)
 #define RCC_PLLMul_10                   ((uint32_t)0x00200000)
 #define RCC_PLLMul_11                   ((uint32_t)0x00240000)
 #define RCC_PLLMul_12                   ((uint32_t)0x00280000)
 #define RCC_PLLMul_13                   ((uint32_t)0x002C0000)
 #define RCC_PLLMul_14                   ((uint32_t)0x00300000)
 #define RCC_PLLMul_15                   ((uint32_t)0x00340000)
 #define RCC_PLLMul_16                   ((uint32_t)0x00380000)


#else
 #define RCC_PLLMul_4                    ((uint32_t)0x00080000)
 #define RCC_PLLMul_5                    ((uint32_t)0x000C0000)
 #define RCC_PLLMul_6                    ((uint32_t)0x00100000)
 #define RCC_PLLMul_7                    ((uint32_t)0x00140000)
 #define RCC_PLLMul_8                    ((uint32_t)0x00180000)
 #define RCC_PLLMul_9                    ((uint32_t)0x001C0000)
 #define RCC_PLLMul_6_5                  ((uint32_t)0x00340000)


#endif /* STM32F10X_CL */                              
/**
  * @}
  */

/** @defgroup PREDIV1_division_factor
  * @{
  */
#if defined (STM32F10X_LD_VL) || defined (STM32F10X_MD_VL) || defined (STM32F10X_HD_VL) || defined (STM32F10X_CL)
 #define  RCC_PREDIV1_Div1               ((uint32_t)0x00000000)
 #define  RCC_PREDIV1_Div2               ((uint32_t)0x00000001)
 #define  RCC_PREDIV1_Div3               ((uint32_t)0x00000002)
 #define  RCC_PREDIV1_Div4               ((uint32_t)0x00000003)
 #define  RCC_PREDIV1_Div5               ((uint32_t)0x00000004)
 #define  RCC_PREDIV1_Div6               ((uint32_t)0x00000005)
 #define  RCC_PREDIV1_Div7               ((uint32_t)0x00000006)
 #define  RCC_PREDIV1_Div8               ((uint32_t)0x00000007)
 #define  RCC_PREDIV1_Div9               ((uint32_t)0x00000008)
 #define  RCC_PREDIV1_Div10              ((uint32_t)0x00000009)
 #define  RCC_PREDIV1_Div11              ((uint32_t)0x0000000A)
 #define  RCC_PREDIV1_Div12              ((uint32_t)0x0000000B)
 #define  RCC_PREDIV1_Div13              ((uint32_t)0x0000000C)
 #define  RCC_PREDIV1_Div14              ((uint32_t)0x0000000D)
 #define  RCC_PREDIV1_Div15              ((uint32_t)0x0000000E)
 #define  RCC_PREDIV1_Div16              ((uint32_t)0x0000000F)


#endif
/**
  * @}
  */


/** @defgroup PREDIV1_clock_source
  * @{
  */
#ifdef STM32F10X_CL
/* PREDIV1 clock source (for STM32 connectivity line devices) */
 #define  RCC_PREDIV1_Source_HSE         ((uint32_t)0x00000000) 
 #define  RCC_PREDIV1_Source_PLL2        ((uint32_t)0x00010000) 

#elif defined (STM32F10X_LD_VL) || defined (STM32F10X_MD_VL) || defined (STM32F10X_HD_VL)
/* PREDIV1 clock source (for STM32 Value line devices) */
 #define  RCC_PREDIV1_Source_HSE         ((uint32_t)0x00000000) 

#endif
/**
  * @}
  */

#ifdef STM32F10X_CL
/** @defgroup PREDIV2_division_factor
  * @{
  */
  
 #define  RCC_PREDIV2_Div1               ((uint32_t)0x00000000)
 #define  RCC_PREDIV2_Div2               ((uint32_t)0x00000010)
 #define  RCC_PREDIV2_Div3               ((uint32_t)0x00000020)
 #define  RCC_PREDIV2_Div4               ((uint32_t)0x00000030)
 #define  RCC_PREDIV2_Div5               ((uint32_t)0x00000040)
 #define  RCC_PREDIV2_Div6               ((uint32_t)0x00000050)
 #define  RCC_PREDIV2_Div7               ((uint32_t)0x00000060)
 #define  RCC_PREDIV2_Div8               ((uint32_t)0x00000070)
 #define  RCC_PREDIV2_Div9               ((uint32_t)0x00000080)
 #define  RCC_PREDIV2_Div10              ((uint32_t)0x00000090)
 #define  RCC_PREDIV2_Div11              ((uint32_t)0x000000A0)
 #define  RCC_PREDIV2_Div12              ((uint32_t)0x000000B0)
 #define  RCC_PREDIV2_Div13              ((uint32_t)0x000000C0)
 #define  RCC_PREDIV2_Div14              ((uint32_t)0x000000D0)
 #define  RCC_PREDIV2_Div15              ((uint32_t)0x000000E0)
 #define  RCC_PREDIV2_Div16              ((uint32_t)0x000000F0)


/**
  * @}
  */


/** @defgroup PLL2_multiplication_factor
  * @{
  */
  
 #define  RCC_PLL2Mul_8                  ((uint32_t)0x00000600)
 #define  RCC_PLL2Mul_9                  ((uint32_t)0x00000700)
 #define  RCC_PLL2Mul_10                 ((uint32_t)0x00000800)
 #define  RCC_PLL2Mul_11                 ((uint32_t)0x00000900)
 #define  RCC_PLL2Mul_12                 ((uint32_t)0x00000A00)
 #define  RCC_PLL2Mul_13                 ((uint32_t)0x00000B00)
 #define  RCC_PLL2Mul_14                 ((uint32_t)0x00000C00)
 #define  RCC_PLL2Mul_16                 ((uint32_t)0x00000E00)
 #define  RCC_PLL2Mul_20                 ((uint32_t)0x00000F00)


/**
  * @}
  */


/** @defgroup PLL3_multiplication_factor
  * @{
  */

 #define  RCC_PLL3Mul_8                  ((uint32_t)0x00006000)
 #define  RCC_PLL3Mul_9                  ((uint32_t)0x00007000)
 #define  RCC_PLL3Mul_10                 ((uint32_t)0x00008000)
 #define  RCC_PLL3Mul_11                 ((uint32_t)0x00009000)
 #define  RCC_PLL3Mul_12                 ((uint32_t)0x0000A000)
 #define  RCC_PLL3Mul_13                 ((uint32_t)0x0000B000)
 #define  RCC_PLL3Mul_14                 ((uint32_t)0x0000C000)
 #define  RCC_PLL3Mul_16                 ((uint32_t)0x0000E000)
 #define  RCC_PLL3Mul_20                 ((uint32_t)0x0000F000)

/**
  * @}
  */

#endif /* STM32F10X_CL */


/** @defgroup System_clock_source 
  * @{
  */

#define RCC_SYSCLKSource_HSI             ((uint32_t)0x00000000)
#define RCC_SYSCLKSource_HSE             ((uint32_t)0x00000001)
#define RCC_SYSCLKSource_PLLCLK          ((uint32_t)0x00000002)

/**
  * @}
  */

/** @defgroup AHB_clock_source 
  * @{
  */

#define RCC_SYSCLK_Div1                  ((uint32_t)0x00000000)
#define RCC_SYSCLK_Div2                  ((uint32_t)0x00000080)
#define RCC_SYSCLK_Div4                  ((uint32_t)0x00000090)
#define RCC_SYSCLK_Div8                  ((uint32_t)0x000000A0)
#define RCC_SYSCLK_Div16                 ((uint32_t)0x000000B0)
#define RCC_SYSCLK_Div64                 ((uint32_t)0x000000C0)
#define RCC_SYSCLK_Div128                ((uint32_t)0x000000D0)
#define RCC_SYSCLK_Div256                ((uint32_t)0x000000E0)
#define RCC_SYSCLK_Div512                ((uint32_t)0x000000F0)

/**
  * @}
  */ 

/** @defgroup APB1_APB2_clock_source 
  * @{
  */

#define RCC_HCLK_Div1                    ((uint32_t)0x00000000)
#define RCC_HCLK_Div2                    ((uint32_t)0x00000400)
#define RCC_HCLK_Div4                    ((uint32_t)0x00000500)
#define RCC_HCLK_Div8                    ((uint32_t)0x00000600)
#define RCC_HCLK_Div16                   ((uint32_t)0x00000700)

/**
  * @}
  */

/** @defgroup RCC_Interrupt_source 
  * @{
  */

#define RCC_IT_LSIRDY                    ((uint8_t)0x01)
#define RCC_IT_LSERDY                    ((uint8_t)0x02)
#define RCC_IT_HSIRDY                    ((uint8_t)0x04)
#define RCC_IT_HSERDY                    ((uint8_t)0x08)
#define RCC_IT_PLLRDY                    ((uint8_t)0x10)
#define RCC_IT_CSS                       ((uint8_t)0x80)

#ifndef STM32F10X_CL

#else
 #define RCC_IT_PLL2RDY                  ((uint8_t)0x20)
 #define RCC_IT_PLL3RDY                  ((uint8_t)0x40)


#endif /* STM32F10X_CL */ 


/**
  * @}
  */

#ifndef STM32F10X_CL
/** @defgroup USB_Device_clock_source 
  * @{
  */

 #define RCC_USBCLKSource_PLLCLK_1Div5   ((uint8_t)0x00)
 #define RCC_USBCLKSource_PLLCLK_Div1    ((uint8_t)0x01)

/**
  * @}
  */
#else
/** @defgroup USB_OTG_FS_clock_source 
  * @{
  */
 #define RCC_OTGFSCLKSource_PLLVCO_Div3    ((uint8_t)0x00)
 #define RCC_OTGFSCLKSource_PLLVCO_Div2    ((uint8_t)0x01)

/**
  * @}
  */
#endif /* STM32F10X_CL */ 


#ifdef STM32F10X_CL
/** @defgroup I2S2_clock_source 
  * @{
  */
 #define RCC_I2S2CLKSource_SYSCLK        ((uint8_t)0x00)
 #define RCC_I2S2CLKSource_PLL3_VCO      ((uint8_t)0x01)


/**
  * @}
  */

/** @defgroup I2S3_clock_source 
  * @{
  */
 #define RCC_I2S3CLKSource_SYSCLK        ((uint8_t)0x00)
 #define RCC_I2S3CLKSource_PLL3_VCO      ((uint8_t)0x01)

/**
  * @}
  */
#endif /* STM32F10X_CL */  
  

/** @defgroup ADC_clock_source 
  * @{
  */

#define RCC_PCLK2_Div2                   ((uint32_t)0x00000000)
#define RCC_PCLK2_Div4                   ((uint32_t)0x00004000)
#define RCC_PCLK2_Div6                   ((uint32_t)0x00008000)
#define RCC_PCLK2_Div8                   ((uint32_t)0x0000C000)

/**
  * @}
  */

/** @defgroup LSE_configuration 
  * @{
  */

#define RCC_LSE_OFF                      ((uint8_t)0x00)
#define RCC_LSE_ON                       ((uint8_t)0x01)
#define RCC_LSE_Bypass                   ((uint8_t)0x04)

/**
  * @}
  */

/** @defgroup RTC_clock_source 
  * @{
  */

#define RCC_RTCCLKSource_LSE             ((uint32_t)0x00000100)
#define RCC_RTCCLKSource_LSI             ((uint32_t)0x00000200)
#define RCC_RTCCLKSource_HSE_Div128      ((uint32_t)0x00000300)

/**
  * @}
  */

/** @defgroup AHB_peripheral 
  * @{
  */

#define RCC_AHBPeriph_DMA1               ((uint32_t)0x00000001)
#define RCC_AHBPeriph_DMA2               ((uint32_t)0x00000002)
#define RCC_AHBPeriph_SRAM               ((uint32_t)0x00000004)
#define RCC_AHBPeriph_FLITF              ((uint32_t)0x00000010)
#define RCC_AHBPeriph_CRC                ((uint32_t)0x00000040)

#ifndef STM32F10X_CL
 #define RCC_AHBPeriph_FSMC              ((uint32_t)0x00000100)
 #define RCC_AHBPeriph_SDIO              ((uint32_t)0x00000400)
#else
 #define RCC_AHBPeriph_OTG_FS            ((uint32_t)0x00001000)
 #define RCC_AHBPeriph_ETH_MAC           ((uint32_t)0x00004000)
 #define RCC_AHBPeriph_ETH_MAC_Tx        ((uint32_t)0x00008000)
 #define RCC_AHBPeriph_ETH_MAC_Rx        ((uint32_t)0x00010000)


#endif /* STM32F10X_CL */
/**
  * @}
  */

/** @defgroup APB2_peripheral 
  * @{
  */

#define RCC_APB2Periph_AFIO              ((uint32_t)0x00000001)
#define RCC_APB2Periph_GPIOA             ((uint32_t)0x00000004)
#define RCC_APB2Periph_GPIOB             ((uint32_t)0x00000008)
#define RCC_APB2Periph_GPIOC             ((uint32_t)0x00000010)
#define RCC_APB2Periph_GPIOD             ((uint32_t)0x00000020)
#define RCC_APB2Periph_GPIOE             ((uint32_t)0x00000040)
#define RCC_APB2Periph_GPIOF             ((uint32_t)0x00000080)
#define RCC_APB2Periph_GPIOG             ((uint32_t)0x00000100)
#define RCC_APB2Periph_ADC1              ((uint32_t)0x00000200)
#define RCC_APB2Periph_ADC2              ((uint32_t)0x00000400)
#define RCC_APB2Periph_TIM1              ((uint32_t)0x00000800)
#define RCC_APB2Periph_SPI1              ((uint32_t)0x00001000)
#define RCC_APB2Periph_TIM8              ((uint32_t)0x00002000)
#define RCC_APB2Periph_USART1            ((uint32_t)0x00004000)
#define RCC_APB2Periph_ADC3              ((uint32_t)0x00008000)
#define RCC_APB2Periph_TIM15             ((uint32_t)0x00010000)
#define RCC_APB2Periph_TIM16             ((uint32_t)0x00020000)
#define RCC_APB2Periph_TIM17             ((uint32_t)0x00040000)
#define RCC_APB2Periph_TIM9              ((uint32_t)0x00080000)
#define RCC_APB2Periph_TIM10             ((uint32_t)0x00100000)
#define RCC_APB2Periph_TIM11             ((uint32_t)0x00200000)

#define IS_RCC_APB2_PERIPH(PERIPH) ((((PERIPH) & 0xFFC00002) == 0x00) && ((PERIPH) != 0x00))
/**
  * @}
  */ 

/** @defgroup APB1_peripheral 
  * @{
  */

#define RCC_APB1Periph_TIM2              ((uint32_t)0x00000001)
#define RCC_APB1Periph_TIM3              ((uint32_t)0x00000002)
#define RCC_APB1Periph_TIM4              ((uint32_t)0x00000004)
#define RCC_APB1Periph_TIM5              ((uint32_t)0x00000008)
#define RCC_APB1Periph_TIM6              ((uint32_t)0x00000010)
#define RCC_APB1Periph_TIM7              ((uint32_t)0x00000020)
#define RCC_APB1Periph_TIM12             ((uint32_t)0x00000040)
#define RCC_APB1Periph_TIM13             ((uint32_t)0x00000080)
#define RCC_APB1Periph_TIM14             ((uint32_t)0x00000100)
#define RCC_APB1Periph_WWDG              ((uint32_t)0x00000800)
#define RCC_APB1Periph_SPI2              ((uint32_t)0x00004000)
#define RCC_APB1Periph_SPI3              ((uint32_t)0x00008000)
#define RCC_APB1Periph_USART2            ((uint32_t)0x00020000)
#define RCC_APB1Periph_USART3            ((uint32_t)0x00040000)
#define RCC_APB1Periph_UART4             ((uint32_t)0x00080000)
#define RCC_APB1Periph_UART5             ((uint32_t)0x00100000)
#define RCC_APB1Periph_I2C1              ((uint32_t)0x00200000)
#define RCC_APB1Periph_I2C2              ((uint32_t)0x00400000)
#define RCC_APB1Periph_USB               ((uint32_t)0x00800000)
#define RCC_APB1Periph_CAN1              ((uint32_t)0x02000000)
#define RCC_APB1Periph_CAN2              ((uint32_t)0x04000000)
#define RCC_APB1Periph_BKP               ((uint32_t)0x08000000)
#define RCC_APB1Periph_PWR               ((uint32_t)0x10000000)
#define RCC_APB1Periph_DAC               ((uint32_t)0x20000000)
#define RCC_APB1Periph_CEC               ((uint32_t)0x40000000)
 
#define IS_RCC_APB1_PERIPH(PERIPH) ((((PERIPH) & 0x81013600) == 0x00) && ((PERIPH) != 0x00))

/**
  * @}
  */

/** @defgroup Clock_source_to_output_on_MCO_pin 
  * @{
  */

#define RCC_MCO_NoClock                  ((uint8_t)0x00)
#define RCC_MCO_SYSCLK                   ((uint8_t)0x04)
#define RCC_MCO_HSI                      ((uint8_t)0x05)
#define RCC_MCO_HSE                      ((uint8_t)0x06)
#define RCC_MCO_PLLCLK_Div2              ((uint8_t)0x07)

#ifndef STM32F10X_CL

#else
 #define RCC_MCO_PLL2CLK                 ((uint8_t)0x08)
 #define RCC_MCO_PLL3CLK_Div2            ((uint8_t)0x09)
 #define RCC_MCO_XT1                     ((uint8_t)0x0A)
 #define RCC_MCO_PLL3CLK                 ((uint8_t)0x0B)


#endif /* STM32F10X_CL */ 

/**
  * @}
  */

/** @defgroup RCC_Flag 
  * @{
  */

#define RCC_FLAG_HSIRDY                  ((uint8_t)0x21)
#define RCC_FLAG_HSERDY                  ((uint8_t)0x31)
#define RCC_FLAG_PLLRDY                  ((uint8_t)0x39)
#define RCC_FLAG_LSERDY                  ((uint8_t)0x41)
#define RCC_FLAG_LSIRDY                  ((uint8_t)0x61)
#define RCC_FLAG_PINRST                  ((uint8_t)0x7A)
#define RCC_FLAG_PORRST                  ((uint8_t)0x7B)
#define RCC_FLAG_SFTRST                  ((uint8_t)0x7C)
#define RCC_FLAG_IWDGRST                 ((uint8_t)0x7D)
#define RCC_FLAG_WWDGRST                 ((uint8_t)0x7E)
#define RCC_FLAG_LPWRRST                 ((uint8_t)0x7F)

#ifndef STM32F10X_CL

#else
 #define RCC_FLAG_PLL2RDY                ((uint8_t)0x3B) 
 #define RCC_FLAG_PLL3RDY                ((uint8_t)0x3D) 

#endif /* STM32F10X_CL */ 



#ifdef __cplusplus
}
#endif

#endif /* __STM32F10x_RCC_H */
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */ 

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
