/**
 * =====================================================================================
 * 	File: dsi_fb.hpp
 * 	Created Date: Monday, August 19th 2019, 7:08:27 pm
 * 	Author: Lucjan Bryndza
 * 	Copyright (c) 2019 BoFF
 * 
 * 	GPL v2/3
 * =====================================================================================
 */

#pragma once
#include <utility>
#include "disp_base.hpp"

namespace periph::display {
    namespace bus {
        class ibus;
    }
    class fbdev;
    class idisplay;
}

namespace gfx::drv {
    class dsi_fb final : public disp_base {
    public:
        /** Constructor
        */
        dsi_fb( periph::display::fbdev& fb,
                periph::display::idisplay& disp );
        //! Destructor
        virtual ~dsi_fb() {}
        /* Get PIXEL */
        color_t get_pixel(coord_t x, coord_t y) override;
        /* Set PIXEL */
        void set_pixel(coord_t x, coord_t y, color_t color) override;
        /* Clear the screen */
        void clear( color_t color ) override;
        /* Blit area */
        void blit(coord_t x, coord_t y, coord_t cx, coord_t cy,
                coord_t src_y, const color_t* buf) override;
        /* Set blit area (viewport) */
        void ll_blit(coord_t x, coord_t y, coord_t cx, coord_t cy) override;
        /* Push into the memory */
        void ll_blit(const color_t *buf, size_t) override {
            blit(m_saved_x,m_saved_y,m_saved_cx,m_saved_cy,0,buf);
        }
        /* Fill area */
        void fill(coord_t x, coord_t y, coord_t cx, 
                    coord_t cy, color_t color) override;
        /* Vertical scroll */
        void vert_scroll(coord_t x, coord_t y, coord_t cx, 
                          coord_t cy, int lines, color_t bgcolor) override;
        /* Power ctl */
        bool power_ctl(power_ctl_t mode) override;
        /* Rotate screen */
        virtual void rotate(rotation_t rot) override;
        /* Set backlight percent */
        void backlight(int percent) override;
        //! Get backlight
        int backlight() override;
        std::pair<color_t*,size_t> get_rbuf() override {
            return std::make_pair<color_t*,size_t>( reinterpret_cast<color_t*>(0xC0800000), 8*1024*1024 );
        }
    private:
        //! Move single line
        void move_line(coord_t x, coord_t cx, coord_t row_from, coord_t row_to);
    private:
        //! FBdev class 
        periph::display::fbdev& m_fb;
        //! Framebuffer display class
        periph::display::idisplay& m_ddsp;
        // Store params for partial blit
        coord_t m_saved_x, m_saved_y, m_saved_cx, m_saved_cy {};
        uint8_t* m_line_buf {};
    };
}