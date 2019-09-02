/**
 * =====================================================================================
 * 	File: input_touchpad.hpp
 * 	Created Date: Monday, September 2nd 2019, 9:38:24 pm
 * 	Author: Lucjan Bryndza
 * 	Copyright (c) 2019 BoFF
 * 
 * 	GPL v2/3
 * =====================================================================================
 */

#pragma once

#include "input_dev.hpp"
#include <isix/ostime.h>
#include <utility>

namespace gfx::drv {
    
    //! Generic touchpad device for the graphics library
    class input_touchpad : private input_dev {
    public:
        //Frame constructor
        explicit input_touchpad(gui::frame& frame)
            : input_dev(frame)
        {}
    protected:
        //Internal function report touch screen
        int report_touch(input::detail::touch_tag&& touch) {
            input::event_info ev {
                isix::get_jiffies(),
                input::event_info::EV_TOUCH,
                nullptr,
                {}
            };
            ev.touch = std::move(touch);
            return report_event(ev);
        }
    };
}
