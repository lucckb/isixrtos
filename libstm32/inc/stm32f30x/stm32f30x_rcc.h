/*
 * =====================================================================================
 *
 *       Filename:  stm32f30x_rcc.h
 *
 *    Description:  Original file stm32 micro
 *
 *        Version:  1.0
 *        Created:  09.12.2017 18:56:44
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once



/** @defgroup RCC_Exported_Constants
  * @{
  */

/** @defgroup RCC_HSE_configuration 
  * @{
  */

#define RCC_HSE_OFF                      ((uint8_t)0x00)
#define RCC_HSE_ON                       ((uint8_t)0x01)
#define RCC_HSE_Bypass                   ((uint8_t)0x05)
#define IS_RCC_HSE(HSE) (((HSE) == RCC_HSE_OFF) || ((HSE) == RCC_HSE_ON) || \
                         ((HSE) == RCC_HSE_Bypass))

/**
  * @}
  */ 
 
/** @defgroup RCC_PLL_Clock_Source 
  * @{
  */
#define RCC_PLLSource_HSI                RCC_CFGR_PLLSRC_HSI_PREDIV     /*!< Only applicable for STM32F303xE devices */
#define RCC_PLLSource_HSI_Div2           RCC_CFGR_PLLSRC_HSI_Div2
#define RCC_PLLSource_PREDIV1            RCC_CFGR_PLLSRC_PREDIV1
 
/**
  * @}
  */ 

/** @defgroup RCC_PLL_Multiplication_Factor 
  * @{
  */

#define RCC_PLLMul_2                    RCC_CFGR_PLLMULL2
#define RCC_PLLMul_3                    RCC_CFGR_PLLMULL3
#define RCC_PLLMul_4                    RCC_CFGR_PLLMULL4
#define RCC_PLLMul_5                    RCC_CFGR_PLLMULL5
#define RCC_PLLMul_6                    RCC_CFGR_PLLMULL6
#define RCC_PLLMul_7                    RCC_CFGR_PLLMULL7
#define RCC_PLLMul_8                    RCC_CFGR_PLLMULL8
#define RCC_PLLMul_9                    RCC_CFGR_PLLMULL9
#define RCC_PLLMul_10                   RCC_CFGR_PLLMULL10
#define RCC_PLLMul_11                   RCC_CFGR_PLLMULL11
#define RCC_PLLMul_12                   RCC_CFGR_PLLMULL12
#define RCC_PLLMul_13                   RCC_CFGR_PLLMULL13
#define RCC_PLLMul_14                   RCC_CFGR_PLLMULL14
#define RCC_PLLMul_15                   RCC_CFGR_PLLMULL15
#define RCC_PLLMul_16                   RCC_CFGR_PLLMULL16

/** @defgroup RCC_PREDIV1_division_factor
  * @{
  */
#define  RCC_PREDIV1_Div1               RCC_CFGR2_PREDIV1_DIV1
#define  RCC_PREDIV1_Div2               RCC_CFGR2_PREDIV1_DIV2
#define  RCC_PREDIV1_Div3               RCC_CFGR2_PREDIV1_DIV3
#define  RCC_PREDIV1_Div4               RCC_CFGR2_PREDIV1_DIV4
#define  RCC_PREDIV1_Div5               RCC_CFGR2_PREDIV1_DIV5
#define  RCC_PREDIV1_Div6               RCC_CFGR2_PREDIV1_DIV6
#define  RCC_PREDIV1_Div7               RCC_CFGR2_PREDIV1_DIV7
#define  RCC_PREDIV1_Div8               RCC_CFGR2_PREDIV1_DIV8
#define  RCC_PREDIV1_Div9               RCC_CFGR2_PREDIV1_DIV9
#define  RCC_PREDIV1_Div10              RCC_CFGR2_PREDIV1_DIV10
#define  RCC_PREDIV1_Div11              RCC_CFGR2_PREDIV1_DIV11
#define  RCC_PREDIV1_Div12              RCC_CFGR2_PREDIV1_DIV12
#define  RCC_PREDIV1_Div13              RCC_CFGR2_PREDIV1_DIV13
#define  RCC_PREDIV1_Div14              RCC_CFGR2_PREDIV1_DIV14
#define  RCC_PREDIV1_Div15              RCC_CFGR2_PREDIV1_DIV15
#define  RCC_PREDIV1_Div16              RCC_CFGR2_PREDIV1_DIV16

 
/** @defgroup RCC_System_Clock_Source 
  * @{
  */

