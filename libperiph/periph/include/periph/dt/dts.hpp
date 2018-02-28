#pragma once

#include <cstdint>

namespace periph {
namespace dt {

	//! Available buses
	enum class bus : short {
		_empty,
		axi,
		ahb,
		apb1,
		apb2,
		cpu
	};

	//! Pin function type
	enum class pinfunc : short {
		_empty,
		sck,
		miso,
		mosi,
		rxd,
	};



	/** Get pin by peripheral address
	 * @param addr Input device address
	 * @param pinf Requested function
	 * @return error code @see bus_errors
	 */
	int get_periph_pin( void *addr, pinfunc pinf );

	/** Get pin by peripheral name
	 *
	 * @param name Input device peripheral name
	 * @param pin Requested function
	 * @return error code @see bus_errors
	 */
	int get_periph_pin( const char* name, pinfunc pinf );


	/**
	 * Get periph clock by device address
	 * @param addr Device address
	 * @return error code @see bus_errors
	 */
	int get_periph_clock( void* addr );


	/**
	 * Get periph clock by device name
	 * @param addr Device address
	 * @return error code @see bus_errors
	 */
	int get_periph_clock( const char* name );

	/**
	 * Get bus clock by bus name
	 * @param xbuss Current bus
	 * @return error code @see bus_errors
	 */
	int get_bus_clock( bus xbus );

}}





