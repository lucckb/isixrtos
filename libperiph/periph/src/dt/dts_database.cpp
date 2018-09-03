/*
 * dts_database.cpp
 *
 *  Created on: 28 lut 2018
 *      Author: lucck
 */

#include <periph/dt/dts.hpp>
#include <periph/dt/dts_config.hpp>
#include <periph/core/error.hpp>
#include <cstring>

namespace periph::dt {


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
	void fill_clk_periph( const _dts_config::device* dev, clk_periph& xclk ) {
		xclk.xbus = dev->devbus;
		xclk.bit =  dev->clken;
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
	* Get periph clock information
	* @param[in] name Input device peripheral name
	* @param[out] xclk Clock output struct info
	*/
int get_periph_clock( const char* name, clk_periph& xclk )
{
	auto dev = find_dev( name );
	if( !dev ) {
		return error::nodev;
	} else {
		fill_clk_periph( dev, xclk );
		return error::success;
	}
}

int get_periph_clock( void* addr, clk_periph& xclk )
{
	auto dev = find_dev(addr);
	if(!dev) {
		return error::nodev;
	} else {
		fill_clk_periph(dev, xclk);
		return error::success;
	}
}




/** Get pin peripheral function
	* @param[in] pin Requested function
	* @return error code @see bus_errors
	*/
int get_periph_pin_mux( const char* name )
{
	auto dev = find_dev( name );
	if( !dev ) {
		return error::nodev;
	} else {
		return dev->mux;
	}
}

int get_periph_pin_mux( void* addr )
{
	auto dev = find_dev( addr );
	if( !dev ) {
		return error::nodev;
	} else {
		return dev->mux;
	}
}



//! Get periph addr
uintptr_t get_periph_base_address( const char* name )
{
	auto dev = find_dev( name );
	if( !dev ) {
		return 0;
	} else {
		return dev->addr;
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
			return clk->speedfn();
		}
	}
	return error::noclk;
}


/** Get device configuration
	* @param[in] name Device name
	* @param[in] conf Device configuration structure
	*/
template <typename T, typename K>
int get_periph_devconf_impl(T ident, K& conf)
{
	auto dev = find_dev(ident);
	if(!dev) {
		return error::nodev;
	}
	if(!dev->conf) {
		return error::noent;
	}
	conf = *static_cast<const K*>(dev->conf);
	return error::success;
}

int get_periph_devconf(const char* name, device_conf& conf)
{
	return get_periph_devconf_impl(name,conf);
}

int get_periph_devconf(void* addr, device_conf& conf)
{
	return get_periph_devconf_impl(addr,conf);
}

int get_periph_devconf(const char* name, const device_conf_base*& conf)
{
	auto dev = find_dev(name);
	if(!dev) {
		return error::nodev;
	}
	if(!dev->conf) {
		return error::noent;
	}
	conf = dev->conf;
	return error::success;
}

}
