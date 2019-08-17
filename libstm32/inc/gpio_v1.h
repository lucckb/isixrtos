/*
 * gpio_f1x.h
 *
 *  Created on: 16-07-2012
 *      Author: lucck
 */

#ifndef STM32_GPIO_F1X_H_
#define STM32_GPIO_F1X_H_
/*----------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>
#include "stm32lib.h"
/*----------------------------------------------------------*/
#ifdef __cplusplus
namespace stm32 {
#endif
/*----------------------------------------------------------*/
#ifdef __cplusplus
static const uint32_t GPIO_Remap_SPI1             = 0x00000001;
static const uint32_t GPIO_Remap_I2C1             = 0x00000002;
static const uint32_t GPIO_Remap_USART1           = 0x00000004;
static const uint32_t GPIO_Remap_USART2           = 0x00000008;
static const uint32_t GPIO_PartialRemap_USART3    = 0x00140010;
static const uint32_t GPIO_FullRemap_USART3       = 0x00140030;
static const uint32_t GPIO_PartialRemap_TIM1      = 0x00160040;
static const uint32_t GPIO_FullRemap_TIM1         = 0x001600C0;
static const uint32_t GPIO_PartialRemap1_TIM2     = 0x00180100;
static const uint32_t GPIO_PartialRemap2_TIM2     = 0x00180200;
static const uint32_t GPIO_FullRemap_TIM2         = 0x00180300;
static const uint32_t GPIO_PartialRemap_TIM3      = 0x001A0800;
static const uint32_t GPIO_FullRemap_TIM3         = 0x001A0C00;
static const uint32_t GPIO_Remap_TIM4             = 0x00001000;
static const uint32_t GPIO_Remap1_CAN1            = 0x001D4000;
static const uint32_t GPIO_Remap2_CAN1            = 0x001D6000;
static const uint32_t GPIO_Remap_PD01             = 0x00008000;
static const uint32_t GPIO_Remap_TIM5CH4_LSI      = 0x00200001;
static const uint32_t GPIO_Remap_ADC1_ETRGINJ     = 0x00200002;
static const uint32_t GPIO_Remap_ADC1_ETRGREG     = 0x00200004;
static const uint32_t GPIO_Remap_ADC2_ETRGINJ     = 0x00200008;
static const uint32_t GPIO_Remap_ADC2_ETRGREG     = 0x00200010;
static const uint32_t GPIO_Remap_ETH              = 0x00200020;
static const uint32_t GPIO_Remap_CAN2             = 0x00200040;
static const uint32_t GPIO_Remap_SWJ_NoJTRST      = 0x00300100;
static const uint32_t GPIO_Remap_SWJ_JTAGDisable  = 0x00300200;
static const uint32_t GPIO_Remap_SWJ_Disable      = 0x00300400;
static const uint32_t GPIO_Remap_SPI3             = 0x00201100;
static const uint32_t GPIO_Remap_TIM2ITR1_PTP_SOF = 0x00202000;
static const uint32_t GPIO_Remap_PTP_PPS          = 0x00204000;
static const uint32_t GPIO_Remap_TIM15            = 0x80000001;
static const uint32_t GPIO_Remap_TIM16            = 0x80000002;
static const uint32_t GPIO_Remap_TIM17            = 0x80000004;
static const uint32_t GPIO_Remap_CEC              = 0x80000008;
static const uint32_t GPIO_Remap_TIM1_DMA         = 0x80000010;
static const uint32_t GPIO_Remap_TIM9             = 0x80000020;
static const uint32_t GPIO_Remap_TIM10            = 0x80000040;
static const uint32_t GPIO_Remap_TIM11            = 0x80000080;
static const uint32_t GPIO_Remap_TIM13            = 0x80000100;
static const uint32_t GPIO_Remap_TIM14            = 0x80000200;
static const uint32_t GPIO_Remap_FSMC_NADV        = 0x80000400;
static const uint32_t GPIO_Remap_TIM67_DAC_DMA    = 0x80000800;
static const uint32_t GPIO_Remap_TIM12            = 0x80001000;
static const uint32_t GPIO_Remap_MISC             = 0x80002000;

#else

#define GPIO_Remap_SPI1             ((uint32_t)0x00000001)  /*!< SPI1 Alternate Function mapping */
#define GPIO_Remap_I2C1             ((uint32_t)0x00000002)  /*!< I2C1 Alternate Function mapping */
#define GPIO_Remap_USART1           ((uint32_t)0x00000004)  /*!< USART1 Alternate Function mapping */
#define GPIO_Remap_USART2           ((uint32_t)0x00000008)  /*!< USART2 Alternate Function mapping */
#define GPIO_PartialRemap_USART3    ((uint32_t)0x00140010)  /*!< USART3 Partial Alternate Function mapping */
#define GPIO_FullRemap_USART3       ((uint32_t)0x00140030)  /*!< USART3 Full Alternate Function mapping */
#define GPIO_PartialRemap_TIM1      ((uint32_t)0x00160040)  /*!< TIM1 Partial Alternate Function mapping */
#define GPIO_FullRemap_TIM1         ((uint32_t)0x001600C0)  /*!< TIM1 Full Alternate Function mapping */
#define GPIO_PartialRemap1_TIM2     ((uint32_t)0x00180100)  /*!< TIM2 Partial1 Alternate Function mapping */
#define GPIO_PartialRemap2_TIM2     ((uint32_t)0x00180200)  /*!< TIM2 Partial2 Alternate Function mapping */
#define GPIO_FullRemap_TIM2         ((uint32_t)0x00180300)  /*!< TIM2 Full Alternate Function mapping */
#define GPIO_PartialRemap_TIM3      ((uint32_t)0x001A0800)  /*!< TIM3 Partial Alternate Function mapping */
#define GPIO_FullRemap_TIM3         ((uint32_t)0x001A0C00)  /*!< TIM3 Full Alternate Function mapping */
#define GPIO_Remap_TIM4             ((uint32_t)0x00001000)  /*!< TIM4 Alternate Function mapping */
#define GPIO_Remap1_CAN1            ((uint32_t)0x001D4000)  /*!< CAN1 Alternate Function mapping */
#define GPIO_Remap2_CAN1            ((uint32_t)0x001D6000)  /*!< CAN1 Alternate Function mapping */
#define GPIO_Remap_PD01             ((uint32_t)0x00008000)  /*!< PD01 Alternate Function mapping */
#define GPIO_Remap_TIM5CH4_LSI      ((uint32_t)0x00200001)  /*!< LSI connected to TIM5 Channel4 input capture for calibration */
#define GPIO_Remap_ADC1_ETRGINJ     ((uint32_t)0x00200002)  /*!< ADC1 External Trigger Injected Conversion remapping */
#define GPIO_Remap_ADC1_ETRGREG     ((uint32_t)0x00200004)  /*!< ADC1 External Trigger Regular Conversion remapping */
#define GPIO_Remap_ADC2_ETRGINJ     ((uint32_t)0x00200008)  /*!< ADC2 External Trigger Injected Conversion remapping */
#define GPIO_Remap_ADC2_ETRGREG     ((uint32_t)0x00200010)  /*!< ADC2 External Trigger Regular Conversion remapping */
#define GPIO_Remap_ETH              ((uint32_t)0x00200020)  /*!< Ethernet remapping (only for Connectivity line devices) */
#define GPIO_Remap_CAN2             ((uint32_t)0x00200040)  /*!< CAN2 remapping (only for Connectivity line devices) */
#define GPIO_Remap_SWJ_NoJTRST      ((uint32_t)0x00300100)  /*!< Full SWJ Enabled (JTAG-DP + SW-DP) but without JTRST */
#define GPIO_Remap_SWJ_JTAGDisable  ((uint32_t)0x00300200)  /*!< JTAG-DP Disabled and SW-DP Enabled */
#define GPIO_Remap_SWJ_Disable      ((uint32_t)0x00300400)  /*!< Full SWJ Disabled (JTAG-DP + SW-DP) */
#define GPIO_Remap_SPI3             ((uint32_t)0x00201100)  /*!< SPI3/I2S3 Alternate Function mapping (only for Connectivity line devices) */
#define GPIO_Remap_TIM2ITR1_PTP_SOF ((uint32_t)0x00202000)  /*!< Ethernet PTP output or USB OTG SOF (Start of Frame) connected                                                               to TIM2 Internal Trigger 1 for calibration                                                                (only for Connectivity line devices) */
#define GPIO_Remap_PTP_PPS          ((uint32_t)0x00204000)  /*!< Ethernet MAC PPS_PTS output on PB05 (only for Connectivity line devices) */
#define GPIO_Remap_TIM15            ((uint32_t)0x80000001)  /*!< TIM15 Alternate Function mapping (only for Value line devices) */
#define GPIO_Remap_TIM16            ((uint32_t)0x80000002)  /*!< TIM16 Alternate Function mapping (only for Value line devices) */
#define GPIO_Remap_TIM17            ((uint32_t)0x80000004)  /*!< TIM17 Alternate Function mapping (only for Value line devices) */
#define GPIO_Remap_CEC              ((uint32_t)0x80000008)  /*!< CEC Alternate Function mapping (only for Value line devices) */
#define GPIO_Remap_TIM1_DMA         ((uint32_t)0x80000010)  /*!< TIM1 DMA requests mapping (only for Value line devices) */
#define GPIO_Remap_TIM9             ((uint32_t)0x80000020)  /*!< TIM9 Alternate Function mapping (only for XL-density devices) */
#define GPIO_Remap_TIM10            ((uint32_t)0x80000040)  /*!< TIM10 Alternate Function mapping (only for XL-density devices) */
#define GPIO_Remap_TIM11            ((uint32_t)0x80000080)  /*!< TIM11 Alternate Function mapping (only for XL-density devices) */
#define GPIO_Remap_TIM13            ((uint32_t)0x80000100)  /*!< TIM13 Alternate Function mapping (only for High density Value line and XL-density devices) */
#define GPIO_Remap_TIM14            ((uint32_t)0x80000200)  /*!< TIM14 Alternate Function mapping (only for High density Value line and XL-density devices) */
#define GPIO_Remap_FSMC_NADV        ((uint32_t)0x80000400)  /*!< FSMC_NADV Alternate Function mapping (only for High density Value line and XL-density devices) */
#define GPIO_Remap_TIM67_DAC_DMA    ((uint32_t)0x80000800)  /*!< TIM6/TIM7 and DAC DMA requests remapping (only for High density Value line devices) */
#define GPIO_Remap_TIM12            ((uint32_t)0x80001000)  /*!< TIM12 Alternate Function mapping (only for High density Value line devices) */
#define GPIO_Remap_MISC             ((uint32_t)0x80002000)  /*!< Miscellaneous Remap (DMA2 Channel5 Position and DAC Trigger remapping, only for High density Value line devices) */

#endif
/*----------------------------------------------------------*/

#ifdef __cplusplus
namespace _internal {
namespace gpio_f1 {
static const uint16_t EVCR_PORTPINCONFIG_MASK  =  0xFF80;
static const uint16_t LSB_MASK                 =  0xFFFF;
static const uint32_t DBGAFR_POSITION_MASK     =  0x000F0000;
static const uint32_t DBGAFR_SWJCFG_MASK       =  0xF0FFFFFF;
static const uint32_t DBGAFR_LOCATION_MASK     =  0x00200000;
static const uint32_t DBGAFR_NUMBITS_MASK      =  0x00100000;
}}
#else
#define EVCR_PORTPINCONFIG_MASK     ((uint16_t)0xFF80)
#define LSB_MASK                    ((uint16_t)0xFFFF)
#define DBGAFR_POSITION_MASK        ((uint32_t)0x000F0000)
#define DBGAFR_SWJCFG_MASK          ((uint32_t)0xF0FFFFFF)
#define DBGAFR_LOCATION_MASK        ((uint32_t)0x00200000)
#define DBGAFR_NUMBITS_MASK         ((uint32_t)0x00100000)
#endif
/*----------------------------------------------------------*/

/** GPIO bits macros */
enum e_gpio_mode
{
	//! GPIO mode input
	GPIO_MODE_INPUT = 0,
	//! GPIO mode out 10MHZ
	GPIO_MODE_10MHZ = 1,
	//! GPIO mode out 2MHZ
	GPIO_MODE_2MHZ = 2,
	//! GPIO mode out 50MHZ
	GPIO_MODE_50MHZ = 3
};

enum e_gpio_ocnf
{
	//! GPIO mode PUSH-PULL
	GPIO_CNF_GPIO_PP = 0,
	//! GPIO mode Open Drain
	GPIO_CNF_GPIO_OD = 1,
	//! Alternate function PUSH-PULL
	GPIO_CNF_ALT_PP = 2,
	//! Alternate function Open Drain
	GPIO_CNF_ALT_OD = 3
};

enum e_gpio_icnf
{
	//! Configuration IN analog mode
	GPIO_CNF_IN_ANALOG = 0,
	//! Configuration IN float mode
	GPIO_CNF_IN_FLOAT = 1,
	//! Configuration IN pullup mode
	GPIO_CNF_IN_PULLUP = 2
};
/*----------------------------------------------------------*/
//! Set GPIO bit macro
static inline void gpio_set(GPIO_TypeDef* port , unsigned bit)
{
	port->BSRR = 1<<bit;
}
/*----------------------------------------------------------*/
//! Clear GPIO bit macro
static inline void gpio_clr(GPIO_TypeDef* port , unsigned bit)
{
	port->BRR = 1<<bit;
}
/*----------------------------------------------------------*/
//! Set by the mask
static inline void gpio_set_mask(GPIO_TypeDef* port , uint16_t bitmask)
{
	port->BSRR = bitmask;
}
/*----------------------------------------------------------*/
//! Clear GPIO bit mask
static inline void gpio_clr_mask(GPIO_TypeDef* port , uint16_t bitmask)
{
	port->BRR = bitmask;
}
/*----------------------------------------------------------*/
//! set clr in one op
static inline void gpio_set_clr_mask(GPIO_TypeDef* port , uint16_t enflags, uint16_t mask)
{
	port->BSRR = (uint32_t)(enflags & mask) | ((uint32_t)( ~enflags & mask)<<16);
}
/*----------------------------------------------------------*/
//! Get GPIO bit macro
//#define io_get(PORT,BIT) (((PORT)->IDR & (1<<(BIT)))?1:0)
static inline bool gpio_get(GPIO_TypeDef* port , unsigned bit)
{
	return (port->IDR >> (bit))&1;
}
/*----------------------------------------------------------*/
//! Get GPIO bit mask
static inline uint16_t gpio_get_mask(GPIO_TypeDef* port , uint16_t bitmask)
{
	return port->IDR & bitmask;
}
/*----------------------------------------------------------*/
/**
 * 	Configure GPIO line into selected mode
 *	@param[in] port Port to configure
 *	@param[in] bit  Pin bit number
 *	@param[in] mode New port mode
 *	@param[in] config New port configuration
 */
static inline void gpio_config(GPIO_TypeDef* port,uint8_t bit,uint32_t mode,uint32_t config)
{
	if(bit>7)
	{
		port->CRH &= ~(0xf<<(4*(bit-8)));
		port->CRH |= (mode|(config<<2)) << (4*(bit-8));
	}
	else
	{
		port->CRL &= ~(0xf<<(4*bit));
		port->CRL |= (mode|(config<<2)) << (4*bit);
	}
}
/*----------------------------------------------------------*/
enum gpio_port_source
{
	GPIO_PortSourceGPIOA   =    0x00,
	GPIO_PortSourceGPIOB   =    0x01,
	GPIO_PortSourceGPIOC   =    0x02,
	GPIO_PortSourceGPIOD   =    0x03,
	GPIO_PortSourceGPIOE   =    0x04,
	GPIO_PortSourceGPIOF   =    0x05,
	GPIO_PortSourceGPIOG   =    0x06
};
enum gpio_pin_source
{
	GPIO_PinSource0     =       0x00,
	GPIO_PinSource1     =       0x01,
	GPIO_PinSource2     =       0x02,
	GPIO_PinSource3     =       0x03,
	GPIO_PinSource4     =       0x04,
	GPIO_PinSource5     =       0x05,
	GPIO_PinSource6     =       0x06,
	GPIO_PinSource7     =       0x07,
	GPIO_PinSource8     =       0x08,
	GPIO_PinSource9     =       0x09,
	GPIO_PinSource10    =       0x0A,
	GPIO_PinSource11    =       0x0B,
	GPIO_PinSource12    =       0x0C,
	GPIO_PinSource13    =       0x0D,
	GPIO_PinSource14    =       0x0E,
	GPIO_PinSource15    =       0x0F
};
/*----------------------------------------------------------*/
/**
  * @brief  Selects the GPIO pin used as EXTI Line.
  * @param  GPIO_PortSource: selects the GPIO port to be used as source for EXTI lines.
  *   This parameter can be GPIO_PortSourceGPIOx where x can be (A..G).
  * @param  GPIO_PinSource: specifies the EXTI line to be configured.
  *   This parameter can be GPIO_PinSourcex where x can be (0..15).
  * @retval None
  */
static inline void gpio_exti_line_config(uint8_t GPIO_PortSource, uint8_t GPIO_PinSource)
{
  uint32_t tmp = 0x00;
  tmp = ((uint32_t)0x0F) << (0x04 * (GPIO_PinSource & (uint8_t)0x03));
  AFIO->EXTICR[GPIO_PinSource >> 0x02] &= ~tmp;
  AFIO->EXTICR[GPIO_PinSource >> 0x02] |= (((uint32_t)GPIO_PortSource) << (0x04 * (GPIO_PinSource & (uint8_t)0x03)));
}
/*----------------------------------------------------------*/
/**
 * 	Configure GPIO lines into selected mode
 *	@param[in] port Port to configure
 *	@param[in] bitmask Pin bitmap to configure
 *	@param[in] mode New port mode
 *	@param[in] config New port configuration
 */
static inline void gpio_config_ext(GPIO_TypeDef* port, uint16_t bit, uint32_t mode, uint32_t config)
{
	for(unsigned i=0; i<16; i++)
	{
		if(bit & (1<<i))
		{
			gpio_config(port,i,mode,config);
		}
	}
}
/*----------------------------------------------------------*/
#ifdef __cplusplus
namespace _internal {
namespace gpio_f1 {
#endif
/*----------------------------------------------------------*/
//Internal port to number conversion
static inline int _gpio_clock_port_to_number( GPIO_TypeDef* port )
{
	if		( port == GPIOA ) return RCC_APB2Periph_GPIOA;
	else if ( port == GPIOB ) return RCC_APB2Periph_GPIOB;
	else if ( port == GPIOC ) return RCC_APB2Periph_GPIOC;
	else if ( port == GPIOD ) return RCC_APB2Periph_GPIOD;
	else if ( port == GPIOE ) return RCC_APB2Periph_GPIOE;
	else if ( port == GPIOF ) return RCC_APB2Periph_GPIOF;
	else if ( port == GPIOG ) return RCC_APB2Periph_GPIOG;
	else return -1;
}
/*----------------------------------------------------------*/
#ifdef __cplusplus
}}
#endif
/*----------------------------------------------------------*/

/*** Enable or disable CLK for selected port
 * @param[in] port GPIO porrt
 * @param[in] enable Enable disable flag
 */
static inline void gpio_clock_enable( GPIO_TypeDef* port, bool enable )
{
#ifdef __cplusplus
	using namespace stm32;
	using namespace _internal::gpio_f1;
#endif
	if(_gpio_clock_port_to_number( port ) < 0 )
		return;
	if( enable )
		RCC->APB2ENR |=  _gpio_clock_port_to_number( port );
	else
		RCC->APB2ENR &=  ~_gpio_clock_port_to_number( port );
}
/*----------------------------------------------------------*/
/* Remap pin to the selected perhipheral */

static inline void gpio_pin_remap_config( uint32_t GPIO_Remap, bool enable )
{
#ifdef __cplusplus
	using namespace _internal::gpio_f1;
#endif

  uint32_t tmp = 0x00, tmp1 = 0x00, tmpreg = 0x00, tmpmask = 0x00;

  if((GPIO_Remap & 0x80000000) == 0x80000000)
  {
    tmpreg = AFIO->MAPR2;
  }
  else
  {
    tmpreg = AFIO->MAPR;
  }

  tmpmask = (GPIO_Remap & DBGAFR_POSITION_MASK) >> 0x10;
  tmp = GPIO_Remap & LSB_MASK;

  if ((GPIO_Remap & (DBGAFR_LOCATION_MASK | DBGAFR_NUMBITS_MASK)) == (DBGAFR_LOCATION_MASK | DBGAFR_NUMBITS_MASK))
  {
    tmpreg &= DBGAFR_SWJCFG_MASK;
    AFIO->MAPR &= DBGAFR_SWJCFG_MASK;
  }
  else if ((GPIO_Remap & DBGAFR_NUMBITS_MASK) == DBGAFR_NUMBITS_MASK)
  {
    tmp1 = ((uint32_t)0x03) << tmpmask;
    tmpreg &= ~tmp1;
    tmpreg |= ~DBGAFR_SWJCFG_MASK;
  }
  else
  {
    tmpreg &= ~(tmp << ((GPIO_Remap >> 0x15)*0x10));
    tmpreg |= ~DBGAFR_SWJCFG_MASK;
  }

  if ( enable )
  {
    tmpreg |= (tmp << ((GPIO_Remap >> 0x15)*0x10));
  }

  if((GPIO_Remap & 0x80000000) == 0x80000000)
  {
    AFIO->MAPR2 = tmpreg;
  }
  else
  {
    AFIO->MAPR = tmpreg;
  }
}

 
/**
  * @brief  Selects the Ethernet media interface.
  * @note   This function applies only to STM32 Connectivity line devices.
  * @param  GPIO_ETH_MediaInterface: specifies the Media Interface mode.
  *   This parameter can be one of the following values:
  *     @arg GPIO_ETH_MediaInterface_MII: MII mode
  *     @arg GPIO_ETH_MediaInterface_RMII: RMII mode
  * @retval None
  */

#ifdef __cplusplus
#ifdef __cplusplus
namespace _internal {
namespace gpio_f1 {
#endif

static const uint32_t GPIO_ETH_MediaInterface_MII  = 0x00000000;
static const uint32_t GPIO_ETH_MediaInterface_RMII = 0x00000001;
static const uint32_t AFIO_OFFSET                  = AFIO_BASE - PERIPH_BASE;
static const uint32_t MAPR_OFFSET                  = AFIO_OFFSET + 0x04;
static const uint32_t MII_RMII_SEL_BitNumber       = 0x17;
static const uint32_t MAPR_MII_RMII_SEL_BB         = PERIPH_BB_BASE + (MAPR_OFFSET * 32) + (MII_RMII_SEL_BitNumber * 4);

}}
#else
#define GPIO_ETH_MediaInterface_MII    ((u32)0x00000000)
#define GPIO_ETH_MediaInterface_RMII   ((u32)0x00000001)
#define MAPR_MII_RMII_SEL_BB        (PERIPH_BB_BASE + (MAPR_OFFSET * 32) + (MII_RMII_SEL_BitNumber * 4))
#define MAPR_OFFSET                 (AFIO_OFFSET + 0x04)
#define MII_RMII_SEL_BitNumber      ((u8)0x17)
#define AFIO_OFFSET                 (AFIO_BASE - PERIPH_BASE)
#endif


static inline void gpio_eth_media_interface_config(uint32_t media_ifc)
{
#ifdef __cplusplus
	using namespace _internal::gpio_f1;
#endif
  /* Configure MII_RMII selection bit */
  *(__IO uint32_t *) MAPR_MII_RMII_SEL_BB = media_ifc;
}


/*----------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

/*----------------------------------------------------------*/
#ifndef __cplusplus
#undef EVCR_PORTPINCONFIG_MASK
#undef LSB_MASK
#undef DBGAFR_POSITION_MASK
#undef DBGAFR_SWJCFG_MASK
#undef DBGAFR_LOCATION_MASK
#undef DBGAFR_NUMBITS_MASK
#undef MAPR_MII_RMII_SEL_BB
#undef MAPR_OFFSET
#undef MII_RMII_SEL_BitNumber
#undef AFIO_OFFSET
#endif

/*----------------------------------------------------------*/
#endif /* GPIO_F1X_H_ */
/*----------------------------------------------------------*/
