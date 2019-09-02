/**
 * =====================================================================================
 * 	File: input_dev.hpp
 * 	Created Date: Monday, September 2nd 2019, 9:30:43 pm
 * 	Author: Lucjan Bryndza
 * 	Copyright (c) 2019 BoFF
 * 
 * 	GPL v2/3
 * =====================================================================================
 */
#pragma once
#include <gfx/gui/frame.hpp>

namespace gfx::drv {
    
    //! Generic input device for the graphics library
    class input_dev {
    public:
        //Frame constructor
        explicit input_dev(gui::frame& frame)
            : m_frm(frame)
        {}
        //Noncopyable
        input_dev(input_dev&) = delete;
        input_dev& operator=(input_dev&) = delete;
    protected:
        int report_event(const input::event_info& ev) {
            return m_frm.report_event(ev);
        }
    private:
        gui::frame& m_frm;
    };
}

