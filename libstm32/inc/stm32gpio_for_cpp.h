/*
 * =====================================================================================
 *
 *       Filename:  stm32gpio_for_cpp.h
 *
 *    Description:  STM32 C++ abstract GPIO
 *
 *        Version:  1.0
 *        Created:  17.12.2017 20:24:53
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once

namespace stm32 {
namespace gpio {

	using pin_id_t = unsigned char;

	//! Pin description claas
	class pin_desc
	{
		constexpr unsigned p2i( GPIO_TypeDef* const port ) {
#ifdef GPIOA
			if( port==GPIOA ) return PA;
#endif
#ifdef GPIOB
			else if( port==GPIOB ) return PB;
#endif
#ifdef GPIOC
			else if( port==GPIOC ) return PC;
#endif
#ifdef GPIOD
			else if( port==GPIOD ) return PD;
#endif
#ifdef GPIOE
			else if( port==GPIOE ) return PE;
#endif
#ifdef GPIOF
			else if( port==GPIOE ) return PF;
#endif
#ifdef GPIOG
			else if( port==GPIOG ) return PG;
#endif
#ifdef GPIOH
			else if( port==GPIOH ) return PH;
#endif
#ifdef GPIOI
			else if( port==GPIOI ) return PI;
#endif
			else return 0;
		}
		static constexpr auto PORT_SHIFT = 4;
		static constexpr pin_id_t PORT_MASK = 0xF0U;
		static constexpr pin_id_t PIN_MASK = 0x0FU;
	public:
		//! Constructor
		enum portno : unsigned { INVPORT, PA, PB, PC, PD, PE, PF, PG, PH, PI };
		pin_desc(GPIO_TypeDef* port, unsigned pin)
			: m_pins( (pin&PIN_MASK) | p2i(port)<<PORT_SHIFT)
		{}
		constexpr pin_desc( portno port, unsigned pin)
			: m_pins( (pin&PIN_MASK) | port<<PORT_SHIFT)
		{}
		//! Default constructor invalid state
		constexpr pin_desc() : m_pins(INVPORT) {}
		//Public members
		GPIO_TypeDef* port() const noexcept {
			switch( m_pins >> PORT_SHIFT )
			{
#ifdef GPIOA
			case PA: return GPIOA;
#endif
#ifdef GPIOB
			case PB: return GPIOB;
#endif
#ifdef GPIOC
			case PC: return GPIOC;
#endif
#ifdef GPIOD
			case PD: return GPIOD;
#endif
#ifdef GPIOE
			case PE: return GPIOE;
#endif
#ifdef GPIOF
			case PF: return GPIOF;
#endif
#ifdef GPIOG
			case PG: return GPIOG;
#endif
#ifdef GPIOH
			case PH: return GPIOH;
#endif
#ifdef GPIOI
			case PI: return GPIOI;
#endif
			default: return nullptr;
			}
		}
		//! Get pin ord number
		auto ord() const noexcept {
			return m_pins & PIN_MASK;
		}
		//! Get pin bit
		auto bv() const noexcept {
			return 1U<<ord();
		}
		//! Return packed value
		auto packed() const noexcept {
			return m_pins;
		}
		operator bool() const noexcept {
			return m_pins!=INVPORT;
		}
	private:
		pin_id_t m_pins {};
	};
}};

