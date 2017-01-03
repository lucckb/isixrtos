/*
 * =====================================================================================
 *
 *       Filename:  stm32rcc.h
 *
 *    Description:  STM32 
 *
 *        Version:  1.0
 *        Created:  25.03.2016 20:47:00
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *
 * =====================================================================================
 */

#pragma once

#include <stm32lib.h>



#define RCC_OFFSET                (RCC_BASE - PERIPH_BASE)

/* --- CR Register ---*/

/* Alias word address of HSION bit */
#define CR_OFFSET                 (RCC_OFFSET + 0x00)
#define HSION_BitNumber           0x00
#define CR_HSION_BB               (PERIPH_BB_BASE + (CR_OFFSET * 32) + (HSION_BitNumber * 4))

/* Alias word address of PLLON bit */
#define PLLON_BitNumber           0x18
#define CR_PLLON_BB               (PERIPH_BB_BASE + (CR_OFFSET * 32) + (PLLON_BitNumber * 4))

/* Alias word address of CSSON bit */
#define CSSON_BitNumber           0x13
#define CR_CSSON_BB               (PERIPH_BB_BASE + (CR_OFFSET * 32) + (CSSON_BitNumber * 4))

/* --- CFGR Register ---*/
/* Alias word address of USBPRE bit */
#define CFGR_OFFSET               (RCC_OFFSET + 0x04)
#define USBPRE_BitNumber          0x16
#define CFGR_USBPRE_BB            (PERIPH_BB_BASE + (CFGR_OFFSET * 32) + (USBPRE_BitNumber * 4))
/* Alias word address of I2SSRC bit */
#define I2SSRC_BitNumber          0x17
#define CFGR_I2SSRC_BB            (PERIPH_BB_BASE + (CFGR_OFFSET * 32) + (I2SSRC_BitNumber * 4))

/* --- BDCR Register ---*/

/* Alias word address of RTCEN bit */
#define BDCR_OFFSET               (RCC_OFFSET + 0x20)
#define RTCEN_BitNumber           0x0F
#define BDCR_RTCEN_BB             (PERIPH_BB_BASE + (BDCR_OFFSET * 32) + (RTCEN_BitNumber * 4))

/* Alias word address of BDRST bit */
#define BDRST_BitNumber           0x10
#define BDCR_BDRST_BB             (PERIPH_BB_BASE + (BDCR_OFFSET * 32) + (BDRST_BitNumber * 4))

/* --- CSR Register ---*/

/* Alias word address of LSION bit */
#define CSR_OFFSET                (RCC_OFFSET + 0x24)
#define LSION_BitNumber           0x00
#define CSR_LSION_BB              (PERIPH_BB_BASE + (CSR_OFFSET * 32) + (LSION_BitNumber * 4))

/* RCC Flag Mask */
#define FLAG_MASK                 ((uint8_t)0x1F)

/* CFGR register byte 3 (Bits[31:23]) base address */
#define CFGR_BYTE3_ADDRESS        ((uint32_t)0x40021007)

/* CIR register byte 2 (Bits[15:8]) base address */
#define CIR_BYTE2_ADDRESS         ((uint32_t)0x40021009)

/* CIR register byte 3 (Bits[23:16]) base address */
#define CIR_BYTE3_ADDRESS         ((uint32_t)0x4002100A)

/* CR register byte 2 (Bits[23:16]) base address */
#define CR_BYTE2_ADDRESS          ((uint32_t)0x40021002)

