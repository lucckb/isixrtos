/*
 * =====================================================================================
 *
 *       Filename:  clocks_impl.hpp
 *
 *    Description:  STM32 clock implementation
 *
 *        Version:  1.0
 *        Created:  05.03.2018 21:17:54
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once

#include <periph/dt/types.hpp>
#include <periph/core/error.hpp>
#include <stm32_ll_bus.h>

namespace periph {
namespace clock {
namespace stm32 {
namespace clocks_impl {
namespace {

	inline int device_enable( const dt::clk_periph& clk ) {
		switch( clk.xbus ) {
			case periph::dt::bus::ahb1:
				LL_AHB1_GRP1_EnableClock( 1U << clk.bit );
				return error::success;
			case periph::dt::bus::apb1:
				LL_APB1_GRP1_EnableClock( 1U << clk.bit );
				return error::success;
			case periph::dt::bus::apb2:
				LL_APB2_GRP1_EnableClock( 1U << clk.bit );
				return error::success;
#			ifdef RCC_AHB3_SUPPORT
			case periph::dt::bus::ahb3:
				LL_AHB3_GRP1_EnableClock( 1U << clk.bit );
				return error::success;
#			endif
			default:
				return error::nobus;
		}
	}
	inline int device_disable( const dt::clk_periph& clk ) {
		switch( clk.xbus ) {
			case periph::dt::bus::ahb1:
				LL_AHB1_GRP1_DisableClock( 1U << clk.bit );
				return error::success;
			case periph::dt::bus::apb1:
				LL_APB1_GRP1_DisableClock( 1U << clk.bit );
				return error::success;
			case periph::dt::bus::apb2:
				LL_APB2_GRP1_DisableClock( 1U << clk.bit );
				return error::success;
#			ifdef RCC_AHB3_SUPPORT
			case periph::dt::bus::ahb3:
				LL_AHB3_GRP1_DisableClock( 1U << clk.bit );
				return error::success;
#			endif
			default:
				return error::nobus;
		}
	}

	inline int device_is_enabled( const dt::clk_periph& clk ) {
		switch( clk.xbus ) {
			case periph::dt::bus::ahb1:
				return LL_AHB1_GRP1_IsEnabledClock( 1U << clk.bit );
			case periph::dt::bus::apb1:
				return LL_APB1_GRP1_IsEnabledClock( 1U << clk.bit );
			case periph::dt::bus::apb2:
				return LL_APB2_GRP1_IsEnabledClock( 1U << clk.bit );
#			ifdef RCC_AHB3_SUPPORT
			case periph::dt::bus::ahb3:
				return LL_AHB3_GRP1_IsEnabledClock( 1U << clk.bit );
#			endif
			default:
				return error::nobus;
		}
	}

	inline int device_reset( const dt::clk_periph& clk ) {
		switch( clk.xbus ) {
			case periph::dt::bus::ahb1:
				LL_AHB1_GRP1_ForceReset( 1U << clk.bit );
				LL_AHB1_GRP1_ReleaseReset( 1U << clk.bit );
				return error::success;
			case periph::dt::bus::apb1:
				LL_APB1_GRP1_ForceReset( 1U << clk.bit );
				LL_APB1_GRP1_ReleaseReset( 1U << clk.bit );
				return error::success;
			case periph::dt::bus::apb2:
				LL_APB2_GRP1_ForceReset( 1U << clk.bit );
				LL_APB2_GRP1_ReleaseReset( 1U << clk.bit );
				return error::success;
#			ifdef RCC_AHB3_SUPPORT
			case periph::dt::bus::ahb3:
				LL_AHB3_GRP1_ForceReset( 1U << clk.bit );
				LL_AHB3_GRP1_ReleaseReset( 1U << clk.bit );
				return error::success;
#			endif
			default:
				return error::nobus;
		}
	}

}}}

	namespace impl = stm32::clocks_impl;

}}



