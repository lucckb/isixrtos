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
        static constexpr auto c_ts_addr = 0x54;
        static constexpr auto c_ts_addr2 = 0x70;
    public:
        //Constructor
        ft6x06_touch(periph::drivers::i2c_master& i2c,gui::frame& frame);
        // Start the main thread
        void start() noexcept;
    private:
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
        periph::drivers::i2c_master& m_i2c;
        isix::thread m_thr;
    };

}