#ifdef __cplusplus
 namespace stm32 {
#endif

/* Resets the RCC clock configuration to the default reset state */
static inline void rcc_deinit(void)
{
  /* Set HSION bit */
  RCC->CR |= (uint32_t)0x00000001;

  /* Reset SW[1:0], HPRE[3:0], PPRE[2:0] and MCOSEL[2:0] bits */
  RCC->CFGR &= (uint32_t)0xF8FFC000;
  
  /* Reset HSEON, CSSON and PLLON bits */
  RCC->CR &= (uint32_t)0xFEF6FFFF;

  /* Reset HSEBYP bit */
  RCC->CR &= (uint32_t)0xFFFBFFFF;

  /* Reset PLLSRC, PLLXTPRE, PLLMUL and USBPRE bits */
  RCC->CFGR &= (uint32_t)0xFF80FFFF;
#if defined(STM32MCU_MAJOR_TYPE_F3)
  /* Reset PREDIV1[3:0] and ADCPRE[13:4] bits */
  RCC->CFGR2 &= (uint32_t)0xFFFFC000;
  /* Reset USARTSW[1:0], I2CSW and TIMSW bits */
  RCC->CFGR3 &= (uint32_t)0xF00ECCC;
#elif defined(STM32MCU_MAJOR_TYPE_F37)
  /* Reset PREDIV1[3:0] bits */
  RCC->CFGR2 &= (uint32_t)0xFFFFFFF0;
  /* Reset USARTSW[1:0], I2CSW and CECSW bits */
  RCC->CFGR3 &= (uint32_t)0xFFF0F8C;
#endif
  /* Disable all interrupts */
  RCC->CIR = 0x00000000;
}

/**
  * @brief  Configures the External High Speed oscillator (HSE).
  * @note   After enabling the HSE (RCC_HSE_ON or RCC_HSE_Bypass), the application
  *         software should wait on HSERDY flag to be set indicating that HSE clock
  *         is stable and can be used to clock the PLL and/or system clock.
  * @note   HSE state can not be changed if it is used directly or through the
  *         PLL as system clock. In this case, you have to select another source
  *         of the system clock then change the HSE state (ex. disable it).
  * @note   The HSE is stopped by hardware when entering STOP and STANDBY modes.         
  * @note   This function resets the CSSON bit, so if the Clock security system(CSS)
  *         was previously enabled you have to enable it again after calling this
  *         function.
  * @param  RCC_HSE: specifies the new state of the HSE.
  *   This parameter can be one of the following values:
  *     @arg RCC_HSE_OFF: turn OFF the HSE oscillator, HSERDY flag goes low after
  *                       6 HSE oscillator clock cycles.
  *     @arg RCC_HSE_ON: turn ON the HSE oscillator
  *     @arg RCC_HSE_Bypass: HSE oscillator bypassed with external clock
  * @retval None
  */
static inline void rcc_hse_config(uint8_t RCC_HSE)
{

  /* Reset HSEON and HSEBYP bits before configuring the HSE ------------------*/
  *(__IO uint8_t *) CR_BYTE2_ADDRESS = RCC_HSE_OFF;

  /* Set the new HSE configuration -------------------------------------------*/
  *(__IO uint8_t *) CR_BYTE2_ADDRESS = RCC_HSE;

}
/**
  * @brief  Checks whether the specified RCC flag is set or not.
  * @param  RCC_FLAG: specifies the flag to check.
  *   This parameter can be one of the following values:
  *     @arg RCC_FLAG_HSIRDY: HSI oscillator clock ready  
  *     @arg RCC_FLAG_HSERDY: HSE oscillator clock ready
  *     @arg RCC_FLAG_PLLRDY: PLL clock ready
  *     @arg RCC_FLAG_MCOF: MCO Flag  
  *     @arg RCC_FLAG_LSERDY: LSE oscillator clock ready
  *     @arg RCC_FLAG_LSIRDY: LSI oscillator clock ready
  *     @arg RCC_FLAG_OBLRST: Option Byte Loader (OBL) reset 
  *     @arg RCC_FLAG_PINRST: Pin reset
  *     @arg RCC_FLAG_PORRST: POR/PDR reset
  *     @arg RCC_FLAG_SFTRST: Software reset
  *     @arg RCC_FLAG_IWDGRST: Independent Watchdog reset
  *     @arg RCC_FLAG_WWDGRST: Window Watchdog reset
  *     @arg RCC_FLAG_LPWRRST: Low Power reset
  * @retval The new state of RCC_FLAG (SET or RESET).
  */
static inline bool rcc_get_flag_status(uint8_t RCC_FLAG)
{
  uint32_t tmp = 0;
  uint32_t statusreg = 0;
  /* Get the RCC register index */
  tmp = RCC_FLAG >> 5;
  if (tmp == 0)               /* The flag to check is in CR register */
  {
    statusreg = RCC->CR;
  }
  else if (tmp == 1)          /* The flag to check is in BDCR register */
  {
    statusreg = RCC->BDCR;
  }
#if defined(STM32MCU_MAJOR_TYPE_F3)
  else if (tmp == 4)          /* The flag to check is in CFGR register */
  {
    statusreg = RCC->CFGR;
  }
#endif
  else                       /* The flag to check is in CSR register */
  {
    statusreg = RCC->CSR;
  }

  /* Get the flag position */
  tmp = RCC_FLAG & FLAG_MASK;

  return ((statusreg & ((uint32_t)1 << tmp)) != 0 );
}


static inline bool rcc_wait_for_hse_startup(void)
{
  __IO uint32_t StartUpCounter = 0;
  bool hsestatus = false;
  
  /* Wait till HSE is ready and if timeout is reached exit */
  do
  {
    hsestatus = rcc_get_flag_status(RCC_FLAG_HSERDY);
    StartUpCounter++;  
  } while((StartUpCounter != HSE_STARTUP_TIMEOUT) && (hsestatus == RESET));
  
  return rcc_get_flag_status(RCC_FLAG_HSERDY);
}

/**
  * @brief  Adjusts the Internal High Speed oscillator (HSI) calibration value.
  * @note   The calibration is used to compensate for the variations in voltage
  *         and temperature that influence the frequency of the internal HSI RC.
  * @param  HSICalibrationValue: specifies the calibration trimming value.
  *         This parameter must be a number between 0 and 0x1F.
  * @retval None
  */
static inline void rcc_adjust_hsi_calibration_value(uint8_t HSICalibrationValue)
{
  uint32_t tmpreg = RCC->CR;

  /* Clear HSITRIM[4:0] bits */
  tmpreg &= ~RCC_CR_HSITRIM;

  /* Set the HSITRIM[4:0] bits according to HSICalibrationValue value */
  tmpreg |= (uint32_t)HSICalibrationValue << 3;

  /* Store the new value */
  RCC->CR = tmpreg;
}

/**
  * @brief  Enables or disables the Internal High Speed oscillator (HSI).
  * @note   The HSI is stopped by hardware when entering STOP and STANDBY modes.
  *         It is used (enabled by hardware) as system clock source after startup
  *         from Reset, wakeup from STOP and STANDBY mode, or in case of failure
  *         of the HSE used directly or indirectly as system clock (if the Clock
  *         Security System CSS is enabled).
  * @note   HSI can not be stopped if it is used as system clock source. In this case,
  *         you have to select another source of the system clock then stop the HSI.
  * @note   After enabling the HSI, the application software should wait on HSIRDY
  *         flag to be set indicating that HSI clock is stable and can be used as
  *         system clock source.
  * @param  NewState: new state of the HSI.
  *          This parameter can be: ENABLE or DISABLE.
  * @note   When the HSI is stopped, HSIRDY flag goes low after 6 HSI oscillator
  *         clock cycles.
  * @retval None
  */
static inline void rcc_hsi_cmd( bool enable )
{
  *(__IO uint32_t *) CR_HSION_BB = (uint32_t)enable;
}



/**
  * @brief  Configures the External Low Speed oscillator (LSE).
  * @note   As the LSE is in the Backup domain and write access is denied to this
  *         domain after reset, you have to enable write access using 
  *         PWR_BackupAccessCmd(ENABLE) function before to configure the LSE
  *         (to be done once after reset).
  * @note   Care must be taken when using this function to configure LSE mode 
  *         as it clears systematically the LSEON bit before any new configuration.
  * @note   After enabling the LSE (RCC_LSE_ON or RCC_LSE_Bypass), the application
  *         software should wait on LSERDY flag to be set indicating that LSE clock
  *         is stable and can be used to clock the RTC.
  * @param  RCC_LSE: specifies the new state of the LSE.
  *   This parameter can be one of the following values:
  *     @arg RCC_LSE_OFF: turn OFF the LSE oscillator, LSERDY flag goes low after
  *                       6 LSE oscillator clock cycles.
  *     @arg RCC_LSE_ON: turn ON the LSE oscillator
  *     @arg RCC_LSE_Bypass: LSE oscillator bypassed with external clock
  * @retval None
  */
static inline void rcc_lse_config(uint32_t RCC_LSE)
{
  /* Reset LSEON and LSEBYP bits before configuring the LSE ------------------*/
  /* Reset LSEON bit */
  RCC->BDCR &= ~(RCC_BDCR_LSEON);

  /* Reset LSEBYP bit */
  RCC->BDCR &= ~(RCC_BDCR_LSEBYP);

  /* Configure LSE */
  RCC->BDCR |= RCC_LSE;
}


/**
  * @brief  Configures the External Low Speed oscillator (LSE) drive capability.
  * @param  RCC_LSEDrive: specifies the new state of the LSE drive capability.
  *   This parameter can be one of the following values:
  *     @arg RCC_LSEDrive_Low: LSE oscillator low drive capability.
  *     @arg RCC_LSEDrive_MediumLow: LSE oscillator medium low drive capability.
  *     @arg RCC_LSEDrive_MediumHigh: LSE oscillator medium high drive capability.
  *     @arg RCC_LSEDrive_High: LSE oscillator high drive capability.
  * @retval None
  */
static inline void rcc_lse_drive_config(uint32_t RCC_LSEDrive)
{
  
  /* Clear LSEDRV[1:0] bits */
  RCC->BDCR &= ~(RCC_BDCR_LSEDRV);

  /* Set the LSE Drive */
  RCC->BDCR |= RCC_LSEDrive;
}

/**
  * @brief  Enables or disables the Internal Low Speed oscillator (LSI).
  * @note   After enabling the LSI, the application software should wait on
  *         LSIRDY flag to be set indicating that LSI clock is stable and can
  *         be used to clock the IWDG and/or the RTC.
  * @note   LSI can not be disabled if the IWDG is running.
  * @param  NewState: new state of the LSI.
  *          This parameter can be: ENABLE or DISABLE.
  * @note   When the LSI is stopped, LSIRDY flag goes low after 6 LSI oscillator
  *         clock cycles.
  * @retval None
  */
static inline void rcc_lsi_cmd( bool enable )
{

  *(__IO uint32_t *) CSR_LSION_BB = (uint32_t)enable;
}

/**
  * @brief  Configures the PLL clock source and multiplication factor.
  * @note   This function must be used only when the PLL is disabled.
  * @note   The minimum input clock frequency for PLL is 2 MHz (when using HSE as
  *         PLL source).   
  * @param  RCC_PLLSource: specifies the PLL entry clock source.
  *   This parameter can be one of the following values:
  *     @arg RCC_PLLSource_HSI: HSI oscillator clockselected as PLL clock entry  
  *     @arg RCC_PLLSource_HSI_Div2: HSI oscillator clock divided by 2 selected as
  *         PLL clock entry
  *     @arg RCC_PLLSource_PREDIV1: PREDIV1 clock selected as PLL clock source              
  * @param  RCC_PLLMul: specifies the PLL multiplication factor, which drive the PLLVCO clock
  *   This parameter can be RCC_PLLMul_x where x:[2,16] 
  *                                               
  * @retval None
  */
static inline void rcc_pll_config(uint32_t RCC_PLLSource, uint32_t RCC_PLLMul)
{
  
  /* Clear PLL Source [16] and Multiplier [21:18] bits */
  RCC->CFGR &= ~(RCC_CFGR_PLLMULL | RCC_CFGR_PLLSRC);

  /* Set the PLL Source and Multiplier */
  RCC->CFGR |= (uint32_t)(RCC_PLLSource | RCC_PLLMul);
}

/**
  * @brief  Enables or disables the PLL.
  * @note   After enabling the PLL, the application software should wait on 
  *         PLLRDY flag to be set indicating that PLL clock is stable and can
  *         be used as system clock source.
  * @note   The PLL can not be disabled if it is used as system clock source
  * @note   The PLL is disabled by hardware when entering STOP and STANDBY modes.    
  * @param  NewState: new state of the PLL.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void rcc_pll_cmd( bool enable )
{
  *(__IO uint32_t *) CR_PLLON_BB = (uint32_t)enable;
}

/**
  * @brief  Configures the PREDIV1 division factor.
  * @note   This function must be used only when the PLL is disabled.
  * @param  RCC_PREDIV1_Div: specifies the PREDIV1 clock division factor.
  *         This parameter can be RCC_PREDIV1_Divx where x:[1,16]
  * @retval None
  */
static inline void rcc_prediv1_config(uint32_t RCC_PREDIV1_Div)
{
  uint32_t tmpreg = 0;

  tmpreg = RCC->CFGR2;
  /* Clear PREDIV1[3:0] bits */
  tmpreg &= ~(RCC_CFGR2_PREDIV1);

  /* Set the PREDIV1 division factor */
  tmpreg |= RCC_PREDIV1_Div;

  /* Store the new value */
  RCC->CFGR2 = tmpreg;
}

/**
  * @brief  Enables or disables the Clock Security System.
  * @note   If a failure is detected on the HSE oscillator clock, this oscillator
  *         is automatically disabled and an interrupt is generated to inform the
  *         software about the failure (Clock Security System Interrupt, CSSI),
  *         allowing the MCU to perform rescue operations. The CSSI is linked to 
  *         the Cortex-M4 NMI (Non-Maskable Interrupt) exception vector.  
  * @param  NewState: new state of the Clock Security System.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void rcc_clock_security_system_cmd(bool enable)
{
  
  *(__IO uint32_t *) CR_CSSON_BB = (uint32_t)enable;
}

#ifdef STM32F303xC
static inline void rcc_mco_config(uint8_t RCC_MCOSource, uint32_t RCC_MCOPrescaler)
{
  uint32_t tmpreg = 0;
    
  /* Get CFGR value */  
  tmpreg = RCC->CFGR;
  /* Clear MCOPRE[2:0] bits */
  tmpreg &= ~(RCC_CFGR_MCO_PRE | RCC_CFGR_MCO | RCC_CFGR_PLLNODIV);
  /* Set the RCC_MCOSource and RCC_MCOPrescaler */
  tmpreg |= (RCC_MCOPrescaler | RCC_MCOSource<<24);
  /* Store the new value */
  RCC->CFGR = tmpreg;
}
#else
/**
  * @brief  Selects the clock source to output on MCO pin (PA8).
  * @note   PA8 should be configured in alternate function mode.
  * @param  RCC_MCOSource: specifies the clock source to output.
  *          This parameter can be one of the following values:
  *            @arg RCC_MCOSource_NoClock: No clock selected.
  *            @arg RCC_MCOSource_LSI: LSI oscillator clock selected.
  *            @arg RCC_MCOSource_LSE: LSE oscillator clock selected.
  *            @arg RCC_MCOSource_SYSCLK: System clock selected.
  *            @arg RCC_MCOSource_HSI: HSI oscillator clock selected.
  *            @arg RCC_MCOSource_HSE: HSE oscillator clock selected.
  *            @arg RCC_MCOSource_PLLCLK_Div2: PLL clock divided by 2 selected.
  * @retval None
  */
