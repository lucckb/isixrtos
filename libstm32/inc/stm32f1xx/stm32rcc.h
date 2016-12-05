/*
 * stm32rcc.h
 *
 *  Created on: 17-02-2012
 *      Author: lucck
 */

#ifndef STM32F1_STM32RCC_H_
#define STM32F1_STM32RCC_H_


#include <stm32f1xx/stm32f10x_lib.h>
#include <stddef.h>
#include "stm32f10x_rcc.h"

#ifdef __cplusplus
 namespace stm32 {
#endif
 
#ifdef __cplusplus
namespace _internal {
namespace rcc {

static const uint32_t RCC_OFFSET         =       (RCC_BASE - PERIPH_BASE);
/* --- CR Register ---*/
/* Alias word address of HSION bit */
static const uint32_t CR_OFFSET         =        (RCC_OFFSET + 0x00);
static const uint32_t HSION_BitNumber    =       0x00;
static const uint32_t CR_HSION_BB        =       (PERIPH_BB_BASE + (CR_OFFSET * 32) + (HSION_BitNumber * 4));
/* Alias word address of PLLON bit */
static const uint32_t PLLON_BitNumber  =         0x18;
static const uint32_t CR_PLLON_BB       =       (PERIPH_BB_BASE + (CR_OFFSET * 32) + (PLLON_BitNumber * 4));
#ifdef STM32F10X_CL
 /* Alias word address of PLL2ON bit */
 static const uint32_t PLL2ON_BitNumber    =      0x1A;
 static const uint32_t CR_PLL2ON_BB         =     (PERIPH_BB_BASE + (CR_OFFSET * 32) + (PLL2ON_BitNumber * 4));
 /* Alias word address of PLL3ON bit */
 static const uint32_t PLL3ON_BitNumber    =      0x1C;
 static const uint32_t CR_PLL3ON_BB        =      (PERIPH_BB_BASE + (CR_OFFSET * 32) + (PLL3ON_BitNumber * 4));
#endif /* STM32F10X_CL */
/* Alias word address of CSSON bit */
static const uint32_t CSSON_BitNumber    =       0x13;
static const uint32_t CR_CSSON_BB        =       (PERIPH_BB_BASE + (CR_OFFSET * 32) + (CSSON_BitNumber * 4));
/* --- CFGR Register ---*/
/* Alias word address of USBPRE bit */
static const uint32_t CFGR_OFFSET         =      (RCC_OFFSET + 0x04);
#ifndef STM32F10X_CL
 static const uint32_t USBPRE_BitNumber    =      0x16;
 static const uint32_t CFGR_USBPRE_BB     =       (PERIPH_BB_BASE + (CFGR_OFFSET * 32) + (USBPRE_BitNumber * 4));
#else
 static const uint32_t OTGFSPRE_BitNumber   =     0x16;
 static const uint32_t CFGR_OTGFSPRE_BB     =     (PERIPH_BB_BASE + (CFGR_OFFSET * 32) + (OTGFSPRE_BitNumber * 4));
#endif /* STM32F10X_CL */
/* --- BDCR Register ---*/
/* Alias word address of RTCEN bit */
static const uint32_t BDCR_OFFSET         =      (RCC_OFFSET + 0x20);
static const uint32_t RTCEN_BitNumber     =      0x0F;
static const uint32_t BDCR_RTCEN_BB       =      (PERIPH_BB_BASE + (BDCR_OFFSET * 32) + (RTCEN_BitNumber * 4));
/* Alias word address of BDRST bit */
static const uint32_t BDRST_BitNumber     =      0x10;
static const uint32_t BDCR_BDRST_BB       =      (PERIPH_BB_BASE + (BDCR_OFFSET * 32) + (BDRST_BitNumber * 4));
/* --- CSR Register ---*/
/* Alias word address of LSION bit */
static const uint32_t CSR_OFFSET          =      (RCC_OFFSET + 0x24);
static const uint32_t LSION_BitNumber     =      0x00;
static const uint32_t CSR_LSION_BB        =      (PERIPH_BB_BASE + (CSR_OFFSET * 32) + (LSION_BitNumber * 4));
#ifdef STM32F10X_CL
/* --- CFGR2 Register ---*/
 /* Alias word address of I2S2SRC bit */
 static const uint32_t CFGR2_OFFSET         =     (RCC_OFFSET + 0x2C);
 static const uint32_t I2S2SRC_BitNumber     =    0x11;
 static const uint32_t CFGR2_I2S2SRC_BB     =     (PERIPH_BB_BASE + (CFGR2_OFFSET * 32) + (I2S2SRC_BitNumber * 4));
 /* Alias word address of I2S3SRC bit */
 static const uint32_t I2S3SRC_BitNumber    =     0x12;
 static const uint32_t CFGR2_I2S3SRC_BB     =     (PERIPH_BB_BASE + (CFGR2_OFFSET * 32) + (I2S3SRC_BitNumber * 4));
#endif /* STM32F10X_CL */

/* CR register bit mask */
static const uint32_t CR_HSEBYP_Reset    =      0xFFFBFFFF;
static const uint32_t CR_HSEBYP_Set      =      0x00040000;
static const uint32_t CR_HSEON_Reset     =      0xFFFEFFFF;
static const uint32_t CR_HSEON_Set       =      0x00010000;
static const uint32_t CR_HSITRIM_Mask    =      0xFFFFFF07;
/* CFGR register bit mask */
#if defined (STM32F10X_LD_VL) || defined (STM32F10X_MD_VL) || defined (STM32F10X_HD_VL) || defined (STM32F10X_CL)
 static const uint32_t CFGR_PLL_Mask      =      0xFFC2FFFF;
#else
 static const uint32_t CFGR_PLL_Mask      =      0xFFC0FFFF;
#endif /* STM32F10X_CL */
static const uint32_t CFGR_PLLMull_Mask    =     0x003C0000;
static const uint32_t CFGR_PLLSRC_Mask     =     0x00010000;
static const uint32_t CFGR_PLLXTPRE_Mask   =     0x00020000;
static const uint32_t CFGR_SWS_Mask        =     0x0000000C;
static const uint32_t CFGR_SW_Mask         =     0xFFFFFFFC;
static const uint32_t CFGR_HPRE_Reset_Mask =     0xFFFFFF0F;
static const uint32_t CFGR_HPRE_Set_Mask   =     0x000000F0;
static const uint32_t CFGR_PPRE1_Reset_Mask =    0xFFFFF8FF;
static const uint32_t CFGR_PPRE1_Set_Mask  =     0x00000700;
static const uint32_t CFGR_PPRE2_Reset_Mask =    0xFFFFC7FF;
static const uint32_t CFGR_PPRE2_Set_Mask   =    0x00003800;
static const uint32_t CFGR_ADCPRE_Reset_Mask =   0xFFFF3FFF;
static const uint32_t CFGR_ADCPRE_Set_Mask   =   0x0000C000;
/* CSR register bit mask */
static const uint32_t CSR_RMVF_Set      =        0x01000000;
#if defined (STM32F10X_LD_VL) || defined (STM32F10X_MD_VL) || defined (STM32F10X_HD_VL) || defined (STM32F10X_CL)
/* CFGR2 register bit mask */
 static const uint32_t CFGR2_PREDIV1SRC   =      0x00010000;
 static const uint32_t CFGR2_PREDIV1      =      0x0000000F;
#endif
#ifdef STM32F10X_CL
 static const uint32_t CFGR2_PREDIV2    =       0x000000F0;
 static const uint32_t CFGR2_PLL2MUL    =       0x00000F00;
 static const uint32_t CFGR2_PLL3MUL     =      0x0000F000;
#endif /* STM32F10X_CL */
/* RCC Flag Mask */
static const uint8_t FLAG_Mask         =        0x1F;
/* CIR register byte 2 (Bits[15:8]) base address */
static const uint32_t CIR_BYTE2_ADDRESS   =      0x40021009;
/* CIR register byte 3 (Bits[23:16]) base address */
static const uint32_t CIR_BYTE3_ADDRESS    =     0x4002100A;
/* CFGR register byte 4 (Bits[31:24]) base address */
static const uint32_t CFGR_BYTE4_ADDRESS  =      0x40021007;
/* BDCR register base address */
static const uint32_t BDCR_ADDRESS         =     (PERIPH_BASE + BDCR_OFFSET);

}}
#else	//C defines


#define RCC_OFFSET                (RCC_BASE - PERIPH_BASE)
/* --- CR Register ---*/
/* Alias word address of HSION bit */
#define CR_OFFSET                 (RCC_OFFSET + 0x00)
#define HSION_BitNumber           0x00
#define CR_HSION_BB               (PERIPH_BB_BASE + (CR_OFFSET * 32) + (HSION_BitNumber * 4))
/* Alias word address of PLLON bit */
#define PLLON_BitNumber           0x18
#define CR_PLLON_BB               (PERIPH_BB_BASE + (CR_OFFSET * 32) + (PLLON_BitNumber * 4))
#ifdef STM32F10X_CL
 /* Alias word address of PLL2ON bit */
 #define PLL2ON_BitNumber          0x1A
 #define CR_PLL2ON_BB              (PERIPH_BB_BASE + (CR_OFFSET * 32) + (PLL2ON_BitNumber * 4))
 /* Alias word address of PLL3ON bit */
 #define PLL3ON_BitNumber          0x1C
 #define CR_PLL3ON_BB              (PERIPH_BB_BASE + (CR_OFFSET * 32) + (PLL3ON_BitNumber * 4))
#endif /* STM32F10X_CL */
/* Alias word address of CSSON bit */
#define CSSON_BitNumber           0x13
#define CR_CSSON_BB               (PERIPH_BB_BASE + (CR_OFFSET * 32) + (CSSON_BitNumber * 4))
/* --- CFGR Register ---*/
/* Alias word address of USBPRE bit */
#define CFGR_OFFSET               (RCC_OFFSET + 0x04)
#ifndef STM32F10X_CL
 #define USBPRE_BitNumber          0x16
 #define CFGR_USBPRE_BB            (PERIPH_BB_BASE + (CFGR_OFFSET * 32) + (USBPRE_BitNumber * 4))
#else
 #define OTGFSPRE_BitNumber        0x16
 #define CFGR_OTGFSPRE_BB          (PERIPH_BB_BASE + (CFGR_OFFSET * 32) + (OTGFSPRE_BitNumber * 4))
#endif /* STM32F10X_CL */
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
#ifdef STM32F10X_CL
/* --- CFGR2 Register ---*/
 /* Alias word address of I2S2SRC bit */
 #define CFGR2_OFFSET              (RCC_OFFSET + 0x2C)
 #define I2S2SRC_BitNumber         0x11
 #define CFGR2_I2S2SRC_BB          (PERIPH_BB_BASE + (CFGR2_OFFSET * 32) + (I2S2SRC_BitNumber * 4))
 /* Alias word address of I2S3SRC bit */
 #define I2S3SRC_BitNumber         0x12
 #define CFGR2_I2S3SRC_BB          (PERIPH_BB_BASE + (CFGR2_OFFSET * 32) + (I2S3SRC_BitNumber * 4))
#endif /* STM32F10X_CL */

/* CR register bit mask */
#define CR_HSEBYP_Reset           ((uint32_t)0xFFFBFFFF)
#define CR_HSEBYP_Set             ((uint32_t)0x00040000)
#define CR_HSEON_Reset            ((uint32_t)0xFFFEFFFF)
#define CR_HSEON_Set              ((uint32_t)0x00010000)
#define CR_HSITRIM_Mask           ((uint32_t)0xFFFFFF07)
/* CFGR register bit mask */
#if defined (STM32F10X_LD_VL) || defined (STM32F10X_MD_VL) || defined (STM32F10X_HD_VL) || defined (STM32F10X_CL)
 #define CFGR_PLL_Mask            ((uint32_t)0xFFC2FFFF)
#else
 #define CFGR_PLL_Mask            ((uint32_t)0xFFC0FFFF)
#endif /* STM32F10X_CL */
#define CFGR_PLLMull_Mask         ((uint32_t)0x003C0000)
#define CFGR_PLLSRC_Mask          ((uint32_t)0x00010000)
#define CFGR_PLLXTPRE_Mask        ((uint32_t)0x00020000)
#define CFGR_SWS_Mask             ((uint32_t)0x0000000C)
#define CFGR_SW_Mask              ((uint32_t)0xFFFFFFFC)
#define CFGR_HPRE_Reset_Mask      ((uint32_t)0xFFFFFF0F)
#define CFGR_HPRE_Set_Mask        ((uint32_t)0x000000F0)
#define CFGR_PPRE1_Reset_Mask     ((uint32_t)0xFFFFF8FF)
#define CFGR_PPRE1_Set_Mask       ((uint32_t)0x00000700)
#define CFGR_PPRE2_Reset_Mask     ((uint32_t)0xFFFFC7FF)
#define CFGR_PPRE2_Set_Mask       ((uint32_t)0x00003800)
#define CFGR_ADCPRE_Reset_Mask    ((uint32_t)0xFFFF3FFF)
#define CFGR_ADCPRE_Set_Mask      ((uint32_t)0x0000C000)
/* CSR register bit mask */
#define CSR_RMVF_Set              ((uint32_t)0x01000000)
#if defined (STM32F10X_LD_VL) || defined (STM32F10X_MD_VL) || defined (STM32F10X_HD_VL) || defined (STM32F10X_CL)
/* CFGR2 register bit mask */
 #define CFGR2_PREDIV1SRC         ((uint32_t)0x00010000)
 #define CFGR2_PREDIV1            ((uint32_t)0x0000000F)
#endif
#ifdef STM32F10X_CL
 #define CFGR2_PREDIV2            ((uint32_t)0x000000F0)
 #define CFGR2_PLL2MUL            ((uint32_t)0x00000F00)
 #define CFGR2_PLL3MUL            ((uint32_t)0x0000F000)
#endif /* STM32F10X_CL */
/* RCC Flag Mask */
#define FLAG_Mask                 ((uint8_t)0x1F)
/* CIR register byte 2 (Bits[15:8]) base address */
#define CIR_BYTE2_ADDRESS         ((uint32_t)0x40021009)
/* CIR register byte 3 (Bits[23:16]) base address */
#define CIR_BYTE3_ADDRESS         ((uint32_t)0x4002100A)
/* CFGR register byte 4 (Bits[31:24]) base address */
#define CFGR_BYTE4_ADDRESS        ((uint32_t)0x40021007)
/* BDCR register base address */
#define BDCR_ADDRESS              (PERIPH_BASE + BDCR_OFFSET)
#endif

/**
  * @brief  Resets the RCC clock configuration to the default reset state.
  * @param  None
  * @retval None
  */
static inline void rcc_deinit(void)
{
  /* Set HSION bit */
  RCC->CR |= (uint32_t)0x00000001;

  /* Reset SW, HPRE, PPRE1, PPRE2, ADCPRE and MCO bits */
#ifndef STM32F10X_CL
  RCC->CFGR &= (uint32_t)0xF8FF0000;
#else
  RCC->CFGR &= (uint32_t)0xF0FF0000;
#endif /* STM32F10X_CL */

  /* Reset HSEON, CSSON and PLLON bits */
  RCC->CR &= (uint32_t)0xFEF6FFFF;

  /* Reset HSEBYP bit */
  RCC->CR &= (uint32_t)0xFFFBFFFF;

  /* Reset PLLSRC, PLLXTPRE, PLLMUL and USBPRE/OTGFSPRE bits */
  RCC->CFGR &= (uint32_t)0xFF80FFFF;

#ifdef STM32F10X_CL
  /* Reset PLL2ON and PLL3ON bits */
  RCC->CR &= (uint32_t)0xEBFFFFFF;

  /* Disable all interrupts and clear pending bits  */
  RCC->CIR = 0x00FF0000;

  /* Reset CFGR2 register */
  RCC->CFGR2 = 0x00000000;
#elif defined (STM32F10X_LD_VL) || defined (STM32F10X_MD_VL) || defined (STM32F10X_HD_VL)
  /* Disable all interrupts and clear pending bits  */
  RCC->CIR = 0x009F0000;

  /* Reset CFGR2 register */
  RCC->CFGR2 = 0x00000000;
#else
  /* Disable all interrupts and clear pending bits  */
  RCC->CIR = 0x009F0000;
#endif /* STM32F10X_CL */

}

/**
  * @brief  Configures the External High Speed oscillator (HSE).
  * @note   HSE can not be stopped if it is used directly or through the PLL as system clock.
  * @param  RCC_HSE: specifies the new state of the HSE.
  *   This parameter can be one of the following values:
  *     @arg RCC_HSE_OFF: HSE oscillator OFF
  *     @arg RCC_HSE_ON: HSE oscillator ON
  *     @arg RCC_HSE_Bypass: HSE oscillator bypassed with external clock
  * @retval None
  */
static inline void rcc_hse_config(uint32_t RCC_HSE)
{
#ifdef __cplusplus
	using namespace _internal::rcc;
#endif
  /* Reset HSEON and HSEBYP bits before configuring the HSE */
  /* Reset HSEON bit */
  RCC->CR &= CR_HSEON_Reset;
  /* Reset HSEBYP bit */
  RCC->CR &= CR_HSEBYP_Reset;
  /* Configure HSE (RCC_HSE_OFF is already covered by the code section above) */
  switch(RCC_HSE)
  {
    case RCC_HSE_ON:
      /* Set HSEON bit */
      RCC->CR |= CR_HSEON_Set;
      break;

    case RCC_HSE_Bypass:
      /* Set HSEBYP and HSEON bits */
      RCC->CR |= CR_HSEBYP_Set | CR_HSEON_Set;
      break;
    default:
      break;
  }
}

/**
  * @brief  Checks whether the specified RCC flag is set or not.
  * @param  RCC_FLAG: specifies the flag to check.
  *
  *   For @b STM32_Connectivity_line_devices, this parameter can be one of the
  *   following values:
  *     @arg RCC_FLAG_HSIRDY: HSI oscillator clock ready
  *     @arg RCC_FLAG_HSERDY: HSE oscillator clock ready
  *     @arg RCC_FLAG_PLLRDY: PLL clock ready
  *     @arg RCC_FLAG_PLL2RDY: PLL2 clock ready
  *     @arg RCC_FLAG_PLL3RDY: PLL3 clock ready
  *     @arg RCC_FLAG_LSERDY: LSE oscillator clock ready
  *     @arg RCC_FLAG_LSIRDY: LSI oscillator clock ready
  *     @arg RCC_FLAG_PINRST: Pin reset
  *     @arg RCC_FLAG_PORRST: POR/PDR reset
  *     @arg RCC_FLAG_SFTRST: Software reset
  *     @arg RCC_FLAG_IWDGRST: Independent Watchdog reset
  *     @arg RCC_FLAG_WWDGRST: Window Watchdog reset
  *     @arg RCC_FLAG_LPWRRST: Low Power reset
  *
  *   For @b other_STM32_devices, this parameter can be one of the following values:
  *     @arg RCC_FLAG_HSIRDY: HSI oscillator clock ready
  *     @arg RCC_FLAG_HSERDY: HSE oscillator clock ready
  *     @arg RCC_FLAG_PLLRDY: PLL clock ready
  *     @arg RCC_FLAG_LSERDY: LSE oscillator clock ready
  *     @arg RCC_FLAG_LSIRDY: LSI oscillator clock ready
  *     @arg RCC_FLAG_PINRST: Pin reset
  *     @arg RCC_FLAG_PORRST: POR/PDR reset
  *     @arg RCC_FLAG_SFTRST: Software reset
  *     @arg RCC_FLAG_IWDGRST: Independent Watchdog reset
  *     @arg RCC_FLAG_WWDGRST: Window Watchdog reset
  *     @arg RCC_FLAG_LPWRRST: Low Power reset
  *
  * @retval The new state of RCC_FLAG (SET or RESET).
  */
static inline bool rcc_get_flag_status(uint8_t RCC_FLAG)
{
#ifdef __cplusplus
	using namespace _internal::rcc;
#endif
  uint32_t tmp = 0;
  uint32_t statusreg = 0;
  /* Get the RCC register index */
  tmp = RCC_FLAG >> 5;
  if (tmp == 1)               /* The flag to check is in CR register */
  {
    statusreg = RCC->CR;
  }
  else if (tmp == 2)          /* The flag to check is in BDCR register */
  {
    statusreg = RCC->BDCR;
  }
  else                       /* The flag to check is in CSR register */
  {
    statusreg = RCC->CSR;
  }
  /* Get the flag position */
  tmp = RCC_FLAG & FLAG_Mask;
  return (statusreg & ((uint32_t)1 << tmp))?true:false;
}


/**
  * @brief  Waits for HSE start-up.
  * @param  None
  * @retval An ErrorStatus enumuration value:
  * - SUCCESS: HSE oscillator is stable and ready to use
  * - ERROR: HSE oscillator not yet ready
  */
static inline bool rcc_wait_for_hse_startup(void)
{
#ifdef __cplusplus
	using namespace _internal::rcc;
#endif
  __IO uint32_t StartUpCounter = 0;
  bool HSEStatus = false;

  /* Wait till HSE is ready and if Time out is reached exit */
  do
  {
    HSEStatus = rcc_get_flag_status(RCC_FLAG_HSERDY);
    StartUpCounter++;
  } while((StartUpCounter != HSE_STARTUP_TIMEOUT) && (HSEStatus == RESET));

  return rcc_get_flag_status(RCC_FLAG_HSERDY)?false:true;
}

/**
  * @brief  Adjusts the Internal High Speed oscillator (HSI) calibration value.
  * @param  HSICalibrationValue: specifies the calibration trimming value.
  *   This parameter must be a number between 0 and 0x1F.
  * @retval None
  */
static inline void rcc_adjust_hsi_calibration_value(uint8_t HSICalibrationValue)
{
#ifdef __cplusplus
	using namespace _internal::rcc;
#endif
  uint32_t tmpreg = 0;
  /* Check the parameters */
  tmpreg = RCC->CR;
  /* Clear HSITRIM[4:0] bits */
  tmpreg &= CR_HSITRIM_Mask;
  /* Set the HSITRIM[4:0] bits according to HSICalibrationValue value */
  tmpreg |= (uint32_t)HSICalibrationValue << 3;
  /* Store the new value */
  RCC->CR = tmpreg;
}

/**
  * @brief  Enables or disables the Internal High Speed oscillator (HSI).
  * @note   HSI can not be stopped if it is used directly or through the PLL as system clock.
  * @param  NewState: new state of the HSI. This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void rcc_hsi_cmd(bool enable)
{
#ifdef __cplusplus
	using namespace _internal::rcc;
#endif
   /* Check the parameters */
  *(__IO uint32_t *) CR_HSION_BB = enable;
}

