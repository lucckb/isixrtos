/**
 * =====================================================================================
 * 	File: otm8009a.cpp
 * 	Created Date: Sunday, August 18th 2019, 8:59:26 pm
 * 	Author: Lucjan Bryndza
 * 	Copyright (c) 2019 BoFF
 * 
 * 	GPL v2/3
 * =====================================================================================
 */

#include <periph/drivers/display/rgb/otm8009a.hpp>
#include <periph/drivers/display/bus/dsi.hpp>

namespace periph::display {

//! Constructor
otm8009a::otm8009a(bus::dsi& dsi, int dsi_chn=0)
    : m_dsi(dsi)
{
}

//! Write command to the DSI interface
int otm8009a::write_cmd(const uint8_t* args, size_t len) noexcept
{
    using bus::dsi;
    if (len < 2)
		return m_dsi(m_dsichn, dsi::DCS_SHORT_PKT_WRITE_P1,args[0], args[1]); 
	else
		return m_dsi(m_dsichn, dsi::DCS_LONG_PKT_WRITE, args, len); 
}

}