#define RCC_SYSCLKSource_HSI             RCC_CFGR_SW_HSI
#define RCC_SYSCLKSource_HSE             RCC_CFGR_SW_HSE
#define RCC_SYSCLKSource_PLLCLK          RCC_CFGR_SW_PLL
#define IS_RCC_SYSCLK_SOURCE(SOURCE) (((SOURCE) == RCC_SYSCLKSource_HSI) || \
                                      ((SOURCE) == RCC_SYSCLKSource_HSE) || \
                                      ((SOURCE) == RCC_SYSCLKSource_PLLCLK))
/**
  * @}
  */

/** @defgroup RCC_AHB_Clock_Source
  * @{
  */

#define RCC_SYSCLK_Div1                  RCC_CFGR_HPRE_DIV1
#define RCC_SYSCLK_Div2                  RCC_CFGR_HPRE_DIV2
#define RCC_SYSCLK_Div4                  RCC_CFGR_HPRE_DIV4
#define RCC_SYSCLK_Div8                  RCC_CFGR_HPRE_DIV8
#define RCC_SYSCLK_Div16                 RCC_CFGR_HPRE_DIV16
#define RCC_SYSCLK_Div64                 RCC_CFGR_HPRE_DIV64
#define RCC_SYSCLK_Div128                RCC_CFGR_HPRE_DIV128
#define RCC_SYSCLK_Div256                RCC_CFGR_HPRE_DIV256
#define RCC_SYSCLK_Div512                RCC_CFGR_HPRE_DIV512

/** @defgroup RCC_APB1_APB2_clock_source 
  * @{
  */

#define RCC_HCLK_Div1                    ((uint32_t)0x00000000)
#define RCC_HCLK_Div2                    ((uint32_t)0x00000400)
#define RCC_HCLK_Div4                    ((uint32_t)0x00000500)
#define RCC_HCLK_Div8                    ((uint32_t)0x00000600)
#define RCC_HCLK_Div16                   ((uint32_t)0x00000700)

/** @defgroup RCC_ADC_clock_source 
  * @{
  */
  
/* ADC1 & ADC2 */
#define RCC_ADC12PLLCLK_OFF                    ((uint32_t)0x00000000)
#define RCC_ADC12PLLCLK_Div1                   ((uint32_t)0x00000100)
#define RCC_ADC12PLLCLK_Div2                   ((uint32_t)0x00000110)
#define RCC_ADC12PLLCLK_Div4                   ((uint32_t)0x00000120)
#define RCC_ADC12PLLCLK_Div6                   ((uint32_t)0x00000130)
#define RCC_ADC12PLLCLK_Div8                   ((uint32_t)0x00000140)
#define RCC_ADC12PLLCLK_Div10                  ((uint32_t)0x00000150)
#define RCC_ADC12PLLCLK_Div12                  ((uint32_t)0x00000160)
#define RCC_ADC12PLLCLK_Div16                  ((uint32_t)0x00000170)
#define RCC_ADC12PLLCLK_Div32                  ((uint32_t)0x00000180)
#define RCC_ADC12PLLCLK_Div64                  ((uint32_t)0x00000190)
#define RCC_ADC12PLLCLK_Div128                 ((uint32_t)0x000001A0)
#define RCC_ADC12PLLCLK_Div256                 ((uint32_t)0x000001B0)

