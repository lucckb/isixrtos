#pragma once

#include <cstdint>
#include <periph/dt/types.hpp>

namespace periph {
namespace dt {



	/** Get pin by peripheral name
	 *
	 * @param[in] name Input device peripheral name
	 * @param[in] pin Requested function
	 * @return error code @see bus_errors
	 */
	int get_periph_pin( const char* name, pinfunc pinf );
	int get_periph_pin( void *addr, pinfunc pinf );

	/** Get pin peripheral function
	 * @param[in] pin Requested function
	 * @return error code @see bus_errors
	 */
	int get_periph_pin_mux( const char* name );
    int get_periph_pin_mux( void* addr );


	/**
	 * Get periph clock speed by device name
	 * @param[in] name Input device peripheral name
	 * @return error code @see bus_errors
	 */
	int get_periph_clock( const char* name );
    int get_periph_clock( void* addr );

	/**
	 * Get periph clock information
	 * @param[in] name Input device peripheral name
	 * @param[out] xclk Clock output struct info
	 */
	int get_periph_clock( const char* name, clk_periph& xclk );
	int get_periph_clock( void* addr, clk_periph& xclk );

	/**
	 * Get bus clock by bus name
	 * @param xbuss Current bus
	 * @return error code @see bus_errors
	 */
	int get_bus_clock( bus xbus );

}}





