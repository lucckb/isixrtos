/*
 * dts_database.cpp
 *
 *  Created on: 28 lut 2018
 *      Author: lucck
 */

#include <periph/dt/dts.hpp>
#include <periph/dt/dts_config.hpp>
#include <periph/error.hpp>
#include <cstring>

namespace periph {
namespace dt {


namespace {

	const _dts_config::device* find_dev( void* addr )
	{
		for( auto dev=_dts_config::the_machine_config.devs; dev->name; ++dev ) {
			if( reinterpret_cast<uintptr_t>(addr) == dev->addr ) {
				return dev;
			}
		}
		return nullptr;
	}
	const _dts_config::device* find_dev( const char* name )
	{
		for( auto dev=_dts_config::the_machine_config.devs; dev->name; ++dev ) {
			if( !std::strcmp(name,dev->name) ) {
				return dev;
			}
		}
		return nullptr;
	}

	int find_pin( const _dts_config::device* dev, pinfunc pinf )
	{
		for( auto pin=dev->pins; pin->gpiofun!=pinfunc::_empty; ++pin ) {
			if( pinf == pin->gpiofun ) {
				return pin->gpio;
			}
		}
		return error::nopin;
	}
}


/** Get pin by peripheral address
 * @param addr Input device address
 * @param pinf Requested function
 * @return error code @see bus_errors
 */
int get_periph_pin( void *addr, pinfunc pinf )
{
	auto dev = find_dev(addr);
	if( !dev ) {
		return error::nodev;
	} else {
		return find_pin(dev,pinf);
	}
}

/** Get pin by peripheral name
 *
 * @param name Input device peripheral name
 * @param pin Requested function
 * @return error code @see bus_errors
 */
int get_periph_pin( const char* name, pinfunc pinf )
{
	auto dev = find_dev(name);
	if( !dev ) {
		return error::nodev;
	} else {
		return find_pin(dev,pinf);
	}
}


/**
 * Get periph clock by device address
 * @param addr Device address
 * @return error code @see bus_errors
 */
int get_periph_clock( void* addr )
{
	auto dev = find_dev(addr);
	if( !dev ) {
		return error::nodev;
	} else {
		return get_bus_clock(dev->devbus);
	}
}


/**
 * Get periph clock by device name
 * @param addr Device address
 * @return error code @see bus_errors
 */
int get_periph_clock( const char* name )
{
	auto dev = find_dev(name);
	if( !dev ) {
		return error::nodev;
	} else {
		return get_bus_clock(dev->devbus);
	}
}

/**
 * Get bus clock by bus name
 * @param xbuss Current bus
 * @return error code @see bus_errors
 */
int get_bus_clock( bus xbus )
{
	for( auto clk=_dts_config::the_machine_config.clk; clk->busid!=bus::_empty; ++clk ) {
		if( clk->busid == xbus ) {
			return clk->speed;
		}
	}
	return error::noclk;
}

}}
