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
	//!Constructor
	gdi( drv::disp_base &gdev ):
		m_gdev( gdev )
	{
	}

	/** Clear the screen using background color */
	void clear()
	{
		m_gdev.clear( m_bg_color );
	}
	/** Draw text using current context and font
	 * @param[in] x position on display
	 * @param[in] y position on display
	 * @param[ch] ch char to display
	 */
	void draw_text( coord_t x, coord_t y, int ch );
	/** Draw text using current context and font
	 * @param[in] x position on display
	 * @param[in] y position on display
	 * @param[str] str string to display
	 */
	void draw_text( coord_t x, coord_t y, const char* str );

	/* Draw line using current grephics context parameter
	 * @param[in] x0 start line point horiz
	 * @param[in] y0 start line point vert
	 * @param[in] x1 end line point horiz
	 * @param[in] y1 end line point vert
	 *
	 */
	void draw_line( coord_t x0, coord_t y0, coord_t x1, coord_t y1 );
	/** Set foreground color in GDI object
	 * @param[in] color  foreground color	 */

	void set_fg_color( color_t color )
	{
		m_color = color;
	}
	/** Set foreground color in GDI object
	   * @param[in] color  foreground color
	*/
	void set_bg_color( color_t color )
	{
		m_bg_color = color;
	}
	/** Set font in GDI context
	 * @param[in] font Font structure
	 */
	void set_font( const font_t &font )
	{
		m_font = &font;
	}
private:
	drv::disp_base &m_gdev;
	const font_t *m_font { &fonts::font_default } ;
	color_t m_color { rgb(255,255,255) };
	color_t m_bg_color { rgb(0,0,0) };
};
/* ------------------------------------------------------------------ */
}}
/* ------------------------------------------------------------------ */
#endif /* GDI_HPP_ */
/* ------------------------------------------------------------------ */
