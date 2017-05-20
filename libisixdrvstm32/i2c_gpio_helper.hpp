/*
 * =====================================================================================
 *
 *       Filename:  i2c_gpio_helper.hpp
 *
 *    Description:  I2c GPIO helper
 *
 *        Version:  1.0
 *        Created:  20.05.2017 18:39:03
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once
#include <stm32gpio.h>

namespace stm32 {
namespace drv {
namespace {
	static const auto I2C1_PORT = GPIOB;
	static constexpr auto I2C1_SDA_PIN_ = 7;
	static constexpr auto I2C1_SCL_PIN_ = 6;
	static constexpr auto I2C1_SDA_PIN = 1U<<I2C1_SDA_PIN_;
	static constexpr auto I2C1_SCL_PIN = 1U<<I2C1_SCL_PIN_;

	static constexpr auto I2C1ALT_SDA_PIN_ = 9;
	static constexpr auto I2C1ALT_SCL_PIN_ = 8;
	static constexpr auto I2C1ALT_SDA_PIN = 1U<<I2C1ALT_SDA_PIN_;
	static constexpr auto I2C1ALT_SCL_PIN = 1U<<I2C1ALT_SCL_PIN_;


	static const auto I2C2_PORT = GPIOB;
	static constexpr auto I2C2_SDA_PIN_ = 11;
	static constexpr auto I2C2_SCL_PIN_ = 10;
	static constexpr auto I2C2_SDA_PIN = 1U<<I2C2_SDA_PIN_;
	static constexpr auto I2C2_SCL_PIN = 1U<<I2C2_SCL_PIN_;


	static const auto I2C2ALT_PORT = GPIOF;
	static constexpr auto I2C2ALT_SDA_PIN_ = 0;
	static constexpr auto I2C2ALT_SCL_PIN_ = 1;
	static constexpr auto I2C2ALT_SDA_PIN = 1U<<I2C2ALT_SDA_PIN_;
	static constexpr auto I2C2ALT_SCL_PIN = 1U<<I2C2ALT_SCL_PIN_;


	static constexpr auto I2C1_PINS = I2C1_SDA_PIN|I2C1_SCL_PIN;
	static constexpr auto I2C1ALT_PINS = I2C1ALT_SDA_PIN|I2C1ALT_SCL_PIN;
	static constexpr auto I2C2_PINS = I2C2_SDA_PIN|I2C2_SCL_PIN;
	static constexpr auto I2C2ALT_PINS = I2C2ALT_SDA_PIN|I2C2ALT_SCL_PIN;

#ifdef STM32MCU_MAJOR_TYPE_F1
	//AFIO optional config
	inline void afio_config( I2C_TypeDef * const, bool  ) {}
#else
	//AFIO optional config
	inline void afio_config( I2C_TypeDef * const i2c, bool alt )
	{

		if( i2c == I2C1 ) {
			if( !alt ) {
				gpio_pin_AF_config( I2C1_PORT, I2C1_SDA_PIN_, GPIO_AF_I2C1 );
				gpio_pin_AF_config( I2C1_PORT, I2C1_SCL_PIN_, GPIO_AF_I2C1 );
			}
			else {
				gpio_pin_AF_config( I2C1_PORT, I2C1ALT_SDA_PIN_, GPIO_AF_I2C1 );
				gpio_pin_AF_config( I2C1_PORT, I2C1ALT_SCL_PIN_, GPIO_AF_I2C1 );
			}
		} else if( i2c == I2C2 ) {
			if( !alt ) {
				gpio_pin_AF_config( I2C2_PORT, I2C2_SDA_PIN_, GPIO_AF_I2C2 );
				gpio_pin_AF_config( I2C2_PORT, I2C2_SCL_PIN_, GPIO_AF_I2C2 );
			} else {
				gpio_pin_AF_config( I2C2ALT_PORT, I2C2ALT_SDA_PIN_, GPIO_AF_I2C2 );
				gpio_pin_AF_config( I2C2ALT_PORT, I2C2ALT_SCL_PIN_, GPIO_AF_I2C2 );
			}

		}
	}
#endif


}}}

