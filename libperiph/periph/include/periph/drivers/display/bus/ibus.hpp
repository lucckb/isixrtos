/**
 * =====================================================================================
 * 	File: ibus.hpp
 * 	Created Date: Monday, August 19th 2019, 8:57:51 pm
 * 	Author: Lucjan Bryndza
 * 	Copyright (c) 2019 BoFF
 * 
 * 	GPL v2/3
 * =====================================================================================
 */

#pragma once

#include <cstdint>
#include <cstddef>
#include <periph/core/error.hpp>

namespace periph::display::bus {

    //! Generic internal bus interface
    class ibus {
    public:
        enum class type : char {
            dsi,    //DSI bus
        };
        //Construtors
        explicit ibus(type _type) 
            : m_type(_type)
        {}
        ibus(ibus&) = delete;
        //Destructor
        virtual ~ibus() {}
        //Copy operator
        ibus& operator=(ibus&)=delete;
        //!Get bus type
        auto instance_of() const noexcept {
            return m_type;
        }
        virtual int open() noexcept;
        virtual int close() noexcept;
        //! Write operation
        virtual int write(int addr, const uint8_t* buf, size_t siz) noexcept = 0;
        //! Read operation
        virtual int read(int /*addr*/, const uint8_t* /*buf*/, size_t /*siz*/) noexcept {
            return error::not_supported;
        }
    private:
        const type m_type;
    };
}

