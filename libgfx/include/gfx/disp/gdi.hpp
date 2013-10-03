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
#include <gfx/disp/static_bitmap.hpp>
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
	explicit gdi( drv::disp_base &gdev, color_t fg = rgb(255,255,255),
			color_t bg = rgb(0,0,0), const font_t* fnt= &fonts::font_default )
	: m_gdev( gdev ), m_color( fg ), m_bg_color( bg ), m_font(fnt)
	{
	}
	//Move constructor
	gdi( gdi&& ) = default;
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

	/** Calculate and get text size
	 * @param[in] str Input text
	 * @return Text length
	 */
	coord_t get_text_width( const char *str );

	/** Set PIXEL
	 * @param[in] x position X
	 * @param[in] y position Y
     * @param[in] bg use bg color
    */
	void set_pixel( coord_t x, coord_t y, bool bg=false)
	{
		m_gdev.set_pixel( x, y, bg?m_bg_color:m_color );
	}
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

	/** Fill the area with selected color
	 * @param[in] x Position X
	 * @param[in] y Position Y
	 * @param[in] cx Height
	 * @param[in] cy Width
	 * @param[in] bg Background color
	 *
	 */
	void fill_area( coord_t x, coord_t y, coord_t cx, coord_t cy, bool bg = false)
	{
		m_gdev.fill( x, y, cx, cy, bg?m_bg_color:m_color );
	}

	/** Draw circle
	 * @param[in] x Start point X
	 * @param[in] y Start point Y
	 * @param[in] radius Radius
	 */
	void draw_circle( coord_t x, coord_t y, coord_t radius );


	/** Draw circle
	  * @param[in] x Start point X
	  * @param[in] y Start point Y
	  * @param[in] a Radius A
	  * @param[in] b Radius B
	  */
	void draw_ellipse( coord_t x, coord_t y, coord_t a, coord_t b );


	/** Draw static bitmap
	 * @param[in] x Start point X
	 * @param[in] y Start point Y
	 * @param[in] bmp Static bitmap from memory
	 * @return Error code
	 */
	int draw_image( coord_t x, coord_t y, const cmem_bitmap_t &bitmap );

	/** Set foreground color in GDI object
	   * @param[in] color  foreground color
	*/
	void set_fg_color( color_t color )
	{
		m_color = color;
	}
	/** Set background color in GDI object
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
	/** Enable or disable background fill */
	void set_fill_background( bool enable )
	{
		m_bg_fill = enable;
	}
private:
	drv::disp_base &m_gdev;
	color_t m_color;
	color_t m_bg_color;
	const font_t* m_font;
	bool m_bg_fill { false };	//Fill the background
};
/* ------------------------------------------------------------------ */
}}
/* ------------------------------------------------------------------ */
#endif /* GDI_HPP_ */
/* ------------------------------------------------------------------ */