static inline void rcc_mco_config(uint8_t RCC_MCOSource)
{
  /* Select MCO clock source and prescaler */
  *(__IO uint8_t *) CFGR_BYTE3_ADDRESS =  RCC_MCOSource; 
}
#endif

/**
  * @brief  Configures the system clock (SYSCLK).
  * @note     The HSI is used (enabled by hardware) as system clock source after
  *           startup from Reset, wake-up from STOP and STANDBY mode, or in case
  *           of failure of the HSE used directly or indirectly as system clock
  *           (if the Clock Security System CSS is enabled).
  * @note     A switch from one clock source to another occurs only if the target
  *           clock source is ready (clock stable after startup delay or PLL locked). 
  *           If a clock source which is not yet ready is selected, the switch will
  *           occur when the clock source will be ready. 
  *           You can use RCC_GetSYSCLKSource() function to know which clock is
  *           currently used as system clock source.  
  * @param  RCC_SYSCLKSource: specifies the clock source used as system clock source 
  *   This parameter can be one of the following values:
  *     @arg RCC_SYSCLKSource_HSI:    HSI selected as system clock source
  *     @arg RCC_SYSCLKSource_HSE:    HSE selected as system clock source
  *     @arg RCC_SYSCLKSource_PLLCLK: PLL selected as system clock source
  * @retval None
  */
