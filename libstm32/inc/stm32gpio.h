/*
 * stm32gpio.h
 *
 *  Created on: 17-07-2012
 *      Author: lucck
 */
#ifndef STM32GPIO_H_
#define STM32GPIO_H_
#if defined(STM32MCU_MAJOR_TYPE_F1)
#include "gpio_v1.h"
#else
#include "gpio_v2.h"
#define _CONFIG_STM32_GPIO_V2_
#endif



#ifdef __cplusplus
namespace stm32 {
#endif
/** Set the config mode common for F1 F2 F4 devices */
enum e_abstract_gpio_config
{
	AGPIO_MODE_INPUT_PULLUP,
	AGPIO_MODE_INPUT_PULLDOWN,
	AGPIO_MODE_INPUT_FLOATING,
	AGPIO_MODE_OUTPUT_PP,
	AGPIO_MODE_OUTPUT_OD_PULLUP,
	AGPIO_MODE_OUTPUT_OD_PULLDOWN,
	AGPIO_MODE_ALTERNATE_PP,
	AGPIO_MODE_ALTERNATE_OD_PULLUP,
	AGPIO_MODE_ALTERNATE_OD_PULLDOWN,
	AGPIO_MODE_ANALOG
};
/* GPIO pin to bit */
static inline unsigned short gpioPIN(int no)
{
	return 1<<no;
}
/** Set the output speed for F1 F2 F4 devices */
enum e_abstract_gpio_speed
{
	AGPIO_SPEED_VLOW,		/** Very low gpio speed */
	AGPIO_SPEED_LOW,		/** low gpio speed */
	AGPIO_SPEED_HALF,		/** half gpio speed */
	AGPIO_SPEED_FULL		/** full gpio speed */
};
/** GPIO abstract result */
enum e_abstract_gpio_result
{
	EA_GPIO_RESULT_SUCCESS = 0,
	EA_GPIO_RESULT_FAILURE = -1
};
#ifdef __cplusplus
namespace _internal {
namespace stm32 {
#endif
/** Internal function speed to value conversion **/
static inline int _gpio_speed_to_value(enum e_abstract_gpio_speed vspeed )
{
#if defined(STM32MCU_MAJOR_TYPE_F1)
	switch( vspeed )
	{
	case AGPIO_SPEED_VLOW:  return GPIO_MODE_2MHZ;
	case AGPIO_SPEED_LOW:   return GPIO_MODE_10MHZ;
	case AGPIO_SPEED_HALF:  return GPIO_MODE_50MHZ;
	case AGPIO_SPEED_FULL:  return GPIO_MODE_50MHZ;
	}
	return GPIO_MODE_2MHZ;
#else
	switch( vspeed )
	{
	case AGPIO_SPEED_VLOW:  return GPIO_SPEED_LOW;
	case AGPIO_SPEED_LOW:   return GPIO_SPEED_MED;
	case AGPIO_SPEED_HALF:  return GPIO_SPEED_FAST;
	case AGPIO_SPEED_FULL:  return GPIO_SPEED_HI;
	}
	return AGPIO_SPEED_VLOW;
#endif
}
#ifdef __cplusplus
}}
#endif
/**  Configure selected GPIO using abstract mode
 * @param[in] Port GPIO port
 * @param[in] bit Number of gpio bit
 * @param[in] conf Current port configuration
 * @param[in] speed Port speed in output mode
 * @result success or failure */
static inline int gpio_abstract_config(GPIO_TypeDef* port, uint8_t bit, enum e_abstract_gpio_config conf,
		enum e_abstract_gpio_speed speed  )
{
#ifdef __cplusplus
	using namespace _internal::stm32;
#endif
#if defined(STM32MCU_MAJOR_TYPE_F1)
	switch( conf )
	{
	case AGPIO_MODE_INPUT_PULLUP:
		gpio_config( port, bit, GPIO_MODE_INPUT, GPIO_CNF_IN_PULLUP );
		gpio_set( port, bit );
		break;
	case AGPIO_MODE_INPUT_PULLDOWN:
		gpio_config( port, bit, GPIO_MODE_INPUT, GPIO_CNF_IN_PULLUP );
		gpio_set( port, bit );
		break;
	case AGPIO_MODE_INPUT_FLOATING:
		gpio_config( port, bit, GPIO_MODE_INPUT, GPIO_CNF_IN_FLOAT );
		break;
	case AGPIO_MODE_OUTPUT_PP:
		gpio_config( port, bit, _gpio_speed_to_value( speed ), GPIO_CNF_GPIO_PP );
		break;
	case AGPIO_MODE_OUTPUT_OD_PULLUP:
		gpio_config( port, bit, _gpio_speed_to_value( speed ), GPIO_CNF_GPIO_OD );
		break;
	case AGPIO_MODE_OUTPUT_OD_PULLDOWN:
	case AGPIO_MODE_ALTERNATE_OD_PULLDOWN:
		return EA_GPIO_RESULT_FAILURE;
	case AGPIO_MODE_ALTERNATE_PP:
		gpio_config( port, bit, _gpio_speed_to_value( speed ), GPIO_CNF_ALT_PP );
		break;
	case AGPIO_MODE_ALTERNATE_OD_PULLUP:
		gpio_config( port, bit, _gpio_speed_to_value( speed ), GPIO_CNF_ALT_OD );
		break;
	case AGPIO_MODE_ANALOG:
		gpio_config( port, bit, GPIO_MODE_INPUT, GPIO_CNF_IN_ANALOG );
		break;
	}
#else
	switch( conf )
	{
	case AGPIO_MODE_INPUT_PULLUP:
		gpio_config( port, bit, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, 0, 0 );
		break;
	case AGPIO_MODE_INPUT_PULLDOWN:
		gpio_config( port, bit, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, 0, 0 );
		break;
	case AGPIO_MODE_INPUT_FLOATING:
		gpio_config( port, bit, GPIO_MODE_INPUT, GPIO_PUPD_NONE, 0, 0 );
		break;
	case AGPIO_MODE_OUTPUT_PP:
		gpio_config( port, bit, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, _gpio_speed_to_value( speed ), GPIO_OTYPE_PP );
		break;
	case AGPIO_MODE_OUTPUT_OD_PULLUP:
		gpio_config( port, bit, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, _gpio_speed_to_value( speed ), GPIO_OTYPE_OD );
		break;
	case AGPIO_MODE_OUTPUT_OD_PULLDOWN:
		gpio_config( port, bit, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, _gpio_speed_to_value( speed ), GPIO_OTYPE_OD );
		break;
	case AGPIO_MODE_ALTERNATE_PP:
		gpio_config( port, bit, GPIO_MODE_ALTERNATE, GPIO_PUPD_NONE, _gpio_speed_to_value( speed ), GPIO_OTYPE_PP );
		break;
	case AGPIO_MODE_ALTERNATE_OD_PULLUP:
		gpio_config( port, bit, GPIO_MODE_ALTERNATE, GPIO_PUPD_PULLUP, _gpio_speed_to_value( speed ), GPIO_OTYPE_OD );
		break;
	case AGPIO_MODE_ALTERNATE_OD_PULLDOWN:
		gpio_config( port, bit, GPIO_MODE_ALTERNATE, GPIO_PUPD_PULLDOWN, _gpio_speed_to_value( speed ), GPIO_OTYPE_OD );
		break;
	case AGPIO_MODE_ANALOG:
		gpio_config( port, bit, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, 0, 0 );
		break;
	}
#endif
	return EA_GPIO_RESULT_SUCCESS;
}
/**  Configure selected GPIO using abstract mode
 * @param[in] Port GPIO port
 * @param[in] bit Bitmask
 * @param[in] conf Current port configuration
 * @param[in] speed Port speed in output mode
 * @result success or failure */
static inline int gpio_abstract_config_ext(GPIO_TypeDef* port, uint16_t bit, enum e_abstract_gpio_config conf,
		enum e_abstract_gpio_speed speed  )
{
	for(unsigned i=0; i<16; i++)
	{
		if(bit & (1<<i))
		{
			if( gpio_abstract_config( port, i, conf, speed ) == EA_GPIO_RESULT_FAILURE )
				return EA_GPIO_RESULT_FAILURE;
		}
	}
	return EA_GPIO_RESULT_SUCCESS;
}

#ifdef __cplusplus
}
#endif


#ifdef __cplusplus
#include "stm32gpio_for_cpp.h"
#endif


#endif /* STM32GPIO_H_ */
