#pragma once

#include <type_traits>
#include <stm32_ll_gpio.h>
#include <periph/gpio/gpiomodes.hpp>

namespace periph::gpio::stm32::gpio_v2 {
namespace {
	constexpr auto PINS_PER_PORT=16;
	auto num2port(int pin) {
		switch(pin/PINS_PER_PORT) {
#ifdef GPIOA
		case 0: return GPIOA;
#endif
#ifdef GPIOB
		case 1: return GPIOB;
#endif
#ifdef GPIOC
		case 2: return GPIOC;
#endif
#ifdef GPIOD
		case 3: return GPIOD;
#endif
#ifdef GPIOE
		case 4: return GPIOE;
#endif
#ifdef GPIOF
		case 5: return GPIOF;
#endif
#ifdef GPIOG
		case 6: return GPIOG;
#endif
#ifdef GPIOH
		case 7: return GPIOH;
#endif
#ifdef GPIOI
		case 8: return GPIOI;
#endif
#ifdef GPIOJ
		case 9: return GPIOJ;
#endif
#ifdef GPIOK
		case 10: return GPIOK;
#endif
		default: return static_cast<decltype(GPIOA)>(nullptr);
		}
	}
	inline auto num2pin(int pin) {
		return 1U<<(pin%PINS_PER_PORT);
	}
	inline auto tag_pulltype2pullmode(pulltype pu) {
		switch(pu) {
			case pulltype::floating: return LL_GPIO_PULL_NO;
			case pulltype::up: return LL_GPIO_PULL_UP;
			case pulltype::down: return LL_GPIO_PULL_DOWN;
			default: return 0U;
		}
	}
	inline auto tag_outtype2pullmode(outtype ot) {
		switch(ot) {
			case outtype::pushpull: return LL_GPIO_PULL_NO;
			case outtype::opendrain_pu: return LL_GPIO_PULL_UP;
			case outtype::opendrain_pd: return LL_GPIO_PULL_DOWN;
			default: return LL_GPIO_PULL_NO;
		}
	}
	inline auto tag_outtype2outputtype(outtype ot) {
		switch(ot) {
			case outtype::pushpull: return LL_GPIO_OUTPUT_PUSHPULL;
			default: return LL_GPIO_OUTPUT_OPENDRAIN;
		}
	}
	inline auto tag_speed2speed(speed spd) {
		switch(spd) {
			case speed::low: return LL_GPIO_SPEED_FREQ_LOW;
			case speed::medium: return LL_GPIO_SPEED_FREQ_MEDIUM;
			case speed::high: return LL_GPIO_SPEED_FREQ_HIGH;
			default: return LL_GPIO_SPEED_FREQ_HIGH;
		}
	}



	//! Configure GPIO as input
	inline void setup(int pin, const periph::gpio::mode::in& tag) {
		LL_GPIO_SetPinMode(num2port(pin),num2pin(pin),LL_GPIO_MODE_INPUT);
		LL_GPIO_SetPinPull(num2port(pin),num2pin(pin),tag_pulltype2pullmode(tag.pu));
	}

	//! Configure GPIO as analog mode
	inline void setup(int pin, const periph::gpio::mode::an& tag) {
		LL_GPIO_SetPinMode(num2port(pin),num2pin(pin),LL_GPIO_MODE_ANALOG);
		LL_GPIO_SetPinPull(num2port(pin),num2pin(pin),tag_pulltype2pullmode(tag.pu));
	}

	//! Output mode config
	inline void setup(int pin, const periph::gpio::mode::out& tag) {
		LL_GPIO_SetPinMode(num2port(pin),num2pin(pin),LL_GPIO_MODE_OUTPUT);
		LL_GPIO_SetPinSpeed(num2port(pin),num2pin(pin),tag_speed2speed(tag.spd));
		LL_GPIO_SetPinPull(num2port(pin),num2pin(pin),tag_outtype2pullmode(tag.out));
		LL_GPIO_SetPinOutputType(num2port(pin),num2pin(pin),tag_outtype2outputtype(tag.out));
	}
	//! Alternate mode config
	inline void setup(int pin, const periph::gpio::mode::alt& tag) {
		LL_GPIO_SetPinMode(num2port(pin),num2pin(pin),LL_GPIO_MODE_ALTERNATE);
		LL_GPIO_SetPinSpeed(num2port(pin),num2pin(pin),tag_speed2speed(tag.spd));
		LL_GPIO_SetPinPull(num2port(pin),num2pin(pin),tag_outtype2pullmode(tag.out));
		if(pin%PINS_PER_PORT<8)
			LL_GPIO_SetAFPin_0_7(num2port(pin),num2pin(pin),tag.altno);
		else
			LL_GPIO_SetAFPin_8_15(num2port(pin),num2pin(pin),tag.altno);
		LL_GPIO_SetPinOutputType(num2port(pin),num2pin(pin),tag_outtype2outputtype(tag.out));
	}
	
	//! GPIO set
	inline void set(int pin, bool val) {
		if(val)
			LL_GPIO_SetOutputPin(num2port(pin),num2pin(pin));
		else
			LL_GPIO_ResetOutputPin(num2port(pin),num2pin(pin));
	}
	//! GPIO get
	inline bool get(int pin) {
		return LL_GPIO_IsInputPinSet(num2port(pin),num2pin(pin));
	}
	//! Toggle the port
	inline void toggle(int pin) {
		LL_GPIO_TogglePin(num2port(pin),num2pin(pin));
	}
	//!Set all values
	inline void set_mask(int port, unsigned mask) {
		LL_GPIO_SetOutputPin(num2port(port),mask);
	}
	//! Clear values by mask
	inline void clr_mask(int port, unsigned mask) {
		LL_GPIO_ResetOutputPin(num2port(port),mask);
	}
	//! Set clr mask
	inline void set_clr_mask(int port, unsigned set, unsigned clr) {
		WRITE_REG(num2port(port)->BSRR, (set&0xffffU) | (clr&0xffffU)<<16U);
	}
	//! Get mask
	inline unsigned get_mask(int port, unsigned mask) {
		return LL_GPIO_ReadInputPort(num2port(port))&mask;
	}

}}