/**
  * @brief  Configures the PLL clock source and multiplication factor.
  * @note   This function must be used only when the PLL is disabled.
  * @param  RCC_PLLSource: specifies the PLL entry clock source.
  *   For @b STM32_Connectivity_line_devices or @b STM32_Value_line_devices,
  *   this parameter can be one of the following values:
  *     @arg RCC_PLLSource_HSI_Div2: HSI oscillator clock divided by 2 selected as PLL clock entry
  *     @arg RCC_PLLSource_PREDIV1: PREDIV1 clock selected as PLL clock entry
  *   For @b other_STM32_devices, this parameter can be one of the following values:
  *     @arg RCC_PLLSource_HSI_Div2: HSI oscillator clock divided by 2 selected as PLL clock entry
  *     @arg RCC_PLLSource_HSE_Div1: HSE oscillator clock selected as PLL clock entry
  *     @arg RCC_PLLSource_HSE_Div2: HSE oscillator clock divided by 2 selected as PLL clock entry
  * @param  RCC_PLLMul: specifies the PLL multiplication factor.
  *   For @b STM32_Connectivity_line_devices, this parameter can be RCC_PLLMul_x where x:{[4,9], 6_5}
  *   For @b other_STM32_devices, this parameter can be RCC_PLLMul_x where x:[2,16]
  * @retval None
  */
