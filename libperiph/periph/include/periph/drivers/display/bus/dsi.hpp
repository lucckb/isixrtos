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

namespace periph::display {
    struct layer_info;
}
namespace periph::display::bus {
    // Class representing DSI device
    class dsi {
    public:
        using vch_t = uint8_t;
        using data_t = uint8_t;
        using datal_t = uint32_t;
        using param_t = uint8_t;
        //Default constructor for dsi
        dsi(const char dsi_name[]);
        // Default destructor for dsi
        ~dsi() {}
        // Noncopyable constructor
        dsi(dsi&) = delete;
        // Noncopyable copy operator
        dsi& operator=(dsi&) = delete;
        /**
         * Open DSI device using default display name
         * @param[in] disp_name Display name
         * @return error code
         */
        int open();
        /** Close already opened DSI device */
        int close();

        /** DSI short command operator */
        int operator()(vch_t vchid, data_t data_type, 
                    param_t data0, param_t data1);
        
        /** DSI long command operator */
        int operator()(vch_t vchid, datal_t data_type, 
                const data_t* param, size_t nparams); 
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