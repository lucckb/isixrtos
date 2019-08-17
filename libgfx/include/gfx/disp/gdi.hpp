 
/*
 * gdi.hpp
 *
 *  Created on: 14-06-2013
 *      Author: lucck
 */
 
#pragma once
 
#include <gfx/drivers/disp/disp_base.hpp>
#include <gfx/disp/bitmap_fonts.hpp>
#include <gfx/disp/static_bitmap.hpp>
#include <algorithm>
 
namespace gfx {
namespace disp {

 
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
	 * @return 0 if fail or new pos
	 */
	coord_t draw_text( coord_t x, coord_t y, int ch );
	/** Draw text using current context and font
	 * @param[in] x position on display
	 * @param[in] y position on display
	 * @param[str] str string to display
	 * @return X position after the last char
	 */
	coord_t draw_text( coord_t x, coord_t y, const char* str );

	/** Calculate and get text size
	 * @param[in] str Input text
	 * @return Text length
	 */
	coord_t get_text_width( const char *str ) const;

	/** Calculate and get text second version char only
	 * @param[in] ch Input text
	 * @return Text length
	 */
	coord_t get_text_width( const char ch ) const;

	/** Calculate text height
	 * @return text height */
	coord_t get_text_height( ) const;


	/** Set Pixel
	 * @param[in] x position X
	 * @param[in] y position Y
     * @param[in] bg use bg color
    */
	void set_pixel( coord_t x, coord_t y, bool bg=false)
	{
		m_gdev.set_pixel( x, y, bg?m_bg_color:m_color );
	}

	/** Set pixel using color 
	 * @param[in] x position X
	 * @param[in] y position Y
	 * @param[in] color Pixel color */
	void set_pixel_color( coord_t x, coord_t y, color_t color ) {
		m_gdev.set_pixel( x, y, color );
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

	/** Change brightnes of fg color 
	 * @param[in] luma Luminance change value 
	 */
	void bright_fg_color( int luma ) {
		m_color = colorspace::brigh( m_color, luma );
	}

	/** Change brightnes of fg color 
	 * @param[in] luma Luminance change value 
	 */
	void bright_bg_color( int luma ) {
		m_bg_color = colorspace::brigh( m_bg_color, luma );
	}

	/** Set foreground color in GDI object
	   * @param[in] color  foreground color
	*/
	void set_fg_color( color_t color ) {
		m_color = color;
	}

	/** Set background color in GDI object
	   * @param[in] color  foreground color
	*/
	void set_bg_color( color_t color ) {
		m_bg_color = color;
	}

	/** Swap FG and BG colors
	 */
	void swap_colors() {
		std::swap( m_color, m_bg_color );
	}

	/** Set font in GDI context
	 * @param[in] font Font structure
	 */
	void set_font( const font_t &font ) {
		m_font = &font;
	}

	/** Enable or disable Fill */
	void set_fill( bool enable ) {
		m_bg_fill = enable;
	}

	/** Vertical scroll data 
	 * @param[in] x Start position x
	 * @param[in] y Start position y 
	 * @param[in] cx Horizontal scroll size
	 * @param[in] cy Veritcal scroll size 
	 * @param[in] lines Lines to scroll
	 * @param[in] bg_color Background color to fill
	 **/
	void scroll( coord_t x, coord_t y, coord_t cx, coord_t cy, int lines, color_t bg_color )
	{
		m_gdev.vert_scroll(x, y, cx, cy, lines, bg_color);
	}
private:
	drv::disp_base &m_gdev;
	color_t m_color;
	color_t m_bg_color;
	const font_t* m_font;
	bool m_bg_fill { false };	//Fill the background
};
 
}}
 