static inline void rcc_sysclk_config(uint32_t rcc_sysclksource)
{
  uint32_t tmpreg = RCC->CFGR;

  /* clear sw[1:0] bits */
  tmpreg &= ~RCC_CFGR_SW;

  /* set sw[1:0] bits according to rcc_sysclksource value */
  tmpreg |= rcc_sysclksource;

  /* store the new value */
  RCC->CFGR = tmpreg;
}

/**
  * @brief  Returns the clock source used as system clock.
  * @param  None
  * @retval The clock source used as system clock. The returned value can be one
  *         of the following:
  *              - 0x00: HSI used as system clock
  *              - 0x04: HSE used as system clock
  *              - 0x08: PLL used as system clock
  */
static inline uint8_t rcc_get_sysclk_source(void)
{
  return ((uint8_t)(RCC->CFGR & RCC_CFGR_SWS));
}


/**
  * @brief  Configures the AHB clock (HCLK).
  * @note   Depending on the device voltage range, the software has to set correctly
  *         these bits to ensure that HCLK not exceed the maximum allowed frequency
  *         (for more details refer to section above
  *           "CPU, AHB and APB busses clocks configuration functions")
  * @param  RCC_SYSCLK: defines the AHB clock divider. This clock is derived from
  *         the system clock (SYSCLK).
  *          This parameter can be one of the following values:
  *            @arg RCC_SYSCLK_Div1: AHB clock = SYSCLK
  *            @arg RCC_SYSCLK_Div2: AHB clock = SYSCLK/2
  *            @arg RCC_SYSCLK_Div4: AHB clock = SYSCLK/4
  *            @arg RCC_SYSCLK_Div8: AHB clock = SYSCLK/8
  *            @arg RCC_SYSCLK_Div16: AHB clock = SYSCLK/16
  *            @arg RCC_SYSCLK_Div64: AHB clock = SYSCLK/64
  *            @arg RCC_SYSCLK_Div128: AHB clock = SYSCLK/128
  *            @arg RCC_SYSCLK_Div256: AHB clock = SYSCLK/256
  *            @arg RCC_SYSCLK_Div512: AHB clock = SYSCLK/512
  * @retval None
  */
static inline void rcc_hclk_config(uint32_t RCC_SYSCLK)
{
  uint32_t tmpreg = RCC->CFGR;

  /* Clear HPRE[3:0] bits */
  tmpreg &= ~RCC_CFGR_HPRE;

  /* Set HPRE[3:0] bits according to RCC_SYSCLK value */
  tmpreg |= RCC_SYSCLK;

  /* Store the new value */
  RCC->CFGR = tmpreg;
}
	 
/**
  * @brief  Configures the Low Speed APB clock (PCLK1).
  * @param  RCC_HCLK: defines the APB1 clock divider. This clock is derived from
  *         the AHB clock (HCLK).
  *          This parameter can be one of the following values:
  *            @arg RCC_HCLK_Div1:  APB1 clock = HCLK
  *            @arg RCC_HCLK_Div2:  APB1 clock = HCLK/2
  *            @arg RCC_HCLK_Div4:  APB1 clock = HCLK/4
  *            @arg RCC_HCLK_Div8:  APB1 clock = HCLK/8
  *            @arg RCC_HCLK_Div16: APB1 clock = HCLK/16
  * @retval None
  */
static inline void rcc_pclk1_config(uint32_t RCC_HCLK)
{
  uint32_t tmpreg = RCC->CFGR;

  /* Clear PPRE1[2:0] bits */
  tmpreg &= ~RCC_CFGR_PPRE1;

  /* Set PPRE1[2:0] bits according to RCC_HCLK value */
  tmpreg |= RCC_HCLK;

  /* Store the new value */
  RCC->CFGR = tmpreg;
}

/**
  * @brief  Configures the High Speed APB clock (PCLK2).
  * @param  RCC_HCLK: defines the APB2 clock divider. This clock is derived from
  *         the AHB clock (HCLK).
  *          This parameter can be one of the following values:
  *            @arg RCC_HCLK_Div1:  APB2 clock = HCLK
  *            @arg RCC_HCLK_Div2:  APB2 clock = HCLK/2
  *            @arg RCC_HCLK_Div4:  APB2 clock = HCLK/4
  *            @arg RCC_HCLK_Div8:  APB2 clock = HCLK/8
  *            @arg RCC_HCLK_Div16: APB2 clock = HCLK/16
  * @retval None
  */
static inline void rcc_pclk2_config(uint32_t RCC_HCLK)
{

  uint32_t tmpreg = RCC->CFGR;

  /* Clear PPRE2[2:0] bits */
  tmpreg &= ~RCC_CFGR_PPRE2;

  /* Set PPRE2[2:0] bits according to RCC_HCLK value */
  tmpreg |= RCC_HCLK << 3;

  /* Store the new value */
  RCC->CFGR = tmpreg;
}

#if defined(STM32MCU_MAJOR_TYPE_F3)
/**
  * @brief  Configures the ADC clock (ADCCLK).
  * @param  RCC_PLLCLK: defines the ADC clock divider. This clock is derived from 
	ex RCC_ADC12PLLCLK_Div12
  */
static inline void rcc_adc_clk_config(uint32_t RCC_PLLCLK)
{
  uint32_t tmp = 0;
  

  tmp = (RCC_PLLCLK >> 28);
  
  /* Clears ADCPRE34 bits */
  if (tmp != 0)
  {
    RCC->CFGR2 &= ~RCC_CFGR2_ADCPRE34;
  }
   /* Clears ADCPRE12 bits */
  else
  {
    RCC->CFGR2 &= ~RCC_CFGR2_ADCPRE12;
  }
  /* Set ADCPRE bits according to RCC_PLLCLK value */
  RCC->CFGR2 |= RCC_PLLCLK;
}
#elif defined(STM32MCU_MAJOR_TYPE_F37)
/**
  * @brief  Configures the ADC clock (ADCCLK).
  * @param  RCC_PCLK2: defines the ADC clock divider. This clock is derived from
  *         the APB2 clock (PCLK2).
  *          This parameter can be one of the following values:
  *             @arg RCC_PCLK2_Div2: ADC clock = PCLK2/2
  *             @arg RCC_PCLK2_Div4: ADC clock = PCLK2/4
  *             @arg RCC_PCLK2_Div6: ADC clock = PCLK2/6
  *             @arg RCC_PCLK2_Div8: ADC clock = PCLK2/8
  * @retval None
  */
