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
#include <foundation/sys/dbglog.h>

/* TODO: Use DMA2D it is really naive implementation
        with software rendering
*/

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
/* Get PIXEL */
color_t dsi_fb::get_pixel(coord_t x, coord_t y)
{
    auto v = *reinterpret_cast<volatile uint32_t*>(
        reinterpret_cast<uintptr_t>(m_fb.fbmem()) + get_width() * y * 3 + x* 3 );
        return v;
}
/* Set PIXEL */
void dsi_fb::set_pixel(coord_t x, coord_t y, color_t color)
{
    const auto p = reinterpret_cast<volatile uint8_t*>(m_fb.fbmem())
            + get_width() * y * 3 + x* 3;
    *p = color_t_B(color);
    *(p+1) = color_t_G(color);
    *(p+2) = color_t_R(color);
}
/* Clear the screen */
void dsi_fb::clear(color_t color)
{
    const auto b = reinterpret_cast<volatile uint8_t*>(m_fb.fbmem());
    for(auto p=0U; p<get_height()*get_width()*3;p+=3) {
        *(p+b) = color_t_B(color);
        *(p+b+1) = color_t_G(color);
        *(p+b+2) = color_t_R(color);
    }
}
/* Blit area */
void dsi_fb::blit(coord_t x, coord_t y, coord_t cx, coord_t cy,
        coord_t src_y, const color_t *buf)
{
    auto p = reinterpret_cast<volatile uint8_t*>(m_fb.fbmem());
    buf +=  src_y * cx;
    for(auto j=0U;j<cy;++j)
    for(auto i=0U;i<cx;++i)
    {
        auto k = p + get_width()*(y+j)*3 + (x+i)*3;
        *(k+0) = color_t_B(*buf);
        *(k+1) = color_t_G(*buf);
        *(k+2) = color_t_R(*buf++);
    }   
}
/* Set blit area (viewport) */
void dsi_fb::ll_blit(coord_t x, coord_t y, coord_t cx, coord_t cy) 
{
    m_saved_x = x;
    m_saved_y = y;
    m_saved_cx = cx;
    m_saved_cy = cy;
}

/* Fill area */
void dsi_fb::fill(coord_t x, coord_t y, coord_t cx, 
            coord_t cy, color_t color)
{
    const auto p = reinterpret_cast<volatile uint8_t*>(m_fb.fbmem());
    for(auto i=0U;i<cx;++i)
    for(auto j=0U;j<cy;++j)
    {
        auto k = p + get_width() * (y+j)*3 + (x+i)*3;
        *(k+0) = color_t_B(color);
        *(k+1) = color_t_G(color);
        *(k+2) = color_t_R(color);
    }
}
/* Vertical scroll */
void dsi_fb::vert_scroll(coord_t x, coord_t y, coord_t cx, 
                    coord_t cy, int lines, color_t bgcolor)
{
    if (lines > 0)
    {
        for (unsigned l = 0, le = (cy - lines); l < le; ++l)
        {
            move_line(x, cx, y + lines + l, y + l);
        }
    }
    else
    {
        lines = -lines;
        for (int l = cy - lines; l >= 0; --l)
        {
            move_line(x, cx, y + l, y + lines + l);
        }
        lines = -lines;
    }
    if (lines > 0)
        dsi_fb::fill(x, y + cy - lines, cx, lines, bgcolor);
    else
        dsi_fb::fill(x, y, cx, -lines, bgcolor);
}

//! Move single line
void dsi_fb::move_line(coord_t x, coord_t cx, coord_t row_from, coord_t row_to)
{
    const auto p = reinterpret_cast<volatile uint8_t*>(m_fb.fbmem());
    for(auto i=0U;i<cx;++i) {
        const auto p_from = p + get_width() * row_from*3 + (x+i)*3;
        auto p_to = p + get_width() * row_to*3 + (x+i)*3;
        *(p_to+0) = *(p_from+0);
        *(p_to+1) = *(p_from+1);
        *(p_to+2) = *(p_from+2);
        m_line_buf = const_cast<uint8_t*>(p_to);
    }
}
/* Power ctl */
bool dsi_fb::power_ctl(power_ctl_t mode)
{
    int ret {};
    do {
        if(mode==power_ctl_t::on) {
            // Open ltdc controller
            ret = m_fb.open();
            if(ret) break;
            //! Fixme this rename as a separate function
            ret = m_ddsp.open();
            if(ret) break;
        } else if(mode==power_ctl_t::off) {
            ret = m_ddsp.close();
            auto ret2 = m_fb.close();
            if(ret2) ret = ret2;
        }
    } while(0);
    return ret;
}
/* Rotate screen */
void dsi_fb::rotate(rotation_t rot)
{
    switch(rot) {
    case rotation_t::rot_0:
        m_ddsp.orientation(periph::display::idisplay::orientation_t::portrait);
        break;
    case rotation_t::rot_180:
        m_ddsp.orientation(periph::display::idisplay::orientation_t::landscape);
        break;
    default:
        dbg_err("Unable to set orientation %i",int(rot));
        break;
    }
}
/* Set backlight percent */
void dsi_fb::backlight(int percent)
{
    m_ddsp.backlight(percent);
}
//! Get backlight
int dsi_fb::backlight()
{
    return error::error_not_supported;
}

}