static inline void rcc_pll_config(uint32_t RCC_PLLSource, uint32_t RCC_PLLMul)
{
#ifdef __cplusplus
	using namespace _internal::rcc;
#endif
  uint32_t tmpreg = 0;
  tmpreg = RCC->CFGR;
  /* Clear PLLSRC, PLLXTPRE and PLLMUL[3:0] bits */
  tmpreg &= CFGR_PLL_Mask;
  /* Set the PLL configuration bits */
  tmpreg |= RCC_PLLSource | RCC_PLLMul;
  /* Store the new value */
  RCC->CFGR = tmpreg;
}

/**
  * @brief  Enables or disables the PLL.
  * @note   The PLL can not be disabled if it is used as system clock.
  * @param  NewState: new state of the PLL. This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void rcc_pll_cmd(bool enable)
{
#ifdef __cplusplus
	using namespace _internal::rcc;
#endif
  *(__IO uint32_t *) CR_PLLON_BB = enable;
}

#if defined (STM32F10X_LD_VL) || defined (STM32F10X_MD_VL) || defined (STM32F10X_HD_VL) || defined (STM32F10X_CL)
/**
  * @brief  Configures the PREDIV1 division factor.
  * @note
  *   - This function must be used only when the PLL is disabled.
  *   - This function applies only to STM32 Connectivity line and Value line
  *     devices.
  * @param  RCC_PREDIV1_Source: specifies the PREDIV1 clock source.
  *   This parameter can be one of the following values:
  *     @arg RCC_PREDIV1_Source_HSE: HSE selected as PREDIV1 clock
  *     @arg RCC_PREDIV1_Source_PLL2: PLL2 selected as PREDIV1 clock
  * @note
  *   For @b STM32_Value_line_devices this parameter is always RCC_PREDIV1_Source_HSE
  * @param  RCC_PREDIV1_Div: specifies the PREDIV1 clock division factor.
  *   This parameter can be RCC_PREDIV1_Divx where x:[1,16]
  * @retval None
  */
