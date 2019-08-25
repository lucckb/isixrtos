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
#include "idisplay.hpp"

namespace periph::display::bus {
    class dsi;
}

namespace periph::display {

    // Simple driver
    class otm8009a final : public idisplay
    {
    public:
        //! Constructors and destructors
        otm8009a(bus::ibus& dsi, const char name[]);
        virtual ~otm8009a() {}
        //! Open device
        int open() noexcept override;
        //! Close device
        int close() noexcept override;
        //! Setup backlight mode
        void backlight( int percent ) noexcept override;
        //! Set orientation
        int orientation(orientation_t orient) noexcept override;
    private:
        //! Write sequence
        template <typename ...T> int write_seq(T&&... args) noexcept
        {
            uint8_t aargs[sizeof...(args)] = { uint8_t(args)... };
            return write(aargs,sizeof aargs);
        }
        template <typename ...T> int write_cmd(uint16_t cmd, T&&... args) noexcept
        {
            int ret {};
            static constexpr auto MCS_ADRSFT_ = 0x0000; /* W Address Shift Function (1) */
            do {
                ret = write_seq(MCS_ADRSFT_, cmd&0xff);
                if(ret) break;
                ret = write_seq(cmd>>8, args...);
            } while(0);
            return ret;
        }
        // Configure GPIO
        int gpio_conf(bool en) noexcept;
    private:
        const char* const m_dev_name;
        int m_blpin {-1};
    };
}

