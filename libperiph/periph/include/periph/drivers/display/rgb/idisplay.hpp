/**
 * =====================================================================================
 * 	File: idisplay.hpp
 *  Generic display class interface
 * 	Created Date: Monday, August 19th 2019, 8:51:04 pm
 * 	Author: Lucjan Bryndza
 * 	Copyright (c) 2019 BoFF
 * 
 * 	GPL v2/3
 * =====================================================================================
 */

#pragma once
#include <periph/drivers/display/bus/ibus.hpp>
#include <periph/dt/dts.hpp>

namespace periph::display {


//! Generic display interface class
class idisplay {
public:
    enum class orientation: bool {
        portrait,
        landscape
    };
    enum class format: bool {
        rgb888,
        rgb565
    };
    //! Construtors
    idisplay(bus::ibus& bus, const char name[]) 
        : m_bus(bus), m_addr(dt::get_periph_base_address(name))
    {
    }
    idisplay(idisplay&) = delete;
    //! Destructor
    virtual ~idisplay() {};
    //! Non assignable
    idisplay& operator=(idisplay&) = delete;
    //! Open device
    virtual int open(orientation org, format fmt) noexcept = 0;
    //! Close device
    virtual int close() noexcept = 0;
    //! Setup backlight mode
    virtual void backlight( int percent ) noexcept = 0;
protected:
    //! Write command to the selected addr
    int write(const uint8_t* args, size_t len) noexcept {
        return m_bus.write(m_addr,args,len);
    }
    //! Read command from the selected addr
    int read(const uint8_t* args, size_t len) noexcept {
        return m_bus.read(m_addr,args,len);
    }
    //! Get bus
    auto& bus() const {
        return m_bus;
    }
private:
    bus::ibus& m_bus;
    int m_addr;
};

}