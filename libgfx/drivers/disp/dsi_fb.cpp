/**
 * =====================================================================================
 * 	File: dsi_fb.cpp
 *  Description: DSI framebuffer driver
 * 	Created Date: Monday, August 19th 2019, 7:38:10 pm
 * 	Author: Lucjan Bryndza
 * 	Copyright (c) 2019 BoFF
 * 
 * 	GPL v2/3
 * =====================================================================================
 */
#include <gfx/drivers/disp/dsi_fb.hpp>
#include <periph/drivers/display/rgb/fbdev.hpp>
#include <periph/drivers/display/bus/ibus.hpp>
#include <periph/drivers/display/rgb/idisplay.hpp>


namespace gfx::drv {

/** Constructor
*/
//TODO: hardcoded size
dsi_fb::dsi_fb( periph::display::fbdev& fb,
        periph::display::idisplay& disp )
        : disp_base(disp.width(),disp.height())
        ,  m_fb(fb),m_ddsp(disp)
{

}
//! Destructor
dsi_fb::~dsi_fb()
{

}
/* Get PIXEL */
color_t dsi_fb::get_pixel(coord_t x, coord_t y)
{

}
/* Set PIXEL */
void dsi_fb::set_pixel(coord_t x, coord_t y, color_t color)
{

}
/* Clear the screen */
void dsi_fb::clear(color_t color)
{
    auto b = reinterpret_cast<volatile uint8_t*>(m_fb.fbmem());
    for(auto p=0U; p<get_height()*get_width()*m_ddsp.bpp()/8;p+=m_ddsp.bpp()/8) {
        *(p+b) = color_t_B(color);
        *(p+b+1) = color_t_G(color);
        *(p+b+2) = color_t_R(color);
    }
}
/* Blit area */
void dsi_fb::blit(coord_t x, coord_t y, coord_t cx, coord_t cy,
        coord_t src_y, const color_t *buf)
{

}
/* Set blit area (viewport) */
void dsi_fb::ll_blit(coord_t x, coord_t y, coord_t cx, coord_t cy) 
{
}
/* Push into the memory */
void dsi_fb::ll_blit(const color_t *buf, size_t len)
{

}
/* Fill area */
void dsi_fb::fill(coord_t x, coord_t y, coord_t cx, 
            coord_t cy, color_t color)
{

}
/* Vertical scroll */
void dsi_fb::vert_scroll(coord_t x, coord_t y, coord_t cx, 
                    coord_t cy, int lines, color_t bgcolor)
{

}
/* Power ctl */
bool dsi_fb::power_ctl(power_ctl_t mode)
{
    int ret {};
    using cfg = periph::display::idisplay;
    do {
        if(mode==power_ctl_t::on) {
            // Open ltdc controller
            ret = m_fb.open();
            if(ret) break;
            //! Fixme this rename as a separate function
            ret = m_ddsp.open(cfg::orientation::portrait);
            if(ret) break;
        }
    } while(0);
    return ret;
}
/* Rotate screen */
void dsi_fb::rotate(rotation_t rot)
{

}
/* Set backlight percent */
void dsi_fb::backlight(int percent)
{

}
//! Get backlight
int dsi_fb::backlight()
{

}
//! Get render buffer
std::pair<color_t*,size_t> dsi_fb::get_rbuf()
{
}

}
