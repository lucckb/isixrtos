/*
 * =====================================================================================
 *
 *       Filename:  waterfall.cpp
 *
 *    Description:  Waterfall widget
 *
 *        Version:  1.0
 *        Created:  19.05.2014 21:53:46
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#include <gfx/gui/waterfall.hpp>
#include <foundation/dbglog.h>
/* ------------------------------------------------------------------ */ 
namespace gfx {
namespace gui {
/* ------------------------------------------------------------------ */ 
namespace {
	//! Convert amplitude to color
	inline color_t ampl2color( unsigned char a ) {
        if( (a<43) )
            return rgb( 0,0, 255*(a)/43);
        if( (a>=43) && (a<87) )
            return rgb( 0, 255*(a-43)/43, 255 );
        if( (a>=87) && (a<120) )
            return rgb( 0,255, 255-(255*(a-87)/32));
        if( (a>=120) && (a<154) )
            return rgb( (255*(a-120)/33), 255, 0);
        if( (a>=154) && (a<217) )
            return rgb( 255, 255 - (255*(a-154)/62), 0);
        if( (a>=217)  )
            return rgb( 255, 0, 128*(a-217)/38);
		return color::White;
	}
	/** Scale line average */
	void scale_line_avg( color_t *target, const color_t *source, int src_width, int tgt_width )
	{
	/* N.B. because of several simplifications of the algorithm,
	*      the zoom range is restricted between 0.5 and 2. That
	*      is: tgt_width must be >= src_width/2 and <= 2*SrcWidth.
	*/
	int num_pixels = tgt_width;
	int mid = tgt_width / 2;
	int e = 0;
	color_t p;
	auto average = []( color_t a, color_t b ) { return ( a + b ) >> 1; };
	if (tgt_width > src_width)
		num_pixels--;
	while (num_pixels-- > 0) {
		p = *source;
		if (e >= mid)
		p = average(p, *(source+1));
		*target++ = p;
		e += src_width;
		if (e >= tgt_width) {
		e -= tgt_width;
		source++;
		} /* if */
	} /* while */
	if (tgt_width > src_width)
		*target = *source;
	}
}
/* ------------------------------------------------------------------ */ 
//! GUI dram frame
void waterfall::draw_frame()
{
	constexpr auto luma = 64;
	constexpr auto luma2 = luma/2;
	auto gdi = make_wgdi( );
	const auto c = get_coord() + get_owner().get_coord();
	//FRM1
	gdi.bright_fg_color( -luma );
	gdi.draw_line(c.x(), c.y()+c.cy(), c.x()+c.cx(), c.y()+c.cy() );
	gdi.draw_line(c.x()+c.cx(), c.y()+1, c.x()+c.cx(), c.y()+c.cy() );
	//FRM2
	gdi.bright_fg_color( -luma2 );
	gdi.draw_line(c.x()+1, c.y()+c.cy()-1, c.x()+c.cx(), c.y()-1+c.cy() );
	gdi.draw_line(c.x()+c.cx()-1, c.y()+1, c.x()+c.cx()-1, c.y()+c.cy() );
	//FRM3
	gdi.bright_fg_color( -luma );
	gdi.draw_line(c.x()+1, c.y(), c.x()+c.cx()-1, c.y() );
	gdi.draw_line(c.x(), c.y(), c.x(), c.y()+c.cy()-2 );
}
/* ------------------------------------------------------------------ */ 
//! Draw frequency selection line
void waterfall::draw_select_line()
{
#if 0
	if( m_freq_sel > 0 ) {
		const auto c = get_coord() + get_owner().get_coord();
		const auto lwidth = c.cx() - c_margin * 2;
		const auto xpos = (int(m_freq_sel) * int(lwidth) ) / int( m_top_freq ) + c.x() + c_margin;
		auto gdi = make_gdi();
		gdi.set_fg_color( color::White );
		gdi.draw_line( xpos , c.y()+1, xpos , c.y()+c.cy()-2 );
	}
#endif
}
/* ------------------------------------------------------------------ */ 
//! Handle waterfall event info
void waterfall::report_event( const input::event_info&  )
{
	if( m_data_ptr ) {
		modified();
	}
}
/* ------------------------------------------------------------------ */ 
//! Handle repaint 
void waterfall::repaint()
{
	if( !m_data_ptr ) {
		draw_frame();
		dbprintf("Invalid repaint");
		return;
	}
	//Update waterall scrol down before
	auto gdi = make_gdi();
	const auto c = get_coord() + get_owner().get_coord();
	const auto lwidth = c.cx() - c_margin * 2;
	const auto fftI0 = int( m_fftlen * int(m_f0) ) / int(m_fs2);
	const auto fftI1 = int( m_fftlen * int(m_f1) ) / int(m_fs2);
	const auto fftmax =  fftI1 - fftI0;
	//! Scroll the first line down
	gdi.scroll( c.x() + c_margin , c.y()+1, lwidth,
			c.cy() , -1, get_owner().get_layout().bg() 
	);
	for( gfx::coord_t i = 0; i < lwidth; ++i ) {
		const auto ampl = m_data_ptr[ ( i *  fftmax ) / lwidth + fftI0 ] / 128;
		gdi.set_pixel_color( i + c.x() + c_margin, c.y()+1, ampl2color( ampl ) );
	}
	//Draw bottom gui frame
	draw_frame();
	//Draw selection waterfall line
	draw_select_line();
	m_data_ptr = nullptr;
}
/* ------------------------------------------------------------------ */ 
}
}
/* ------------------------------------------------------------------ */ 
