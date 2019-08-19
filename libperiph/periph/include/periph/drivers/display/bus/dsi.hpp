/**
 * =====================================================================================
 * 	File: dsi.hpp
 *  Dsi serial bus for graphics adapters
 * 	Created Date: Saturday, August 17th 2019, 7:33:07 pm
 * 	Author: Lucjan Bryndza
 * 	Copyright (c) 2019 BoFF
 * 	GPL v2/3
 * =====================================================================================
 */
#pragma once
#include <cstdint>
#include <cstddef>
#include "ibus.hpp"

namespace periph::display {
    struct layer_info;
}
namespace periph::display::bus {
    // Class representing DSI device
    class dsi final : public ibus {
    public:
        using vch_t = uint8_t;
        using data_t = uint8_t;
        using datal_t = uint32_t;
        using param_t = uint8_t;
        //! Command defs
        enum cmds : data_t {
            DCS_SHORT_PKT_WRITE_P0 =	0x05, /* DCS short write, no parameters */
            DCS_SHORT_PKT_WRITE_P1 =    0x15, /* DCS short write, one parameter */
            GEN_SHORT_PKT_WRITE_P0 =	0x03, /* Generic short write, no parameters */
            GEN_SHORT_PKT_WRITE_P1 =    0x13, /* Generic short write, one parameter */
	        GEN_SHORT_PKT_WRITE_P2 =	0x23, /* Generic short write, two parameters */
            DCS_LONG_PKT_WRITE =		0x39, /* DCS long write */
            GEN_LONG_PKT_WRITE =	    0x29, /* Generic long write */
        };
        //Default constructor for dsi
        dsi(const char dsi_name[]);
        // Default destructor for dsi
        virtual ~dsi() {}
        // Noncopyable constructor
        dsi(dsi&) = delete;
        // Noncopyable copy operator
        dsi& operator=(dsi&) = delete;
        /**
         * Open DSI device using default display name
         * @param[in] disp_name Display name
         * @return error code
         */
        int open() noexcept override;
        /** Close already opened DSI device */
        int close() noexcept override;
        //! Write operation
        int write(int addr, const uint8_t* buf, size_t siz) noexcept override;
        /** DSI short command operator */
        int raw_write(vch_t vchid, data_t data_type, 
                    param_t data0, param_t data1) noexcept;
        /** DSI long command operator */
        int raw_write(vch_t vchid, datal_t data_type, 
                const data_t* param, size_t nparams) noexcept;
    private:
        //! GPIO configuration
        int gpio_conf(bool en) noexcept;
        //! Hardware setup
        void hardware_setup() noexcept;
        void iow(uint32_t reg, uint32_t val) const noexcept;
        uint32_t ior(uint32_t reg) const noexcept ;
        auto io() const noexcept {
            return reinterpret_cast<void*>(m_base);
        }
    private:
        uintptr_t m_base; //!DSI base address
        const layer_info* m_fbinfo {};
    };
}