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


namespace periph::display {


//! Generic display interface class
class idisplay {
public:
    enum class orientation_t: bool {
        portrait,
        landscape
    };
    idisplay(bus::ibus& bus, const char name[]);
    idisplay(idisplay&) = delete;
    //! Destructor
    virtual ~idisplay() {};
    //! Non assignable
    idisplay& operator=(idisplay&) = delete;
    //! Open device
    virtual int open() noexcept = 0;
    //! Close device
    virtual int close() noexcept = 0;
    //! Set orientation
    virtual int orientation(orientation_t orient) noexcept = 0;
    //! Set backlight mode
    virtual void backlight(int percent) noexcept = 0;
    //! Get display width
    auto width() const noexcept {
        return m_width;
    }
    //! Get display height
    auto height() const noexcept {
        return m_height;
    }
    //! Get BPP disp
    auto bpp() const noexcept {
        return m_bpp;
    }
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
    int m_addr {};
    const char* const m_name;
    short m_width {};
    short m_height {};
    short m_bpp {};
};

}