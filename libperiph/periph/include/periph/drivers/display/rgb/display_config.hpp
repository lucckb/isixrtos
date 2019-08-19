/**
 * =====================================================================================
 * 	File: display_config.hpp
 * 	Created Date: Saturday, August 17th 2019, 7:39:43 pm
 * 	Author: Lucjan Bryndza
 * 	Copyright (c) 2019 BoFF
 * 
 * 	GPL v2/3
 * =====================================================================================
 */

#pragma once

#include <periph/dt/types.hpp>
#include <cstdint>
#include <cstddef>


namespace periph::display {
    // Detailed layer info for RGB display
    struct layer_info {
        uint32_t width;
        uint32_t height;
        uint32_t hsync;
        uint32_t vsync;
        uint32_t vfp;
        uint32_t hfp;
        uint32_t vbp;
        uint32_t hbp;
        uint32_t bpp;
        uint32_t base;
    };
    //Configuration for framebuffer device controller
    struct fb_info : public dt::device_conf_base {
        const layer_info* layers;
        size_t n_layers;
    }; 
    

};