/* ADC3 & ADC4 */
#define RCC_ADC34PLLCLK_OFF                    ((uint32_t)0x10000000)
#define RCC_ADC34PLLCLK_Div1                   ((uint32_t)0x10002000)
#define RCC_ADC34PLLCLK_Div2                   ((uint32_t)0x10002200)
#define RCC_ADC34PLLCLK_Div4                   ((uint32_t)0x10002400)
#define RCC_ADC34PLLCLK_Div6                   ((uint32_t)0x10002600)
#define RCC_ADC34PLLCLK_Div8                   ((uint32_t)0x10002800)
#define RCC_ADC34PLLCLK_Div10                  ((uint32_t)0x10002A00)
#define RCC_ADC34PLLCLK_Div12                  ((uint32_t)0x10002C00)
#define RCC_ADC34PLLCLK_Div16                  ((uint32_t)0x10002E00)
#define RCC_ADC34PLLCLK_Div32                  ((uint32_t)0x10003000)
#define RCC_ADC34PLLCLK_Div64                  ((uint32_t)0x10003200)
#define RCC_ADC34PLLCLK_Div128                 ((uint32_t)0x10003400)
#define RCC_ADC34PLLCLK_Div256                 ((uint32_t)0x10003600)


/**
  * @}
  */

/** @defgroup RCC_TIM_clock_source 
  * @{
  */

#define RCC_TIM1CLK_PCLK                  ((uint32_t)0x00000000)
#define RCC_TIM1CLK_PLLCLK                RCC_CFGR3_TIM1SW

#define RCC_TIM8CLK_PCLK                  ((uint32_t)0x10000000)
#define RCC_TIM8CLK_PLLCLK                ((uint32_t)0x10000200)

#define RCC_TIM15CLK_PCLK                 ((uint32_t)0x20000000)
#define RCC_TIM15CLK_PLLCLK               ((uint32_t)0x20000400)

#define RCC_TIM16CLK_PCLK                 ((uint32_t)0x30000000)
#define RCC_TIM16CLK_PLLCLK               ((uint32_t)0x30000800)

#define RCC_TIM17CLK_PCLK                 ((uint32_t)0x40000000)
#define RCC_TIM17CLK_PLLCLK               ((uint32_t)0x40002000)

#define RCC_TIM20CLK_PCLK                 ((uint32_t)0x50000000)
#define RCC_TIM20CLK_PLLCLK               ((uint32_t)0x50008000)

#define RCC_TIM2CLK_PCLK                  ((uint32_t)0x60000000)
#define RCC_TIM2CLK_PLLCLK                ((uint32_t)0x61000000)

#define RCC_TIM3TIM4CLK_PCLK              ((uint32_t)0x70000000)
#define RCC_TIM3TIM4CLK_PLLCLK            ((uint32_t)0x72000000)

/* legacy RCC_TIM_clock_source*/							  
#define RCC_TIM1CLK_HCLK   RCC_TIM1CLK_PCLK							  
#define RCC_TIM8CLK_HCLK   RCC_TIM8CLK_PCLK
#define RCC_TIM15CLK_HCLK  RCC_TIM15CLK_PCLK							   
#define RCC_TIM16CLK_HCLK  RCC_TIM16CLK_PCLK
#define RCC_TIM17CLK_HCLK  RCC_TIM17CLK_PCLK
#define RCC_TIM20CLK_HCLK  RCC_TIM20CLK_PCLK
#define RCC_TIM2CLK_HCLK   RCC_TIM2CLK_PCLK
#define RCC_TIM3CLK_HCLK   RCC_TIM3TIM4CLK_PCLK
#define RCC_TIM3CLK_PLLCLK RCC_TIM3TIM4CLK_PLLCLK
/**
  * @}
  */

/** @defgroup RCC_HRTIM_clock_source 
  * @{
  */

#define RCC_HRTIM1CLK_HCLK                  ((uint32_t)0x00000000)
#define RCC_HRTIM1CLK_PLLCLK                RCC_CFGR3_HRTIM1SW


/**
  * @}
  */
  
