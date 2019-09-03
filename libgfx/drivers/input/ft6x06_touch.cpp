/**
 * =====================================================================================
 * 	File: ft6x06_touch.cpp
 * 	Created Date: Monday, September 2nd 2019, 10:05:20 pm
 * 	Author: Lucjan Bryndza
 * 	Copyright (c) 2019 BoFF
 * 
 * 	GPL v2/3
 * =====================================================================================
 */

#include <gfx/drivers/input/ft6x06_touch.hpp>
#include <foundation/sys/dbglog.h>
#include <periph/drivers/i2c/i2c_master.hpp>

namespace gfx::drv {

//Constructor
ft6x06_touch::ft6x06_touch(periph::drivers::i2c_master& i2c,gui::frame& frame)
    : input_touchpad(frame),m_i2c(i2c),
       m_thr( isix::thread_create( std::bind(&ft6x06_touch::thread,std::ref(*this))))
{}

// Start the main thread
void ft6x06_touch::start() noexcept
{
    m_thr.start_thread(1024,isix::get_min_priority());
}
// Configure the touchpad
int ft6x06_touch::initialize() noexcept
{
    int ret {};
    do {
        
    } while (0);
    return ret;
}
// Configure the touchpad
int ft6x06_touch::uninitialize() noexcept
{
    int ret {};
    do {
        
    } while (0);
    return ret;
}

// Main thread for read input events
void ft6x06_touch::thread() {
    if(initialize()) {
        dbg_err("Unable to configure touchpad");
    }
    for(;;) {
       isix::wait_ms(1000);
    }
    uninitialize();
}

//Read reg helper function 
int ft6x06_touch::read_reg(int addr, int reg, unsigned char& value)
{
    unsigned char baddr = reg&0xff;
    periph::blk::trx_transfer tran(&baddr,&value, sizeof baddr, sizeof value);
    return m_i2c.transaction(addr, tran);
}

//Write register
int ft6x06_touch::write_reg(int addr, int reg, unsigned char value)
{
    const unsigned char buf[] = { static_cast<unsigned char>(reg), value };
    periph::blk::tx_transfer tran(buf, sizeof buf);
    return m_i2c.transaction(addr, tran);
}


}