static inline void rcc_adc_clk_config(uint32_t RCC_PCLK2)
{

  /* Clear ADCPRE[1:0] bits */
  RCC->CFGR &= ~RCC_CFGR_ADCPRE;

  /* Set ADCPRE[1:0] bits according to RCC_PCLK2 value */
  RCC->CFGR |= RCC_PCLK2;
}
#endif


#if defined(STM32MCU_MAJOR_TYPE_F37)
/**
  * @brief  Configures the SDADC clock (SDADCCLK).
  * @param  RCC_PCLK2: defines the ADC clock divider. This clock is derived from
  *         the system clock (SYSCLK).
  *          This parameter can be one of the following values:
  *             @arg RCC_SDADCCLK_SYSCLK_Div2: SDADC clock = SYSCLK/2
  *             @arg RCC_SDADCCLK_SYSCLK_Div4: SDADC clock = SYSCLK/4
  *             @arg RCC_SDADCCLK_SYSCLK_Div6: SDADC clock = SYSCLK/6
  *             @arg RCC_SDADCCLK_SYSCLK_Div8: SDADC clock = SYSCLK/8
  *             @arg RCC_SDADCCLK_SYSCLK_Div10: SDADC clock = SYSCLK/10
  *             @arg RCC_SDADCCLK_SYSCLK_Div12: SDADC clock = SYSCLK/12
  *             @arg RCC_SDADCCLK_SYSCLK_Div14: SDADC clock = SYSCLK/14
  *             @arg RCC_SDADCCLK_SYSCLK_Div16: SDADC clock = SYSCLK/16
  *             @arg RCC_SDADCCLK_SYSCLK_Div20: SDADC clock = SYSCLK/20
  *             @arg RCC_SDADCCLK_SYSCLK_Div24: SDADC clock = SYSCLK/24
  *             @arg RCC_SDADCCLK_SYSCLK_Div28: SDADC clock = SYSCLK/28
  *             @arg RCC_SDADCCLK_SYSCLK_Div32: SDADC clock = SYSCLK/32
  *             @arg RCC_SDADCCLK_SYSCLK_Div36: SDADC clock = SYSCLK/36
  *             @arg RCC_SDADCCLK_SYSCLK_Div40: SDADC clock = SYSCLK/40
  *             @arg RCC_SDADCCLK_SYSCLK_Div44: SDADC clock = SYSCLK/44
  *             @arg RCC_SDADCCLK_SYSCLK_Div48: SDADC clock = SYSCLK/48
  * @retval None
  */
static inline void rcc_sdadc_clk_config(uint32_t RCC_SDADCCLK)
{
  /* Clear ADCPRE[1:0] bits */
  RCC->CFGR &= ~RCC_CFGR_SDADCPRE;

  /* Set SDADCPRE[4:0] bits according to RCC_PCLK2 value */
  RCC->CFGR |= RCC_SDADCCLK;
}

/**
  * @brief  Configures the CEC clock (CECCLK).
  * @param  RCC_CECCLK: defines the CEC clock source. This clock is derived
  *         from the HSI or LSE clock.
  *          This parameter can be one of the following values:
  *             @arg RCC_CECCLK_HSI_Div244: CEC clock = HSI/244 (32768Hz)
  *             @arg RCC_CECCLK_LSE: CEC clock = LSE
  * @retval None
  */
static inline void rcc_cec_clk_config(uint32_t RCC_CECCLK)
{

 /* Clear CECSW bit */
  RCC->CFGR3 &= ~RCC_CFGR3_CECSW;
  /* Set CECSW bits according to RCC_CECCLK value */
  RCC->CFGR3 |= RCC_CECCLK;
}

#endif




/**
  * @brief  Configures the I2C clock (I2CCLK).
  * @param  RCC_I2CCLK: defines the I2C clock source. This clock is derived 
  *         from the HSI or System clock.
  *   This parameter can be one of the following values:
  *     @arg RCC_I2CxCLK_HSI: I2Cx clock = HSI
  *     @arg RCC_I2CxCLK_SYSCLK: I2Cx clock = System Clock
  *          (x can be 1 or 2 or 3).  
  * @retval None
  */
static inline void rcc_i2c_clk_config(uint32_t RCC_I2CCLK)
{ 
  uint32_t tmp = (RCC_I2CCLK >> 28);
  /* Clear I2CSW bit */
    switch (tmp)
  {
    case 0x00: 
      RCC->CFGR3 &= ~RCC_CFGR3_I2C1SW;
      break;
    case 0x01:
      RCC->CFGR3 &= ~RCC_CFGR3_I2C2SW;
      break;
#ifdef RCC_CFGR3_I2C3SW
    case 0x02:
      RCC->CFGR3 &= ~RCC_CFGR3_I2C3SW;
      break;
#endif
    default:
      break;
  }
  /* Set I2CSW bits according to RCC_I2CCLK value */
  RCC->CFGR3 |= RCC_I2CCLK;
}


#ifdef RCC_CFGR3_TIM1SW
/**
  * @brief  Configures the TIMx clock sources(TIMCLK).
  * @note   For STM32F303xC devices, TIMx can be clocked from the PLL running at 144 MHz 
  *         when the system clock source is the PLL and HCLK & PCLK2 clocks are not divided in respect to SYSCLK.  
  *         For the devices STM32F334x8, STM32F302x8 and STM32F303xE, TIMx can be clocked from the PLL running at 
  *         144 MHz when the system clock source is the PLL and  AHB or APB2 subsystem clocks are not divided by 
  *         more than 2 cumulatively.
  * @note   If one of the previous conditions is missed, the TIM clock source 
  *         configuration is lost and calling again this function becomes mandatory.  
  * @param  RCC_TIMCLK: defines the TIMx clock source.
  *   This parameter can be one of the following values:
  *     @arg RCC_TIMxCLK_PCLK: TIMx clock = APB clock (doubled frequency when prescaled)
  *     @arg RCC_TIMxCLK_PLLCLK: TIMx clock = PLL output (running up to 144 MHz)
  *          (x can be 1, 8, 15, 16, 17, 20, 2, 3,4).
  * @note   For STM32F303xC devices, TIM1 and TIM8 can be clocked at 144MHz. 
  *         For STM32F303xE devices, TIM1/8/20/2/3/4/15/16/17 can be clocked at 144MHz. 
  *         For STM32F334x8 devices , only TIM1 can be clocked at 144MHz.
  *         For STM32F302x8 devices, TIM1/15/16/17 can be clocked at 144MHz
  * @retval None
  */