/** @defgroup RCC_I2C_clock_source 
  * @{
  */

#define RCC_I2C1CLK_HSI                   ((uint32_t)0x00000000)
#define RCC_I2C1CLK_SYSCLK                RCC_CFGR3_I2C1SW

#define RCC_I2C2CLK_HSI                   ((uint32_t)0x10000000)
#define RCC_I2C2CLK_SYSCLK                ((uint32_t)0x10000020)

#define RCC_I2C3CLK_HSI                   ((uint32_t)0x20000000)
#define RCC_I2C3CLK_SYSCLK                ((uint32_t)0x20000040)

/**
  * @}
  */

/** @defgroup RCC_USART_clock_source 
  * @{
  */

#define RCC_USART1CLK_PCLK                  ((uint32_t)0x10000000)
#define RCC_USART1CLK_SYSCLK                ((uint32_t)0x10000001)
#define RCC_USART1CLK_LSE                   ((uint32_t)0x10000002)
#define RCC_USART1CLK_HSI                   ((uint32_t)0x10000003)

#define RCC_USART2CLK_PCLK                  ((uint32_t)0x20000000)
#define RCC_USART2CLK_SYSCLK                ((uint32_t)0x20010000)
#define RCC_USART2CLK_LSE                   ((uint32_t)0x20020000)
#define RCC_USART2CLK_HSI                   ((uint32_t)0x20030000)

#define RCC_USART3CLK_PCLK                  ((uint32_t)0x30000000)
#define RCC_USART3CLK_SYSCLK                ((uint32_t)0x30040000)
#define RCC_USART3CLK_LSE                   ((uint32_t)0x30080000)
#define RCC_USART3CLK_HSI                   ((uint32_t)0x300C0000)

#define RCC_UART4CLK_PCLK                   ((uint32_t)0x40000000)
#define RCC_UART4CLK_SYSCLK                 ((uint32_t)0x40100000)
#define RCC_UART4CLK_LSE                    ((uint32_t)0x40200000)
#define RCC_UART4CLK_HSI                    ((uint32_t)0x40300000)

#define RCC_UART5CLK_PCLK                   ((uint32_t)0x50000000)
#define RCC_UART5CLK_SYSCLK                 ((uint32_t)0x50400000)
#define RCC_UART5CLK_LSE                    ((uint32_t)0x50800000)
#define RCC_UART5CLK_HSI                    ((uint32_t)0x50C00000)


/**
  * @}
  */
       
/** @defgroup RCC_Interrupt_Source 
  * @{
  */

#define RCC_IT_LSIRDY                    ((uint8_t)0x01)
#define RCC_IT_LSERDY                    ((uint8_t)0x02)
#define RCC_IT_HSIRDY                    ((uint8_t)0x04)
#define RCC_IT_HSERDY                    ((uint8_t)0x08)
#define RCC_IT_PLLRDY                    ((uint8_t)0x10)
#define RCC_IT_CSS                       ((uint8_t)0x80)

 
/** @defgroup RCC_LSE_configuration 
  * @{
  */

#define RCC_LSE_OFF                      ((uint32_t)0x00000000)
#define RCC_LSE_ON                       RCC_BDCR_LSEON
#define RCC_LSE_Bypass                   ((uint32_t)(RCC_BDCR_LSEON | RCC_BDCR_LSEBYP))

/** @defgroup RCC_RTC_Clock_Source
  * @{
  */

#define RCC_RTCCLKSource_LSE             RCC_BDCR_RTCSEL_LSE
#define RCC_RTCCLKSource_LSI             RCC_BDCR_RTCSEL_LSI
#define RCC_RTCCLKSource_HSE_Div32       RCC_BDCR_RTCSEL_HSE


/** @defgroup RCC_I2S_Clock_Source
  * @{
  */
#define RCC_I2S2CLKSource_SYSCLK             ((uint8_t)0x00)
#define RCC_I2S2CLKSource_Ext                ((uint8_t)0x01)


/** @defgroup RCC_LSE_Drive_Configuration 
  * @{
  */

