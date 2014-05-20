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
	static inline color_t ampl2color( unsigned char a ) {
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
	//! Scroll the first line down
	gdi.scroll( c.x() + c_margin , c.y()+1, lwidth,
			c.cy() , -1, get_owner().get_layout().bg() 
	);
	for( gfx::coord_t i = 0; i < lwidth; ++i ) {
		const auto ampl = m_data_ptr[ ( i *  m_length ) / lwidth ] / 256;
		gdi.set_pixel_color( i + c.x() + c_margin, c.y()+1, ampl2color( ampl ) );
	}
	//Draw bottom gui frame
	draw_frame();
	m_data_ptr = nullptr;
}
/* ------------------------------------------------------------------ */ 
}
}
/* ------------------------------------------------------------------ */ 
