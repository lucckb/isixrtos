/*
 * =====================================================================================
 *
 *       Filename:  stm32_i2c_v1.cpp
 *
 *    Description: 
 *
 *        Version:  1.0
 *        Created:  21.06.2019 17:41:03
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#include <periph/drivers/i2c/i2c_master.hpp>
#include <periph/dt/dts.hpp>

namespace periph::drivers {

//! Constructor
i2c_master::i2c_master(const char name[])
	: block_device(block_device::type::i2c, dt::get_periph_base_address(name))
{
}

//! Destructor
i2c_master::~i2c_master()
{
}

//! Make transaction
int i2c_master::transaction(int addr, const blk::transfer& data)
{
}

//! DO open
int i2c_master::do_open(int timeout)
{
}

//! Do close
int i2c_master::do_close()
{
}

// Set option
int i2c_master::do_set_option(const option::device_option& opt)
{
}

}