static inline void rcc_prediv1_config(uint32_t RCC_PREDIV1_Source, uint32_t RCC_PREDIV1_Div)
{
#ifdef __cplusplus
	using namespace _internal::rcc;
#endif
  uint32_t tmpreg = 0;
  tmpreg = RCC->CFGR2;
  /* Clear PREDIV1[3:0] and PREDIV1SRC bits */
  tmpreg &= ~(CFGR2_PREDIV1 | CFGR2_PREDIV1SRC);
  /* Set the PREDIV1 clock source and division factor */
  tmpreg |= RCC_PREDIV1_Source | RCC_PREDIV1_Div ;
  /* Store the new value */
  RCC->CFGR2 = tmpreg;
}
#endif

#ifdef STM32F10X_CL
/**
  * @brief  Configures the PREDIV2 division factor.
  * @note
  *   - This function must be used only when both PLL2 and PLL3 are disabled.
  *   - This function applies only to STM32 Connectivity line devices.
  * @param  RCC_PREDIV2_Div: specifies the PREDIV2 clock division factor.
  *   This parameter can be RCC_PREDIV2_Divx where x:[1,16]
  * @retval None
  */
static inline void rcc_prediv2_config(uint32_t RCC_PREDIV2_Div)
{
#ifdef __cplusplus
	using namespace _internal::rcc;
#endif
   uint32_t tmpreg = 0;
  tmpreg = RCC->CFGR2;
  /* Clear PREDIV2[3:0] bits */
  tmpreg &= ~CFGR2_PREDIV2;
  /* Set the PREDIV2 division factor */
  tmpreg |= RCC_PREDIV2_Div;
  /* Store the new value */
  RCC->CFGR2 = tmpreg;
}

/**
  * @brief  Configures the PLL2 multiplication factor.
  * @note
  *   - This function must be used only when the PLL2 is disabled.
  *   - This function applies only to STM32 Connectivity line devices.
  * @param  RCC_PLL2Mul: specifies the PLL2 multiplication factor.
  *   This parameter can be RCC_PLL2Mul_x where x:{[8,14], 16, 20}
  * @retval None
  */
static inline void rcc_pll2_config(uint32_t RCC_PLL2Mul)
{
#ifdef __cplusplus
	using namespace _internal::rcc;
#endif
  uint32_t tmpreg = 0;

  tmpreg = RCC->CFGR2;
  /* Clear PLL2Mul[3:0] bits */
  tmpreg &= ~CFGR2_PLL2MUL;
  /* Set the PLL2 configuration bits */
  tmpreg |= RCC_PLL2Mul;
  /* Store the new value */
  RCC->CFGR2 = tmpreg;
}


/**
  * @brief  Enables or disables the PLL2.
  * @note
  *   - The PLL2 can not be disabled if it is used indirectly as system clock
  *     (i.e. it is used as PLL clock entry that is used as System clock).
  *   - This function applies only to STM32 Connectivity line devices.
  * @param  NewState: new state of the PLL2. This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void rcc_pll2_cmd(bool enable)
{
#ifdef __cplusplus
	using namespace _internal::rcc;
#endif

  *(__IO uint32_t *) CR_PLL2ON_BB = enable;
}


/**
  * @brief  Configures the PLL3 multiplication factor.
  * @note
  *   - This function must be used only when the PLL3 is disabled.
  *   - This function applies only to STM32 Connectivity line devices.
  * @param  RCC_PLL3Mul: specifies the PLL3 multiplication factor.
  *   This parameter can be RCC_PLL3Mul_x where x:{[8,14], 16, 20}
  * @retval None
  */
static inline void rcc_pll3_config(uint32_t RCC_PLL3Mul)
{
#ifdef __cplusplus
	using namespace _internal::rcc;
#endif
  uint32_t tmpreg = 0;

  tmpreg = RCC->CFGR2;
  /* Clear PLL3Mul[3:0] bits */
  tmpreg &= ~CFGR2_PLL3MUL;
  /* Set the PLL3 configuration bits */
  tmpreg |= RCC_PLL3Mul;
  /* Store the new value */
  RCC->CFGR2 = tmpreg;
}


/**
  * @brief  Enables or disables the PLL3.
  * @note   This function applies only to STM32 Connectivity line devices.
  * @param  NewState: new state of the PLL3. This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void rcc_pll3_cmd(bool enable)
{
#ifdef __cplusplus
	using namespace _internal::rcc;
#endif
  *(__IO uint32_t *) CR_PLL3ON_BB = enable;
}
#endif /* STM32F10X_CL */