#define RCC_LSEDrive_Low                 ((uint32_t)0x00000000)
#define RCC_LSEDrive_MediumLow           RCC_BDCR_LSEDRV_0
#define RCC_LSEDrive_MediumHigh          RCC_BDCR_LSEDRV_1
#define RCC_LSEDrive_High                RCC_BDCR_LSEDRV

/**
  * @}
  */
  
/** @defgroup RCC_AHB_Peripherals 
  * @{
  */

#define RCC_AHBPeriph_ADC34               RCC_AHBENR_ADC34EN
#define RCC_AHBPeriph_ADC12               RCC_AHBENR_ADC12EN
#define RCC_AHBPeriph_GPIOA               RCC_AHBENR_GPIOAEN
#define RCC_AHBPeriph_GPIOB               RCC_AHBENR_GPIOBEN
#define RCC_AHBPeriph_GPIOC               RCC_AHBENR_GPIOCEN
#define RCC_AHBPeriph_GPIOD               RCC_AHBENR_GPIODEN
#define RCC_AHBPeriph_GPIOE               RCC_AHBENR_GPIOEEN
#define RCC_AHBPeriph_GPIOF               RCC_AHBENR_GPIOFEN
#define RCC_AHBPeriph_GPIOG               RCC_AHBENR_GPIOGEN
#define RCC_AHBPeriph_GPIOH               RCC_AHBENR_GPIOHEN
#define RCC_AHBPeriph_TS                  RCC_AHBENR_TSEN
#define RCC_AHBPeriph_CRC                 RCC_AHBENR_CRCEN
#define RCC_AHBPeriph_FMC                 RCC_AHBENR_FMCEN
#define RCC_AHBPeriph_FLITF               RCC_AHBENR_FLITFEN
#define RCC_AHBPeriph_SRAM                RCC_AHBENR_SRAMEN
#define RCC_AHBPeriph_DMA2                RCC_AHBENR_DMA2EN
#define RCC_AHBPeriph_DMA1                RCC_AHBENR_DMA1EN


/** @defgroup RCC_APB2_Peripherals 
  * @{
  */

#define RCC_APB2Periph_SYSCFG            RCC_APB2ENR_SYSCFGEN
#define RCC_APB2Periph_TIM1              RCC_APB2ENR_TIM1EN
#define RCC_APB2Periph_SPI1              RCC_APB2ENR_SPI1EN
#define RCC_APB2Periph_TIM8              RCC_APB2ENR_TIM8EN
#define RCC_APB2Periph_USART1            RCC_APB2ENR_USART1EN
#define RCC_APB2Periph_SPI4              RCC_APB2ENR_SPI4EN
#define RCC_APB2Periph_TIM15             RCC_APB2ENR_TIM15EN
#define RCC_APB2Periph_TIM16             RCC_APB2ENR_TIM16EN
#define RCC_APB2Periph_TIM17             RCC_APB2ENR_TIM17EN
#define RCC_APB2Periph_TIM20             RCC_APB2ENR_TIM20EN
#define RCC_APB2Periph_HRTIM1            RCC_APB2ENR_HRTIM1


/** @defgroup RCC_APB1_Peripherals 
  * @{
  */
