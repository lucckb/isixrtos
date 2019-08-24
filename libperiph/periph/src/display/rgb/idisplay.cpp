/**
 * =====================================================================================
 * 	File: idisplay.cpp
 * 	Created Date: Saturday, August 24th 2019, 5:09:08 pm
 *  Description: Idisplay base class for configuration
 * 	Author: Lucjan Bryndza
 * 	Copyright (c) 2019 BoFF
 * 
 * 	GPL v2/3
 * =====================================================================================
 */

#include <periph/drivers/display/rgb/idisplay.hpp>
#include <periph/dt/dts.hpp>
#include <periph/drivers/display/rgb/display_config.hpp>

namespace periph::display {


//! Construtor
idisplay::idisplay(bus::ibus& bus, const char name[]) 
    : m_bus(bus), m_addr(dt::get_periph_base_address(name)),m_name(name)
{
    const dt::device_conf_base* base {};
    int ret = dt::get_periph_devconf(name,base);
    error::expose<error::bus_exception>(ret);
    const auto entry = reinterpret_cast<const fb_info*>(base);
    if(entry->n_layers<1) {
        error::expose<error::generic_exception>(error::invstate);
    }
    m_bpp = entry->layers->bpp;
    m_width = entry->layers->width;
    m_height = entry->layers->height;
}


}