/**
  * @brief  Configures the system clock (SYSCLK).
  * @param  RCC_SYSCLKSource: specifies the clock source used as system clock.
  *   This parameter can be one of the following values:
  *     @arg RCC_SYSCLKSource_HSI: HSI selected as system clock
  *     @arg RCC_SYSCLKSource_HSE: HSE selected as system clock
  *     @arg RCC_SYSCLKSource_PLLCLK: PLL selected as system clock
  * @retval None
  */
static inline void rcc_sysclk_config(uint32_t RCC_SYSCLKSource)
{
#ifdef __cplusplus
	using namespace _internal::rcc;
#endif
  uint32_t tmpreg = 0;
  tmpreg = RCC->CFGR;
  /* Clear SW[1:0] bits */
  tmpreg &= CFGR_SW_Mask;
  /* Set SW[1:0] bits according to RCC_SYSCLKSource value */
  tmpreg |= RCC_SYSCLKSource;
  /* Store the new value */
  RCC->CFGR = tmpreg;
}

/**
  * @brief  Returns the clock source used as system clock.
  * @param  None
  * @retval The clock source used as system clock. The returned value can
  *   be one of the following:
  *     - 0x00: HSI used as system clock
  *     - 0x04: HSE used as system clock
  *     - 0x08: PLL used as system clock
  */
static inline uint8_t rcc_get_sysclk_source(void)
{
#ifdef __cplusplus
	using namespace _internal::rcc;
#endif
  return ((uint8_t)(RCC->CFGR & CFGR_SWS_Mask));
}

/**
  * @brief  Configures the AHB clock (HCLK).
  * @param  RCC_SYSCLK: defines the AHB clock divider. This clock is derived from
  *   the system clock (SYSCLK).
  *   This parameter can be one of the following values:
  *     @arg RCC_SYSCLK_Div1: AHB clock = SYSCLK
  *     @arg RCC_SYSCLK_Div2: AHB clock = SYSCLK/2
  *     @arg RCC_SYSCLK_Div4: AHB clock = SYSCLK/4
  *     @arg RCC_SYSCLK_Div8: AHB clock = SYSCLK/8
  *     @arg RCC_SYSCLK_Div16: AHB clock = SYSCLK/16
  *     @arg RCC_SYSCLK_Div64: AHB clock = SYSCLK/64
  *     @arg RCC_SYSCLK_Div128: AHB clock = SYSCLK/128
  *     @arg RCC_SYSCLK_Div256: AHB clock = SYSCLK/256
  *     @arg RCC_SYSCLK_Div512: AHB clock = SYSCLK/512
  * @retval None
  */
static inline void rcc_hclk_config( uint32_t RCC_SYSCLK )
{
#ifdef __cplusplus
	using namespace _internal::rcc;
#endif
  uint32_t tmpreg = 0;
  tmpreg = RCC->CFGR;
  /* Clear HPRE[3:0] bits */
  tmpreg &= CFGR_HPRE_Reset_Mask;
  /* Set HPRE[3:0] bits according to RCC_SYSCLK value */
  tmpreg |= RCC_SYSCLK;
  /* Store the new value */
  RCC->CFGR = tmpreg;
}

/**
  * @brief  Configures the Low Speed APB clock (PCLK1).
  * @param  RCC_HCLK: defines the APB1 clock divider. This clock is derived from
  *   the AHB clock (HCLK).
  *   This parameter can be one of the following values:
  *     @arg RCC_HCLK_Div1: APB1 clock = HCLK
  *     @arg RCC_HCLK_Div2: APB1 clock = HCLK/2
  *     @arg RCC_HCLK_Div4: APB1 clock = HCLK/4
  *     @arg RCC_HCLK_Div8: APB1 clock = HCLK/8
  *     @arg RCC_HCLK_Div16: APB1 clock = HCLK/16
  * @retval None
  */
static inline void rcc_pclk1_config(uint32_t RCC_HCLK)
{
#ifdef __cplusplus
	using namespace _internal::rcc;
#endif
  uint32_t tmpreg = 0;
  tmpreg = RCC->CFGR;
  /* Clear PPRE1[2:0] bits */
  tmpreg &= CFGR_PPRE1_Reset_Mask;
  /* Set PPRE1[2:0] bits according to RCC_HCLK value */
  tmpreg |= RCC_HCLK;
  /* Store the new value */
  RCC->CFGR = tmpreg;
}

/**
  * @brief  Configures the High Speed APB clock (PCLK2).
  * @param  RCC_HCLK: defines the APB2 clock divider. This clock is derived from
  *   the AHB clock (HCLK).
  *   This parameter can be one of the following values:
  *     @arg RCC_HCLK_Div1: APB2 clock = HCLK
  *     @arg RCC_HCLK_Div2: APB2 clock = HCLK/2
  *     @arg RCC_HCLK_Div4: APB2 clock = HCLK/4
  *     @arg RCC_HCLK_Div8: APB2 clock = HCLK/8
  *     @arg RCC_HCLK_Div16: APB2 clock = HCLK/16
  * @retval None
  */
static inline void rcc_pclk2_config(uint32_t RCC_HCLK)
{
#ifdef __cplusplus
	using namespace _internal::rcc;
#endif
  uint32_t tmpreg = 0;
  tmpreg = RCC->CFGR;
  /* Clear PPRE2[2:0] bits */
  tmpreg &= CFGR_PPRE2_Reset_Mask;
  /* Set PPRE2[2:0] bits according to RCC_HCLK value */
  tmpreg |= RCC_HCLK << 3;
  /* Store the new value */
  RCC->CFGR = tmpreg;
}

/**
  * @brief  Enables or disables the specified RCC interrupts.
  * @param  RCC_IT: specifies the RCC interrupt sources to be enabled or disabled.
  *
  *   For @b STM32_Connectivity_line_devices, this parameter can be any combination
  *   of the following values
  *     @arg RCC_IT_LSIRDY: LSI ready interrupt
  *     @arg RCC_IT_LSERDY: LSE ready interrupt
  *     @arg RCC_IT_HSIRDY: HSI ready interrupt
  *     @arg RCC_IT_HSERDY: HSE ready interrupt
  *     @arg RCC_IT_PLLRDY: PLL ready interrupt
  *     @arg RCC_IT_PLL2RDY: PLL2 ready interrupt
  *     @arg RCC_IT_PLL3RDY: PLL3 ready interrupt
  *
  *   For @b other_STM32_devices, this parameter can be any combination of the
  *   following values
  *     @arg RCC_IT_LSIRDY: LSI ready interrupt
  *     @arg RCC_IT_LSERDY: LSE ready interrupt
  *     @arg RCC_IT_HSIRDY: HSI ready interrupt
  *     @arg RCC_IT_HSERDY: HSE ready interrupt
  *     @arg RCC_IT_PLLRDY: PLL ready interrupt
  *
  * @param  NewState: new state of the specified RCC interrupts.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void rcc_it_config(uint8_t RCC_IT, bool enable)
{
#ifdef __cplusplus
	using namespace _internal::rcc;
#endif
  if ( enable )
  {
    /* Perform Byte access to RCC_CIR bits to enable the selected interrupts */
    *(__IO uint8_t *) CIR_BYTE2_ADDRESS |= RCC_IT;
  }
  else
  {
    /* Perform Byte access to RCC_CIR bits to disable the selected interrupts */
    *(__IO uint8_t *) CIR_BYTE2_ADDRESS &= (uint8_t)~RCC_IT;
  }
}

#ifndef STM32F10X_CL
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
#ifdef __cplusplus
	using namespace _internal::rcc;
#endif

  *(__IO uint32_t *) CFGR_USBPRE_BB = RCC_USBCLKSource;
}
#else
/**
  * @brief  Configures the USB OTG FS clock (OTGFSCLK).
  *   This function applies only to STM32 Connectivity line devices.
  * @param  RCC_OTGFSCLKSource: specifies the USB OTG FS clock source.
  *   This clock is derived from the PLL output.
  *   This parameter can be one of the following values:
  *     @arg  RCC_OTGFSCLKSource_PLLVCO_Div3: PLL VCO clock divided by 2 selected as USB OTG FS clock source
  *     @arg  RCC_OTGFSCLKSource_PLLVCO_Div2: PLL VCO clock divided by 2 selected as USB OTG FS clock source
  * @retval None
  */
