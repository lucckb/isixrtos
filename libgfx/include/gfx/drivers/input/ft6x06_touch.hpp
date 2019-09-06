/**
 * =====================================================================================
 * 	File: ft6x06_touch.hpp
 * 	Created Date: Monday, September 2nd 2019, 9:50:30 pm
 * 	Author: Lucjan Bryndza
 * 	Copyright (c) 2019 BoFF
 * 
 * 	GPL v2/3
 * =====================================================================================
 */

#pragma once

#include "input_touchpad.hpp"
#include <isix/thread.hpp>

//! FWD decl
namespace periph::drivers {
    class i2c_master;
}
namespace gfx::drv {

    // ft6x06_touchpad driver for graphics library
    class ft6x06_touch final : protected input_touchpad {
        //Two types of touch screen i2c addresses
        static constexpr auto c_ts_addr = 0x54;
        //Two types of touch screen i2c addresses
        static constexpr auto c_ts_addr2 = 0x70;
    public:
        //Constructor
        ft6x06_touch(const char disp_name[], periph::drivers::i2c_master& i2c,gui::frame& frame);
        // Start the main thread
        void start() noexcept;
    private:
        //! Calibrate device
        int calibrate() noexcept;
        // Disable it
        int disable_it() noexcept;
        //! Detect device and use address
        int detect_device() noexcept;
        //! Start probe 
        int touch_enable(unsigned short sizex, unsigned short sizey) noexcept;
        // Configure the touchpad
        int initialize() noexcept;
        // Configure the touchpad
        int uninitialize() noexcept;
        //Main i2c thread
        void thread();
        //Read reg helper function
        int read_reg(int addr, int reg, unsigned char& value);
        //Write register
        int write_reg(int addr, int reg, unsigned char value);
    private:
        periph::drivers::i2c_master& m_i2c;     //I2c controller
        isix::thread m_thr;                     //Thread handler
        unsigned char m_addr {};                //Used hardware address
        unsigned char m_orientation {};         //Touch screen orientation
        const char * m_name;
    };
}