static inline void rcc_timclk_config(uint32_t RCC_TIMCLK)
{ 
  uint32_t tmp = (RCC_TIMCLK >> 28);
  switch (tmp)
  {
    case 0x00: 
      RCC->CFGR3 &= ~RCC_CFGR3_TIM1SW;
      break;
    case 0x01:
      RCC->CFGR3 &= ~RCC_CFGR3_TIM8SW;
      break;
    case 0x02:
      RCC->CFGR3 &= ~RCC_CFGR3_TIM15SW;
      break;
    case 0x03:
      RCC->CFGR3 &= ~RCC_CFGR3_TIM16SW;
      break;
    case 0x04:
      RCC->CFGR3 &= ~RCC_CFGR3_TIM17SW;
      break;
    case 0x05:
      RCC->CFGR3 &= ~RCC_CFGR3_TIM20SW;
    case 0x06:
      RCC->CFGR3 &= ~RCC_CFGR3_TIM2SW;
    case 0x07:
      RCC->CFGR3 &= ~RCC_CFGR3_TIM3SW;
      break;
    default:
      break;
  }
  /* Set I2CSW bits according to RCC_TIMCLK value */
  RCC->CFGR3 |= RCC_TIMCLK;
}
#endif

/**
  * @brief  Configures the HRTIM1 clock sources(HRTIM1CLK).
  * @note     The configuration of the HRTIM1 clock source is only possible when the 
  *           SYSCLK = PLL and HCLK and PCLK2 clocks are not divided in respect to SYSCLK
  * @note     If one of the previous conditions is missed, the TIM clock source 
  *           configuration is lost and calling again this function becomes mandatory.  
  * @param  RCC_HRTIMCLK: defines the TIMx clock source.
  *   This parameter can be one of the following values:
  *     @arg RCC_HRTIM1CLK_HCLK: TIMx clock = APB high speed clock (doubled frequency
  *          when prescaled)
  *     @arg RCC_HRTIM1CLK_PLLCLK: TIMx clock = PLL output (running up to 144 MHz)
  *          (x can be 1 or 8).
  * @retval None
  */
#ifdef RCC_CFGR3_HRTIM1SW
static inline void rcc_hrtim1_clk_config(uint32_t RCC_HRTIMCLK)
{ 
  /* Clear HRTIMSW bit */
  RCC->CFGR3 &= ~RCC_CFGR3_HRTIM1SW;
  /* Set HRTIMSW bits according to RCC_HRTIMCLK value */
  RCC->CFGR3 |= RCC_HRTIMCLK;
}
#endif



/**
  * @brief  Configures the I2S clock source (I2SCLK).
  * @note   This function must be called before enabling the SPI2 and SPI3 clocks.
  * @param  RCC_I2SCLKSource: specifies the I2S clock source.
  *          This parameter can be one of the following values:
  *            @arg RCC_I2S2CLKSource_SYSCLK: SYSCLK clock used as I2S clock source
  *            @arg RCC_I2S2CLKSource_Ext: External clock mapped on the I2S_CKIN pin
  *                                        used as I2S clock source
  * @retval None
  */
static inline void rcc_i2s_clk_config(uint32_t RCC_I2SCLKSource)
{

  *(__IO uint32_t *) CFGR_I2SSRC_BB = RCC_I2SCLKSource;
}



/**
  * @brief  Configures the USB clock (USBCLK).
  * @param  RCC_USBCLKSource: specifies the USB clock source. This clock is
  *   derived from the PLL output.
  *   This parameter can be one of the following values:
  *     @arg RCC_USBCLKSource_PLLCLK_1Div5: PLL clock divided by 1,5 selected as USB
  *                                     clock source
  *     @arg RCC_USBCLKSource_PLLCLK_Div1: PLL clock selected as USB clock source
  * @retval None
  */
static inline void rcc_usb_clk_config(uint32_t RCC_USBCLKSource)
{

  *(__IO uint32_t *) CFGR_USBPRE_BB = RCC_USBCLKSource;
}



/**
  * @brief  Configures the USART clock (USARTCLK).
  * @param  RCC_USARTCLK: defines the USART clock source. This clock is derived 
  *         from the HSI or System clock.
  *   This parameter can be one of the following values:
  *     @arg RCC_USARTxCLK_PCLK: USART clock = APB Clock (PCLK)
  *     @arg RCC_USARTxCLK_SYSCLK: USART clock = System Clock
  *     @arg RCC_USARTxCLK_LSE: USART clock = LSE Clock
  *     @arg RCC_USARTxCLK_HSI: USART clock = HSI Clock
  *          (x can be 1, 2, 3, 4 or 5).  
  * @retval None
  */
static inline void rcc_usart_clk_config(uint32_t RCC_USARTCLK)
{ 
  uint32_t tmp = (RCC_USARTCLK >> 28);
  /* Clear USARTSW[1:0] bit */
  switch (tmp)
  {
    case 0x01:  /* clear USART1SW */
      RCC->CFGR3 &= ~RCC_CFGR3_USART1SW;
      break;
    case 0x02:  /* clear USART2SW */
      RCC->CFGR3 &= ~RCC_CFGR3_USART2SW;
      break;
    case 0x03:  /* clear USART3SW */
      RCC->CFGR3 &= ~RCC_CFGR3_USART3SW;
      break;
#ifdef RCC_CFGR3_UART4SW
    case 0x04:  /* clear UART4SW */
      RCC->CFGR3 &= ~RCC_CFGR3_UART4SW;
      break;
#endif
#ifdef RCC_CFGR3_UART5SW
    case 0x05:  /* clear UART5SW */
      RCC->CFGR3 &= ~RCC_CFGR3_UART5SW;
      break;
#endif
    default:
      break;
  }
  /* Set USARTSW bits according to RCC_USARTCLK value */
  RCC->CFGR3 |= RCC_USARTCLK;
}



/**
  * @brief  Configures the RTC clock (RTCCLK).
  * @note     As the RTC clock configuration bits are in the Backup domain and write
  *           access is denied to this domain after reset, you have to enable write
  *           access using PWR_BackupAccessCmd(ENABLE) function before to configure
  *           the RTC clock source (to be done once after reset).    
  * @note     Once the RTC clock is configured it can't be changed unless the RTC
  *           is reset using RCC_BackupResetCmd function, or by a Power On Reset (POR)
  *             
  * @param  RCC_RTCCLKSource: specifies the RTC clock source.
  *   This parameter can be one of the following values:
  *     @arg RCC_RTCCLKSource_LSE: LSE selected as RTC clock
  *     @arg RCC_RTCCLKSource_LSI: LSI selected as RTC clock
  *     @arg RCC_RTCCLKSource_HSE_Div32: HSE divided by 32 selected as RTC clock
  *       
  * @note     If the LSE or LSI is used as RTC clock source, the RTC continues to
  *           work in STOP and STANDBY modes, and can be used as wakeup source.
  *           However, when the HSE clock is used as RTC clock source, the RTC
  *           cannot be used in STOP and STANDBY modes.             
  * @note     The maximum input clock frequency for RTC is 2MHz (when using HSE as
  *           RTC clock source).             
  * @retval None
  */