static inline void rcc_otgfs_clk_config(uint32_t RCC_OTGFSCLKSource)
{
#ifdef __cplusplus
	using namespace _internal::rcc;
#endif

  *(__IO uint32_t *) CFGR_OTGFSPRE_BB = RCC_OTGFSCLKSource;
}
#endif /* STM32F10X_CL */

/**
  * @brief  Configures the ADC clock (ADCCLK).
  * @param  RCC_PCLK2: defines the ADC clock divider. This clock is derived from
  *   the APB2 clock (PCLK2).
  *   This parameter can be one of the following values:
  *     @arg RCC_PCLK2_Div2: ADC clock = PCLK2/2
  *     @arg RCC_PCLK2_Div4: ADC clock = PCLK2/4
  *     @arg RCC_PCLK2_Div6: ADC clock = PCLK2/6
  *     @arg RCC_PCLK2_Div8: ADC clock = PCLK2/8
  * @retval None
  */
static inline void rcc_adc_clk_config(uint32_t RCC_PCLK2)
{
#ifdef __cplusplus
	using namespace _internal::rcc;
#endif
  uint32_t tmpreg = 0;
  tmpreg = RCC->CFGR;
  /* Clear ADCPRE[1:0] bits */
  tmpreg &= CFGR_ADCPRE_Reset_Mask;
  /* Set ADCPRE[1:0] bits according to RCC_PCLK2 value */
  tmpreg |= RCC_PCLK2;
  /* Store the new value */
  RCC->CFGR = tmpreg;
}

#ifdef STM32F10X_CL
/**
  * @brief  Configures the I2S2 clock source(I2S2CLK).
  * @note
  *   - This function must be called before enabling I2S2 APB clock.
  *   - This function applies only to STM32 Connectivity line devices.
  * @param  RCC_I2S2CLKSource: specifies the I2S2 clock source.
  *   This parameter can be one of the following values:
  *     @arg RCC_I2S2CLKSource_SYSCLK: system clock selected as I2S2 clock entry
  *     @arg RCC_I2S2CLKSource_PLL3_VCO: PLL3 VCO clock selected as I2S2 clock entry
  * @retval None
  */
static inline void rcc_i2s2_clk_config(uint32_t RCC_I2S2CLKSource)
{
#ifdef __cplusplus
	using namespace _internal::rcc;
#endif

  *(__IO uint32_t *) CFGR2_I2S2SRC_BB = RCC_I2S2CLKSource;
}

/**
  * @brief  Configures the I2S3 clock source(I2S2CLK).
  * @note
  *   - This function must be called before enabling I2S3 APB clock.
  *   - This function applies only to STM32 Connectivity line devices.
  * @param  RCC_I2S3CLKSource: specifies the I2S3 clock source.
  *   This parameter can be one of the following values:
  *     @arg RCC_I2S3CLKSource_SYSCLK: system clock selected as I2S3 clock entry
  *     @arg RCC_I2S3CLKSource_PLL3_VCO: PLL3 VCO clock selected as I2S3 clock entry
  * @retval None
  */
static inline void rcc_i2s3_clk_config(uint32_t RCC_I2S3CLKSource)
{
#ifdef __cplusplus
	using namespace _internal::rcc;
#endif
  *(__IO uint32_t *) CFGR2_I2S3SRC_BB = RCC_I2S3CLKSource;
}
#endif /* STM32F10X_CL */

/**
  * @brief  Configures the External Low Speed oscillator (LSE).
  * @param  RCC_LSE: specifies the new state of the LSE.
  *   This parameter can be one of the following values:
  *     @arg RCC_LSE_OFF: LSE oscillator OFF
  *     @arg RCC_LSE_ON: LSE oscillator ON
  *     @arg RCC_LSE_Bypass: LSE oscillator bypassed with external clock
  * @retval None
  */
static inline void rcc_lse_config(uint8_t RCC_LSE)
{
#ifdef __cplusplus
	using namespace _internal::rcc;
#endif
  /* Reset LSEON and LSEBYP bits before configuring the LSE ------------------*/
  /* Reset LSEON bit */
  *(__IO uint8_t *) BDCR_ADDRESS = RCC_LSE_OFF;
  /* Reset LSEBYP bit */
  *(__IO uint8_t *) BDCR_ADDRESS = RCC_LSE_OFF;
  /* Configure LSE (RCC_LSE_OFF is already covered by the code section above) */
  switch(RCC_LSE)
  {
    case RCC_LSE_ON:
      /* Set LSEON bit */
      *(__IO uint8_t *) BDCR_ADDRESS = RCC_LSE_ON;
      break;

    case RCC_LSE_Bypass:
      /* Set LSEBYP and LSEON bits */
      *(__IO uint8_t *) BDCR_ADDRESS = RCC_LSE_Bypass | RCC_LSE_ON;
      break;

    default:
      break;
  }
}

/**
  * @brief  Enables or disables the Internal Low Speed oscillator (LSI).
  * @note   LSI can not be disabled if the IWDG is running.
  * @param  NewState: new state of the LSI. This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void rcc_lsi_cmd(bool enable)
{
#ifdef __cplusplus
	using namespace _internal::rcc;
#endif
  *(__IO uint32_t *) CSR_LSION_BB = enable;
}

/**
  * @brief  Configures the RTC clock (RTCCLK).
  * @note   Once the RTC clock is selected it can't be changed unless the Backup domain is reset.
  * @param  RCC_RTCCLKSource: specifies the RTC clock source.
  *   This parameter can be one of the following values:
  *     @arg RCC_RTCCLKSource_LSE: LSE selected as RTC clock
  *     @arg RCC_RTCCLKSource_LSI: LSI selected as RTC clock
  *     @arg RCC_RTCCLKSource_HSE_Div128: HSE clock divided by 128 selected as RTC clock
  * @retval None
  */
static inline void rcc_rtc_clk_config(uint32_t RCC_RTCCLKSource)
{
#ifdef __cplusplus
	using namespace _internal::rcc;
#endif
  /* Select the RTC clock source */
  RCC->BDCR |= RCC_RTCCLKSource;
}


/**
  * @brief  Enables or disables the RTC clock.
  * @note   This function must be used only after the RTC clock was selected using the RCC_RTCCLKConfig function.
  * @param  NewState: new state of the RTC clock. This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void rcc_rtc_clk_cmd(bool enable )
{
#ifdef __cplusplus
	using namespace _internal::rcc;
#endif
  *(__IO uint32_t *) BDCR_RTCEN_BB = (uint32_t)enable;
}

/**
 * @brief Enable or disable selected perhipheral
 * @param clock_type Clock type depends on the bus
 * @param periph  Perhipherals bitmask
 * @param enable Enable or disable
 */
enum rcc_clock_type
{
	rcc_clk_ahb,
	rcc_clk_apb2,
	rcc_clk_apb1
};

static inline void rcc_periph_clock_cmd( enum rcc_clock_type clock_type, uint32_t periph, bool enable )
{
	switch( clock_type )
	{
	case rcc_clk_ahb:
		if( enable ) RCC->AHBENR |= periph;
		else RCC->AHBENR &= ~periph;
		break;
	case rcc_clk_apb2:
		if( enable ) RCC->APB2ENR |= periph;
		else RCC->APB2ENR &= ~periph;
		break;
	case rcc_clk_apb1:
		if( enable ) RCC->APB1ENR |= periph;
		else RCC->APB1ENR &= ~periph;
		break;
	}
}

