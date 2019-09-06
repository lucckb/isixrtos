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
#include <periph/dt/dts.hpp>
#include <periph/drivers/display/rgb/display_config.hpp>
#include "ft6x06_regs.hpp"


namespace gfx::drv {

//! Swap status
enum ts_swap : uint8_t {
    TS_SWAP_NONE = 1,
    TS_SWAP_X = 2,
    TS_SWAP_Y  = 4,
    TS_SWAP_XY = 8
};

//Constructor
ft6x06_touch::ft6x06_touch(const char name[], periph::drivers::i2c_master& i2c,gui::frame& frame)
    : input_touchpad(frame),m_i2c(i2c),
       m_thr( isix::thread_create( std::bind(&ft6x06_touch::thread,std::ref(*this)))),
       m_name(name)
{}

// Start the main thread
void ft6x06_touch::start() noexcept
{
    m_thr.start_thread(1024,isix::get_min_priority());
}
// Detect device qand assign address
int ft6x06_touch::detect_device() noexcept
{
	using namespace detail::regs::ft6x06;
    int ret {};
	uint8_t value {};
    do {
       ret =  read_reg(c_ts_addr,CHIP_ID_REG,value);
	   if(ret||value!=ID_VALUE) {
		   dbg_info("Invalid id reg bus_err: %i val: %i!=%i", ret,value,ID_VALUE);
		   ret = read_reg(c_ts_addr2,CHIP_ID_REG,value);
		   if(ret||value!=ID_VALUE) {
               dbg_info("Invalid id reg bus_err: %i val: %i!=%i", ret,value,ID_VALUE);
               if(!ret) ret = periph::error::init;
			   break;
		   } else {
               m_addr = c_ts_addr2;
           }
	   } else {
           m_addr = c_ts_addr;
       }
    } while (0);
    return ret;
}

//! Calibrate device
int ft6x06_touch::calibrate() noexcept
{
	using namespace detail::regs::ft6x06;
    uint8_t rd_data;
    uint8_t reg_val;
    int status {};

    /* >> Calibration sequence start */
    do {
        /* Switch FT6206 back to factory mode to calibrate */
        reg_val = (FT6206_DEV_MODE_FACTORY & FT6206_DEV_MODE_MASK) << FT6206_DEV_MODE_SHIFT;
        status = write_reg(m_addr, FT6206_DEV_MODE_REG, reg_val); /* 0x40 */
        if(status) break;

        /* Read back the same register FT6206_DEV_MODE_REG */
        status = read_reg(m_addr, FT6206_DEV_MODE_REG, rd_data);
        if (status) break;
        isix::wait_ms(300); /* Wait 300 ms */

        if (((rd_data & (FT6206_DEV_MODE_MASK << FT6206_DEV_MODE_SHIFT)) >> FT6206_DEV_MODE_SHIFT) != FT6206_DEV_MODE_FACTORY)
        {
            /* Return error to caller */
            status = periph::error::inval;
            break;
        }
        /* Start calibration command */
        status = write_reg(m_addr, FT6206_TD_STAT_REG, 0x04);
        if(status) break;
        isix::wait_ms(300); /* Wait 300 ms */

        /* 100 attempts to wait switch from factory mode (calibration) to working mode */
        bool end_cal {};
        for ( int retries = 0; ((retries < 100) && (!end_cal)); retries++)
        {
            status = read_reg(m_addr, FT6206_DEV_MODE_REG, rd_data);
            if(status) break;
            rd_data = (rd_data & (FT6206_DEV_MODE_MASK << FT6206_DEV_MODE_SHIFT)) >> FT6206_DEV_MODE_SHIFT;
            if (rd_data == FT6206_DEV_MODE_WORKING)
            {
                /* Auto Switch to FT6206_DEV_MODE_WORKING : means calibration have ended */
                end_cal = true; /* exit for loop */
            }
            isix::wait_ms(200); /* Wait 200 ms */
        }
    } while(0);
    /* Calibration sequence end << */
    return status;
}
// Disable it
int ft6x06_touch::disable_it() noexcept
{

	using namespace detail::regs::ft6x06;
    uint8_t reg_val = 0;
    reg_val = (FT6206_G_MODE_INTERRUPT_POLLING & (FT6206_G_MODE_INTERRUPT_MASK >> FT6206_G_MODE_INTERRUPT_SHIFT)) << FT6206_G_MODE_INTERRUPT_SHIFT;
    /* Set interrupt polling mode in FT6206_GMODE_REG */
    return write_reg(m_addr, FT6206_GMODE_REG, reg_val);
}

//Touch screen enable
int ft6x06_touch::touch_enable(unsigned short sizex, unsigned short sizey) noexcept
{
    if(sizex<sizey) {
        m_orientation = TS_SWAP_NONE;
    } else {
        m_orientation = TS_SWAP_XY | TS_SWAP_Y;
    }
    //Start the device
    int ret {};
    do {
        ret = calibrate();
        if(ret) break;
        ret = disable_it();
        if(ret) break;
    } while(0);
    return ret;
}
//Initialize touchpad
int ft6x06_touch::initialize() noexcept
{
    int ret {};
    do {
       ret = detect_device(); 
       if(ret) break;
       const periph::dt::device_conf_base* base {};
       ret = periph::dt::get_periph_devconf(m_name,base);
       if(ret) break;
       const auto entry = reinterpret_cast<const periph::display::fb_info*>(base);
       if(entry->n_layers<1) {
            ret = periph::error::inval;
            break;
       }
       ret = touch_enable(entry->layers[0].width, entry->layers[0].height);
       if(ret) break;
    } while(0);
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
    isix::wait_ms(500);
    int ret;
    if((ret=initialize())) {
        dbg_err("Unable to configure touchpad");
        isix::wait_ms(500);
    }
    dbg_info("Initialize status ret %i", ret);
    for(;;) {
       isix::wait_ms(1000);
    }
    uninitialize();
}

//Read reg helper functi
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