static inline void rcc_rtc_clk_config(uint32_t RCC_RTCCLKSource)
{
  /* Select the RTC clock source */
  RCC->BDCR |= RCC_RTCCLKSource;
}

/**
  * @brief  Enables or disables the RTC clock.
  * @note   This function must be used only after the RTC clock source was selected
  *         using the RCC_RTCCLKConfig function.
  * @param  NewState: new state of the RTC clock.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void rcc_rtc_clk_cmd( bool enable )
{

  *(__IO uint32_t *) BDCR_RTCEN_BB = (uint32_t)enable;
}



/**
  * @brief  Forces or releases the Backup domain reset.
  * @note   This function resets the RTC peripheral (including the backup registers)
  *         and the RTC clock source selection in RCC_CSR register.
  * @note   The BKPSRAM is not affected by this reset.
  * @param  NewState: new state of the Backup domain reset.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void rcc_backup_reset_cmd( bool enable )
{
  *(__IO uint32_t *) BDCR_BDRST_BB = (uint32_t)enable;
}




/** @brief  Enables or disables the AHB peripheral clock */
static inline void rcc_ahb_periph_clock_cmd(uint32_t RCC_AHBPeriph, bool enable)
{
  if (enable) {
    RCC->AHBENR |= RCC_AHBPeriph;
  } else {
    RCC->AHBENR &= ~RCC_AHBPeriph;
  }
}


/** @brief  Enables or disables the High Speed APB (APB2) peripheral clock.*/
static inline void rcc_apb2_periph_clock_cmd(uint32_t RCC_APB2Periph, bool enable)
{
  if ( enable ) {
    RCC->APB2ENR |= RCC_APB2Periph;
  } else {
    RCC->APB2ENR &= ~RCC_APB2Periph;
  }
}

/** @brief  Enables or disables the Low Speed APB (APB1) peripheral clock. */
static inline void rcc_apb1_periph_clock_cmd(uint32_t RCC_APB1Periph, bool enable )
{
  if ( enable ) {
    RCC->APB1ENR |= RCC_APB1Periph;
  } else {
    RCC->APB1ENR &= ~RCC_APB1Periph;
  }
}

static inline void rcc_ahb_periph_reset_cmd(uint32_t RCC_AHBPeriph, bool enable )
{
  if ( enable ) {
    RCC->AHBRSTR |= RCC_AHBPeriph;
  } else {
    RCC->AHBRSTR &= ~RCC_AHBPeriph;
  }
}

static inline void rcc_apb1_periph_reset_cmd(uint32_t RCC_APB1Periph, bool enable ) {
  if ( enable ) {
    RCC->APB1RSTR |= RCC_APB1Periph;
  } else {
    RCC->APB1RSTR &= ~RCC_APB1Periph;
  }
}


static inline void rcc_apb2_periph_reset_cmd(uint32_t RCC_APB2Periph, bool enable)
{
  if (enable) {
    RCC->APB2RSTR |= RCC_APB2Periph;
  } else {
    RCC->APB2RSTR &= ~RCC_APB2Periph;
  }
}

/**
  * @brief  Enables or disables the specified RCC interrupts.
  * @note   The CSS interrupt doesn't have an enable bit; once the CSS is enabled
  *         and if the HSE clock fails, the CSS interrupt occurs and an NMI is
  *         automatically generated. The NMI will be executed indefinitely, and 
  *         since NMI has higher priority than any other IRQ (and main program)
  *         the application will be stacked in the NMI ISR unless the CSS interrupt
  *         pending bit is cleared.
  * @param  RCC_IT: specifies the RCC interrupt sources to be enabled or disabled.
  *   This parameter can be any combination of the following values:
  *     @arg RCC_IT_LSIRDY: LSI ready interrupt
  *     @arg RCC_IT_LSERDY: LSE ready interrupt
  *     @arg RCC_IT_HSIRDY: HSI ready interrupt
  *     @arg RCC_IT_HSERDY: HSE ready interrupt
  *     @arg RCC_IT_PLLRDY: PLL ready interrupt
  * @param  NewState: new state of the specified RCC interrupts.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void rcc_it_config(uint8_t RCC_IT, bool enable)
{
  if (enable) {
    /* Perform Byte access to RCC_CIR[13:8] bits to enable the selected interrupts */
    *(__IO uint8_t *) CIR_BYTE2_ADDRESS |= RCC_IT;
  } else {
    /* Perform Byte access to RCC_CIR[13:8] bits to disable the selected interrupts */
    *(__IO uint8_t *) CIR_BYTE2_ADDRESS &= (uint8_t)~RCC_IT;
  }
}



/**
  * @brief  Clears the RCC reset flags.
  *         The reset flags are: RCC_FLAG_OBLRST, RCC_FLAG_PINRST, RCC_FLAG_PORRST, 
  *         RCC_FLAG_SFTRST, RCC_FLAG_IWDGRST, RCC_FLAG_WWDGRST, RCC_FLAG_LPWRRST.
  * @param  None
  * @retval None
  */
static inline void rcc_clear_flag(void)
{
  /* Set RMVF bit to clear the reset flags */
  RCC->CSR |= RCC_CSR_RMVF;
}

/**
  * @brief  Checks whether the specified RCC interrupt has occurred or not.
  * @param  RCC_IT: specifies the RCC interrupt source to check.
  *   This parameter can be one of the following values:
  *     @arg RCC_IT_LSIRDY: LSI ready interrupt
  *     @arg RCC_IT_LSERDY: LSE ready interrupt
  *     @arg RCC_IT_HSIRDY: HSI ready interrupt
  *     @arg RCC_IT_HSERDY: HSE ready interrupt
  *     @arg RCC_IT_PLLRDY: PLL ready interrupt
  *     @arg RCC_IT_CSS: Clock Security System interrupt
  * @retval The new state of RCC_IT (SET or RESET).
  */
static inline bool rcc_get_it_status(uint8_t RCC_IT)
{
  /* Check the status of the specified RCC interrupt */
  return ((RCC->CIR & RCC_IT) != 0 );
}