/** Compatibility with old stdlib */
static inline void rcc_apb2_periph_clock_cmd( uint32_t periph, bool enable)
{
    rcc_periph_clock_cmd(rcc_clk_apb2, periph, enable);
}

static inline void rcc_apb1_periph_clock_cmd( uint32_t periph, bool enable)
{
    rcc_periph_clock_cmd(rcc_clk_apb1, periph, enable);
}

static inline void rcc_ahb_periph_clock_cmd( uint32_t periph, bool enable)
{
    rcc_periph_clock_cmd(rcc_clk_ahb, periph, enable);
}

/**
 * @brief Set reset on selected peripheral
 * @param clock_type Clock type depends on the bus
 * @param periph  Perhipherals bitmask
 * @param enable Enable or disable
 */
#ifdef STM32F10X_CL
static inline void rcc_ahb_periph_reset_cmd(uint32_t periph, bool enable )
{
  if ( enable )
  {
    RCC->AHBRSTR |= periph;
  }
  else
  {
    RCC->AHBRSTR &= ~periph;
  }
}
#endif

/**
 * @brief Set reset on selected peripheral
 * @param clock_type Clock type depends on the bus
 * @param periph  Perhipherals bitmask
 * @param enable Enable or disable
 */
static inline void rcc_apb2_periph_reset_cmd(uint32_t periph, bool enable )
{
  if ( enable )
  {
	  RCC->APB2RSTR |= periph;
  }
  else
  {
	  RCC->APB2RSTR &= ~periph;
  }
}

/**
 * @brief Set reset on selected peripheral
 * @param clock_type Clock type depends on the bus
 * @param periph  Perhipherals bitmask
 * @param enable Enable or disable
 */
static inline void rcc_apb1_periph_reset_cmd(uint32_t periph, bool enable )
{
  if ( enable )
  {
	  RCC->APB1RSTR |= periph;
  }
  else
  {
	  RCC->APB1RSTR &= ~periph;
  }
}

/**
 * @brief Set reset on selected peripheral
 * @param clock_type Clock type depends on the bus
 * @param periph  Perhipherals bitmask
 * @param enable Enable or disable
 */
static inline void rcc_periph_reset_cmd(enum rcc_clock_type clock_type, uint32_t periph, bool enable )
{
	switch( clock_type )
	{
	case rcc_clk_ahb:
#ifdef STM32F10X_CL
		rcc_ahb_periph_reset_cmd( periph, enable );
#endif
		break;
	case rcc_clk_apb2:
		rcc_apb2_periph_reset_cmd( periph, enable );
		break;
	case rcc_clk_apb1:
		rcc_apb1_periph_reset_cmd( periph, enable );
		break;
	}
}

/**
  * @brief  Forces or releases the Backup domain reset.
  * @param  NewState: new state of the Backup domain reset.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void rcc_backup_reset_cmd(bool enable)
{
#ifdef __cplusplus
	using namespace _internal::rcc;
#endif
  *(__IO uint32_t *) BDCR_BDRST_BB = enable;
}

/**
  * @brief  Enables or disables the Clock Security System.
  * @param  NewState: new state of the Clock Security System..
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void rcc_clock_security_system_cmd(bool enable)
{
#ifdef __cplusplus
	using namespace _internal::rcc;
#endif
  *(__IO uint32_t *) CR_CSSON_BB = enable;
}

/**
  * @brief  Selects the clock source to output on MCO pin.
  * @param  RCC_MCO: specifies the clock source to output.
  *
  *   For @b STM32_Connectivity_line_devices, this parameter can be one of the
  *   following values:
  *     @arg RCC_MCO_NoClock: No clock selected
  *     @arg RCC_MCO_SYSCLK: System clock selected
  *     @arg RCC_MCO_HSI: HSI oscillator clock selected
  *     @arg RCC_MCO_HSE: HSE oscillator clock selected
  *     @arg RCC_MCO_PLLCLK_Div2: PLL clock divided by 2 selected
  *     @arg RCC_MCO_PLL2CLK: PLL2 clock selected
  *     @arg RCC_MCO_PLL3CLK_Div2: PLL3 clock divided by 2 selected
  *     @arg RCC_MCO_XT1: External 3-25 MHz oscillator clock selected
  *     @arg RCC_MCO_PLL3CLK: PLL3 clock selected
  *
  *   For  @b other_STM32_devices, this parameter can be one of the following values:
  *     @arg RCC_MCO_NoClock: No clock selected
  *     @arg RCC_MCO_SYSCLK: System clock selected
  *     @arg RCC_MCO_HSI: HSI oscillator clock selected
  *     @arg RCC_MCO_HSE: HSE oscillator clock selected
  *     @arg RCC_MCO_PLLCLK_Div2: PLL clock divided by 2 selected
  *
  * @retval None
  */
static inline void rcc_mco_config(uint8_t RCC_MCO)
{
#ifdef __cplusplus
	using namespace _internal::rcc;
#endif

  /* Perform Byte access to MCO bits to select the MCO source */
  *(__IO uint8_t *) CFGR_BYTE4_ADDRESS = RCC_MCO;
}

/**
  * @brief  Clears the RCC reset flags.
  * @note   The reset flags are: RCC_FLAG_PINRST, RCC_FLAG_PORRST, RCC_FLAG_SFTRST,
  *   RCC_FLAG_IWDGRST, RCC_FLAG_WWDGRST, RCC_FLAG_LPWRRST
  * @param  None
  * @retval None
  */
static inline void rcc_clear_flag(void)
{
#ifdef __cplusplus
	using namespace _internal::rcc;
#endif
  /* Set RMVF bit to clear the reset flags */
  RCC->CSR |= CSR_RMVF_Set;
}

/**
  * @brief  Checks whether the specified RCC interrupt has occurred or not.
  * @param  RCC_IT: specifies the RCC interrupt source to check.
  *
  *   For @b STM32_Connectivity_line_devices, this parameter can be one of the
  *   following values:
  *     @arg RCC_IT_LSIRDY: LSI ready interrupt
  *     @arg RCC_IT_LSERDY: LSE ready interrupt
  *     @arg RCC_IT_HSIRDY: HSI ready interrupt
  *     @arg RCC_IT_HSERDY: HSE ready interrupt
  *     @arg RCC_IT_PLLRDY: PLL ready interrupt
  *     @arg RCC_IT_PLL2RDY: PLL2 ready interrupt
  *     @arg RCC_IT_PLL3RDY: PLL3 ready interrupt
  *     @arg RCC_IT_CSS: Clock Security System interrupt
  *
  *   For @b other_STM32_devices, this parameter can be one of the following values:
  *     @arg RCC_IT_LSIRDY: LSI ready interrupt
  *     @arg RCC_IT_LSERDY: LSE ready interrupt
  *     @arg RCC_IT_HSIRDY: HSI ready interrupt
  *     @arg RCC_IT_HSERDY: HSE ready interrupt
  *     @arg RCC_IT_PLLRDY: PLL ready interrupt
  *     @arg RCC_IT_CSS: Clock Security System interrupt
  *
  * @retval The new state of RCC_IT (SET or RESET).
  */
static inline bool rcc_get_it_status(uint8_t RCC_IT)
{
#ifdef __cplusplus
	using namespace _internal::rcc;
#endif

  /* Check the status of the specified RCC interrupt */
  return (RCC->CIR & RCC_IT)?true:false;

}

