/**
 * =====================================================================================
 * 	File: otm8009a.hpp
 *  Description: OTM8009A base init driver
 * 	Created Date: Sunday, August 18th 2019, 9:00:41 pm
 * 	Author: Lucjan Bryndza
 * 	Copyright (c) 2019 BoFF
 * 
 * 	GPL v2/3
 * =====================================================================================
 */

#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <utility>

namespace periph::display::bus {
    class dsi;
}

namespace periph::display {

    // Simple driver
    class otm8009a
    {
    public:
        enum class orientation: bool {
            portrait,
            landscape
        };
        //! Constructors and destructors
        explicit otm8009a(bus::dsi& dsi, int dsi_chn);
        ~otm8009a() {}
        otm8009a(otm8009a&) = delete;
        otm8009a& operator=(otm8009a&) = delete;
        //! Open device
        int open(orientation org) noexcept;
        //! Close device
        int close() noexcept;
    private:
        //! Write command to the DSI interface
        int write_cmd(const uint8_t* args, size_t len) noexcept;
    private:
        //! Private DSI bus for display
        bus::dsi& m_dsi;
        int m_dsichn;
    };
}

