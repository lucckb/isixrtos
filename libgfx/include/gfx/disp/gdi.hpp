/* ------------------------------------------------------------------ */
/*
 * gdi.hpp
 *
 *  Created on: 14-06-2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#ifndef GFX_GDI_HPP_
#define GFX_GDI_HPP_
/* ------------------------------------------------------------------ */
#include <gfx/drivers/disp/disp_base.hpp>
#include <gfx/disp/bitmap_fonts.hpp>
/* ------------------------------------------------------------------ */
namespace gfx {
namespace disp {
/* ------------------------------------------------------------------ */
class gdi
{
	//Make object noncopyable
	gdi(const gdi&) = delete;
	gdi& operator=(const gdi&) = delete;
public:
	//Constructor
	gdi( drv::disp_base &gdev ):
		m_gdev( gdev )
	{
	}
	//Put char
	int draw_char(coord_t x, coord_t y, int ch, const font_t &font, color_t color, color_t bg_color );
private:
	drv::disp_base &m_gdev;
};
/* ------------------------------------------------------------------ */
}}
/* ------------------------------------------------------------------ */
#endif /* GDI_HPP_ */
/* ------------------------------------------------------------------ */