/**
  * @brief  Clears the RCC's interrupt pending bits.
  * @param  RCC_IT: specifies the interrupt pending bit to clear.
  *
  *   For @b STM32_Connectivity_line_devices, this parameter can be any combination
  *   of the following values:
  *     @arg RCC_IT_LSIRDY: LSI ready interrupt
  *     @arg RCC_IT_LSERDY: LSE ready interrupt
  *     @arg RCC_IT_HSIRDY: HSI ready interrupt
  *     @arg RCC_IT_HSERDY: HSE ready interrupt
  *     @arg RCC_IT_PLLRDY: PLL ready interrupt
  *     @arg RCC_IT_PLL2RDY: PLL2 ready interrupt
  *     @arg RCC_IT_PLL3RDY: PLL3 ready interrupt
  *     @arg RCC_IT_CSS: Clock Security System interrupt
  *
  *   For @b other_STM32_devices, this parameter can be any combination of the
  *   following values:
  *     @arg RCC_IT_LSIRDY: LSI ready interrupt
  *     @arg RCC_IT_LSERDY: LSE ready interrupt
  *     @arg RCC_IT_HSIRDY: HSI ready interrupt
  *     @arg RCC_IT_HSERDY: HSE ready interrupt
  *     @arg RCC_IT_PLLRDY: PLL ready interrupt
  *
  *     @arg RCC_IT_CSS: Clock Security System interrupt
  * @retval None
  */
static inline void rcc_clear_it_pending_bit(uint8_t RCC_IT)
{
#ifdef __cplusplus
	using namespace _internal::rcc;
#endif
  /* Perform Byte access to RCC_CIR[23:16] bits to clear the selected interrupt
     pending bits */
  *(__IO uint8_t *) CIR_BYTE3_ADDRESS = RCC_IT;
}



/*****    EXTENDED ISIX LIBRARY FUNCTIONS  *******/

/** Setup the best flash latency according to the CPU speed
 * @param[in] frequency Sysclk frequency */
static inline void rcc_flash_latency(uint32_t frequency)
{
#if !defined(STM32F10X_LD_VL) && !defined(STM32F10X_MD_VL) && !defined(STM32F10X_HD_VL)
	uint32_t wait_states;
	if(frequency > 72000000ul) frequency = 72000000ul;
	wait_states = (frequency-1) / 24000000ul;	// calculate wait_states (30M is valid for 2.7V to 3.6V voltage range, use 24M for 2.4V to 2.7V, 18M for 2.1V to 2.4V or 16M for  1.8V to 2.1V)
	wait_states &= FLASH_ACR_LATENCY;			// trim to max allowed value - 7
	FLASH->ACR = wait_states;				    // set wait_states, disable all caches and prefetch
	FLASH->ACR = FLASH_ACR_PRFTBE  | wait_states;	// enable caches and prefetch
#else
	(void)frequency;
#endif
}


//! Structure for defining sysclk mode
enum e_sysclk_mode
{
	e_sysclk_hsi,		//! hi speed internal oscilator only
	e_sysclk_hse,		//! hi speed external oscilator only
	e_sysclk_hsi_pll,	//! hi speed internal PLL
	e_sysclk_hse_pll	//! hi speed external PLL
};

#define PLL1_Bit_Shift 18ul
#define PLL1_Bit_Mask 0xf
#define PLL1_Mul_Offset  2ul
#define PRE1Div_Mask 0x0F
#define PRE1Div_Offset  1ul

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
	uint32_t best_frequency_core = 0;
	if( mode == e_sysclk_hse_pll || mode == e_sysclk_hse )
		RCC->CR  |= RCC_CR_HSEON;
	else
		crystal = 8000000ul;
	if( mode == e_sysclk_hsi_pll ) crystal /= 2;
	if( mode == e_sysclk_hse_pll || mode == e_sysclk_hsi_pll )
	{
		uint32_t div, mul, vco_input_frequency, frequency_core;
		uint32_t best_div __attribute__((unused)) = 0;
		uint32_t best_mul = 0;
#if defined(STM32F10X_LD_VL) || defined(STM32F10X_MD_VL) || \
	defined(STM32F10X_HD_VL) || defined(STM32F10X_CL)
		for (div = 1; div <= 16; div++)			// PLL divider
		{
#elif defined(STM32F10X_LD) || defined(STM32F10X_MD) || \
	  defined(STM32F10X_HD) || defined(STM32F10X_XL)
		for (div = 1; div <= 1; div++)			// PLL divider
		{
#else
#error Unknown MCU type
#endif
			vco_input_frequency = crystal / div;
#if defined(STM32F10X_LD_VL) || defined(STM32F10X_MD_VL) || \
	defined(STM32F10X_HD_VL) || defined(STM32F10X_LD) || defined(STM32F10X_MD) || \
	defined(STM32F10X_HD) || defined(STM32F10X_XL)
			for (mul = 2; mul <= 16; mul++)	// Multiply
#elif defined(STM32F10X_CL)
			for (mul = 4; mul <= 9; mul++)	// Multiply
#else
#error Unknown MCU type
#endif
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
#if defined(STM32F10X_LD_VL) || defined(STM32F10X_MD_VL) || \
	defined(STM32F10X_HD_VL) || defined(STM32F10X_CL)
	   RCC->CFGR2 = (best_div - PRE1Div_Offset) & PRE1Div_Mask;
	   if( mode == e_sysclk_hse_pll ) RCC->CFGR |= RCC_CFGR_PLLSRC;
#endif
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
#undef  PLL1_Bit_Shift
#undef  PLL1_Bit_Mask
#undef PLL1_Mul_Offset
#undef PRE1Div_Mask
#undef PRE1Div_Offset



#ifdef __cplusplus
}
#else	//Undefine local #define
#undef RCC_OFFSET
#undef CR_OFFSET
#undef HSION_BitNumber
#undef CR_HSION_BB
#undef PLLON_BitNumber
#undef CR_PLLON_BB
#undef PLL2ON_BitNumber
#undef CR_PLL2ON_BB
#undef PLL3ON_BitNumber
#undef CR_PLL3ON_BB
#undef CSSON_BitNumber
#undef CR_CSSON_BB
#undef CFGR_OFFSET
#undef USBPRE_BitNumber
#undef CFGR_USBPRE_BB
#undef OTGFSPRE_BitNumber
#undef CFGR_OTGFSPRE_BB
#undef BDCR_OFFSET
#undef RTCEN_BitNumber
#undef BDCR_RTCEN_BB
#undef BDRST_BitNumber
#undef BDCR_BDRST_BB
#undef CSR_OFFSET
#undef LSION_BitNumber
#undef CSR_LSION_BB
#undef CFGR2_OFFSET
#undef I2S2SRC_BitNumber
#undef CFGR2_I2S2SRC_BB
#undef I2S3SRC_BitNumber
#undef CFGR2_I2S3SRC_BB
#undef CR_HSEBYP_Reset
#undef CR_HSEBYP_Set
#undef CR_HSEON_Reset
#undef CR_HSEON_Set
#undef CR_HSITRIM_Mask
#undef CFGR_PLL_Mask
#undef CFGR_PLLMull_Mask
#undef CFGR_PLLSRC_Mask
#undef CFGR_PLLXTPRE_Mask
#undef CFGR_SWS_Mask
#undef CFGR_SW_Mask
#undef CFGR_HPRE_Reset_Mask
#undef CFGR_HPRE_Set_Mask
#undef CFGR_PPRE1_Reset_Mask
#undef CFGR_PPRE1_Set_Mask
#undef CFGR_PPRE2_Reset_Mask
#undef CFGR_PPRE2_Set_Mask
#undef CFGR_ADCPRE_Reset_Mask
#undef CFGR_ADCPRE_Set_Mask
#undef CFGR2_PREDIV1SRC
#undef CFGR2_PREDIV1
#undef CFGR2_PREDIV2
#undef CFGR2_PLL2MUL
#undef CFGR2_PLL3MUL
#undef FLAG_Mask
#undef CIR_BYTE2_ADDRESS
#undef CIR_BYTE3_ADDRESS
#undef CFGR_BYTE4_ADDRESS
#undef BDCR_ADDRESS
#endif

#endif /* STM32RCC_H_ */
