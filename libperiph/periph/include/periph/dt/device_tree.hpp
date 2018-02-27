#pragma once

#include <cstdint>

namespace periph {
namespace dt {

	//! Available buses
	enum class bus {
		axi,
		ahb,
		apb1,
		apb2,
		cpu
	};

	//! Pin function type
	enum pinfunc {
		sck,
		miso,
		mosi,
		rxd,
	};

	//! Clock and assigned speed
	struct clock {
		bus busid;
		unsigned speed;
	};

	//! GPIO function assignment to number
	struct pin {
		pinfunc gpiofun;
		unsigned gpio;
	};

	//! Device tree mapper
	struct device {
		const char* name;	//! Device name
		uintptr_t addr;		//! Device  mapped address
		bus devbus;			//! Pin used map
		unsigned mux;		//! Which mux
		const pin* const pins;	//! Pin configuratgion
	};

	//! Global device configuration structure
	struct configuration {
		const clock* const clk;		//! Clocks array
		const device*const devs;		//! Device array
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





