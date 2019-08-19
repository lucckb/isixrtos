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
        enum class format: bool {
            rgb888,
            rgb565
        };
        //! Constructors and destructors
        explicit otm8009a(bus::dsi& dsi, int dsi_chn);
        ~otm8009a() {}
        otm8009a(otm8009a&) = delete;
        otm8009a& operator=(otm8009a&) = delete;
        //! Open device
        int open(orientation org, format fmt) noexcept;
        //! Close device
        int close() noexcept;
    private:
        //! Write command to the DSI interface
        int write_cmd(const uint8_t* args, size_t len) noexcept;
        //! Write sequence
        template <typename ...T> int write_seq(T&&... args) noexcept
        {
            uint8_t aargs[sizeof...(args)] = { uint8_t(args)... };
            return write_cmd(aargs,sizeof aargs);
        }
        //! Write command
        template <typename ...T> int write_cmd(uint16_t cmd, T&&... args) noexcept
        {
            int ret {};
            static constexpr auto MCS_ADRSFT_ = 0x0000; /* W Address Shift Function (1) */
            do {
                ret = write_seq(MCS_ADRSFT_, cmd&0xff);
                if(ret) break;
                ret = write_seq(cmd>>8, args...);
            } while(0);
        }
    private:
        //! Private DSI bus for display
        bus::dsi& m_dsi;
        int m_dsichn;
    };
}

