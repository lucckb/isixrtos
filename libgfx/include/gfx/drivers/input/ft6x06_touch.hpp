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

//! FWD decl
namespace periph::drivers {
    class i2c_master;
}
namespace gfx::drv {

    // ft6x06_touchpad driver for graphics library
    class ft6x06_touch final : protected input_touchpad {
    public:
        //Constructor
        ft6x06_touch(periph::drivers::i2c_master& i2c,gui::frame& frame)
            : input_touchpad(frame),m_i2c(i2c)
        {}
    private:
        periph::drivers::i2c_master& m_i2c;
    };

}