/**
  * @brief  Clears the RCC's interrupt pending bits.
  * @param  RCC_IT: specifies the interrupt pending bit to clear.
  *   This parameter can be any combination of the following values:
  *     @arg RCC_IT_LSIRDY: LSI ready interrupt
  *     @arg RCC_IT_LSERDY: LSE ready interrupt
  *     @arg RCC_IT_HSIRDY: HSI ready interrupt
  *     @arg RCC_IT_HSERDY: HSE ready interrupt
  *     @arg RCC_IT_PLLRDY: PLL ready interrupt
  *     @arg RCC_IT_CSS: Clock Security System interrupt
  * @retval None
  */
static inline void rcc_clear_it_pending_bit(uint8_t RCC_IT)
{
  /* Perform Byte access to RCC_CIR[23:16] bits to clear the selected interrupt
     pending bits */
  *(__IO uint8_t *) CIR_BYTE3_ADDRESS = RCC_IT;
}


//! Structure for defining sysclk mode
enum e_sysclk_mode
{
	e_sysclk_hsi,		//! hi speed internal oscilator only
	e_sysclk_hse,		//! hi speed external oscilator only
	e_sysclk_hsi_pll,	//! hi speed internal PLL
	e_sysclk_hse_pll	//! hi speed external PLL
};




/** Setup the best flash latency according to the CPU speed
 * @param[in] frequency Sysclk frequency */
static inline void rcc_flash_latency(uint32_t frequency)
{
	uint32_t wait_states;
	if(frequency > 72000000ul) frequency = 72000000ul;
	wait_states = (frequency-1) / 24000000ul;	// calculate wait_states (30M is valid for 2.7V to 3.6V voltage range, use 24M for 2.4V to 2.7V, 18M for 2.1V to 2.4V or 16M for  1.8V to 2.1V)
	wait_states &= FLASH_ACR_LATENCY;			// trim to max allowed value - 7
	FLASH->ACR = wait_states;				    // set wait_states, disable all caches and prefetch
	FLASH->ACR = FLASH_ACR_PRFTBE  | wait_states;	// enable caches and prefetch
}


/** Setup SYSCLK mode by unified way without required manual intervention
 * @param[in] mode System mode
 * @param[in] crystal Crystal frequency value
 * @param[in] frequency Excepted system frequency
 * @retval best fit frequency
 *
 */
static inline uint32_t rcc_pll1_sysclk_setup(enum e_sysclk_mode mode, 
		uint32_t crystal, uint32_t frequency)
{
	static const unsigned PLL1_Bit_Shift = 18;
	static const unsigned PLL1_Bit_Mask = 0xf;
	static const unsigned PLL1_Mul_Offset = 2;
	static const unsigned PRE1Div_Mask = 0x0F;
	static const unsigned PRE1Div_Offset = 1;
	uint32_t best_frequency_core = 0;
	if( mode == e_sysclk_hse_pll || mode == e_sysclk_hse )
		RCC->CR  |= RCC_CR_HSEON;
	else
		crystal = 8000000ul;
	if( mode == e_sysclk_hsi_pll ) crystal /= 2;
	if( mode == e_sysclk_hse_pll || mode == e_sysclk_hsi_pll )
	{
		uint32_t div, mul, vco_input_frequency, frequency_core;
		uint32_t best_div = 0, best_mul = 0;
		for (div = 1; div <= 16; div++)			// PLL divider
		{
			vco_input_frequency = crystal / div;
			for (mul = 2; mul <= 16; mul++)	// Multiply
			{
				frequency_core = vco_input_frequency * mul;
				if (frequency_core > frequency)	// skip values over desired frequency
				continue;
				if (frequency_core > best_frequency_core)	// is this configuration better than previous one?
				{
					best_frequency_core = frequency_core;	// yes - save values
					best_div = div;
					best_mul = mul;
				}
			}
		}
	   // configure PLL
	   RCC->CFGR = ((best_mul - PLL1_Mul_Offset) & PLL1_Bit_Mask) << PLL1_Bit_Shift;
	   RCC->CFGR2 = (best_div - PRE1Div_Offset) & PRE1Div_Mask;
	   if( mode == e_sysclk_hse_pll ) RCC->CFGR |= RCC_CFGR_PLLSRC;
		// AHB - no prescaler, APB1 - divide by 16, APB2 - divide by 16 (adefine always safe value)
		RCC->CFGR |= RCC_CFGR_PPRE2_DIV16 | RCC_CFGR_PPRE1_DIV16 | RCC_CFGR_HPRE_DIV1;
	}
	if( mode == e_sysclk_hse_pll || mode == e_sysclk_hse ) while(1)
	{
	   if(RCC->CR & RCC_CR_HSERDY) break;
	}
	if( mode == e_sysclk_hse_pll || mode == e_sysclk_hsi_pll )
	{
		RCC->CR |= RCC_CR_PLLON;
		while(1)
		{
			if(RCC->CR & RCC_CR_PLLRDY) break;
		}
		RCC->CFGR |= RCC_CFGR_SW_PLL;			// change SYSCLK to PLL
		while (((RCC->CFGR) & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);	// wait for switch
	}
	else if( mode == e_sysclk_hsi )
	{
		RCC->CFGR |= RCC_CFGR_SW_HSI;			// change SYSCLK to PLL
		while ( ((RCC->CFGR) & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI );	// wait for switch
		RCC->CR &= ~RCC_CR_PLLON;
		best_frequency_core = crystal;
	}
	else if( mode == e_sysclk_hse )
	{
		RCC->CFGR |= RCC_CFGR_SW_HSE;			// change SYSCLK to PLL
		while ( ((RCC->CFGR) & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSE );	// wait for switch
		RCC->CR &= ~RCC_CR_PLLON;
		best_frequency_core = crystal;
	}
	return best_frequency_core;
}

#ifdef __cplusplus
 }
#endif
//Undef temporary defines
#undef RCC_OFFSET
#undef CR_OFFSET
#undef HSION_BitNumber
#undef CR_HSION_BB
#undef PLLON_BitNumber
#undef CR_PLLON_BB
#undef CSSON_BitNumber
#undef CR_CSSON_BB
#undef CFGR_OFFSET
#undef USBPRE_BitNumber
#undef CFGR_USBPRE_BB
#undef I2SSRC_BitNumber
#undef CFGR_I2SSRC_BB
#undef BDCR_OFFSET
#undef RTCEN_BitNumber
#undef BDCR_RTCEN_BB
#undef BDRST_BitNumber
#undef BDCR_BDRST_BB
#undef CSR_OFFSET
#undef LSION_BitNumber
#undef CSR_LSION_BB
#undef FLAG_MASK
#undef CFGR_BYTE3_ADDRESS
#undef CIR_BYTE2_ADDRESS
#undef CIR_BYTE3_ADDRESS
#undef CR_BYTE2_ADDRESS