#define RCC_APB1Periph_TIM2              RCC_APB1ENR_TIM2EN
#define RCC_APB1Periph_TIM3              RCC_APB1ENR_TIM3EN
#define RCC_APB1Periph_TIM4              RCC_APB1ENR_TIM4EN
#define RCC_APB1Periph_TIM6              RCC_APB1ENR_TIM6EN
#define RCC_APB1Periph_TIM7              RCC_APB1ENR_TIM7EN
#define RCC_APB1Periph_WWDG              RCC_APB1ENR_WWDGEN
#define RCC_APB1Periph_SPI2              RCC_APB1ENR_SPI2EN
#define RCC_APB1Periph_SPI3              RCC_APB1ENR_SPI3EN
#define RCC_APB1Periph_USART2            RCC_APB1ENR_USART2EN
#define RCC_APB1Periph_USART3            RCC_APB1ENR_USART3EN
#define RCC_APB1Periph_UART4             RCC_APB1ENR_UART4EN
#define RCC_APB1Periph_UART5             RCC_APB1ENR_UART5EN
#define RCC_APB1Periph_I2C1              RCC_APB1ENR_I2C1EN
#define RCC_APB1Periph_I2C2              RCC_APB1ENR_I2C2EN
#define RCC_APB1Periph_USB               RCC_APB1ENR_USBEN
#define RCC_APB1Periph_CAN1              RCC_APB1ENR_CAN1EN
#define RCC_APB1Periph_PWR               RCC_APB1ENR_PWREN
#define RCC_APB1Periph_DAC1              RCC_APB1ENR_DAC1EN
#define RCC_APB1Periph_I2C3              RCC_APB1ENR_I2C3EN
#define RCC_APB1Periph_DAC2              RCC_APB1ENR_DAC2EN
#define RCC_APB1Periph_DAC               RCC_APB1Periph_DAC1



/** @defgroup RCC_MCO_Clock_Source
  * @{
  */

#define RCC_MCOSource_NoClock            ((uint8_t)0x00)
#define RCC_MCOSource_LSI                ((uint8_t)0x02)
#define RCC_MCOSource_LSE                ((uint8_t)0x03)
#define RCC_MCOSource_SYSCLK             ((uint8_t)0x04)
#define RCC_MCOSource_HSI                ((uint8_t)0x05)
#define RCC_MCOSource_HSE                ((uint8_t)0x06)
#define RCC_MCOSource_PLLCLK_Div2        ((uint8_t)0x07)
#define RCC_MCOSource_PLLCLK             ((uint8_t)0x87)


/** @defgroup RCC_MCOPrescaler
  * @{
  */

#define RCC_MCOPrescaler_1            RCC_CFGR_MCO_PRE_1
#define RCC_MCOPrescaler_2            RCC_CFGR_MCO_PRE_2
#define RCC_MCOPrescaler_4            RCC_CFGR_MCO_PRE_4
#define RCC_MCOPrescaler_8            RCC_CFGR_MCO_PRE_8
#define RCC_MCOPrescaler_16           RCC_CFGR_MCO_PRE_16
#define RCC_MCOPrescaler_32           RCC_CFGR_MCO_PRE_32
#define RCC_MCOPrescaler_64           RCC_CFGR_MCO_PRE_64
#define RCC_MCOPrescaler_128          RCC_CFGR_MCO_PRE_128


/** @defgroup RCC_USB_Device_clock_source 
  * @{
  */

#define RCC_USBCLKSource_PLLCLK_1Div5   ((uint8_t)0x00)
#define RCC_USBCLKSource_PLLCLK_Div1    ((uint8_t)0x01)


/** @defgroup RCC_Flag 
  * @{
  */
#define RCC_FLAG_HSIRDY                  ((uint8_t)0x01)
#define RCC_FLAG_HSERDY                  ((uint8_t)0x11)
#define RCC_FLAG_PLLRDY                  ((uint8_t)0x19)
#define RCC_FLAG_MCOF                    ((uint8_t)0x9C)
#define RCC_FLAG_LSERDY                  ((uint8_t)0x21)
#define RCC_FLAG_LSIRDY                  ((uint8_t)0x41)
#define RCC_FLAG_OBLRST                  ((uint8_t)0x59)
#define RCC_FLAG_PINRST                  ((uint8_t)0x5A)
#define RCC_FLAG_PORRST                  ((uint8_t)0x5B)
#define RCC_FLAG_SFTRST                  ((uint8_t)0x5C)
#define RCC_FLAG_IWDGRST                 ((uint8_t)0x5D)
#define RCC_FLAG_WWDGRST                 ((uint8_t)0x5E)
#define RCC_FLAG_LPWRRST                 ((uint8_t)0x5F)

