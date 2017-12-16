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
#include <foundation/sys/dbglog.h>

namespace gfx {
namespace gui {

namespace {
	//! Convert amplitude to color
	inline color_t ampl2color( short a ) {
		static constexpr auto scale = 128;
		a /= scale;
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

//! Draw frequency selection line
void waterfall::draw_select_line()
{
	if( m_freq_sel > 0 ) {
		const auto c = get_coord() + get_owner().get_coord();
		const auto lwidth = c.cx() - c_margin * 2;
		const auto p0 = ( int(m_freq_sel-m_f0) * int(lwidth) ) / int(m_f1-m_f0);
		const auto xpos = p0 + c.x() + c_margin;
		auto gdi = make_wgdi(); 
		gdi.set_fg_color( color::White );
		if( m_last_line_pos ) {
			gdi.swap_colors();
			gdi.draw_line( m_last_line_pos , c.y()+1, m_last_line_pos , c.y()+c.cy()-2 );
			gdi.swap_colors();
		}
		gdi.draw_line( xpos , c.y()+1, xpos , c.y()+c.cy()-2 );
		m_last_line_pos = xpos;
	}
}

//! Handle waterfall event info
void waterfall::report_event( const input::event_info& ev )
{
	using evinfo = input::event_info;
	using kstat  = input::detail::keyboard_tag::status;
	if( m_data_ptr ) {
		modified();
	}
	bool mflag = false;
	if( ev.type == evinfo::EV_KEY && !m_readonly ) {
		if( ev.keyb.stat==kstat::DOWN || ev.keyb.stat==kstat::RPT ) {
			if( ev.keyb.key == input::kbdcodes::os_arrow_down ) {
				if( m_freq_sel > m_f0 ) {
					m_freq_sel -= (ev.keyb.ctrlbits.lctrl)?(m_freq_fast_step):(m_freq_step);
					mflag = true;
				}
			}
			else if( ev.keyb.key == input::kbdcodes::os_arrow_up ) {
				if( m_freq_sel < m_f1 ) {
					m_freq_sel += (ev.keyb.ctrlbits.lctrl)?(m_freq_fast_step):(m_freq_step);
					mflag = true;
				}
			}
		}
	} else if( ev.type == evinfo::EV_KNOB && !m_readonly )
	{
		if( ev.knob.diff>0 && m_freq_sel>m_f0 ) {
			m_freq_sel += (ev.knob.diff>2)?(m_freq_fast_step):(m_freq_step);
			mflag = true;
		}
		if(ev.knob.diff<0 && m_freq_sel<m_f1 ) {
			m_freq_sel -= (ev.knob.diff<2)?(m_freq_fast_step):(m_freq_step);
			mflag = true;
		}
	}
	if( mflag ) {
		//Report change event
		modified();
		event btn_event( this, event::evtype::EV_CHANGE );
		emit( btn_event );
	}
}

//! Handle repaint
void waterfall::repaint( bool focus )
{
	if( m_readonly ) {
		if( !focus && !is_modified() ) {
			return;
		}
		auto gdi = make_gdi();
		const auto c = get_coord() + get_owner().get_coord();
		const int lwidth = c.cx() - c_margin * 2;
		gdi.set_bg_color( get_layout().sel() );
		gdi.fill_area( c.x() + c_margin, c.y(), lwidth, c.cy(), true );
		draw_frame();
		return;
	}
	if( !m_data_ptr ) {
		draw_frame();
		draw_select_line();
		return;
	}
	//Update waterall scrol down before
	auto gdi = make_gdi();
	const auto c = get_coord() + get_owner().get_coord();
	const int lwidth = c.cx() - c_margin * 2;
	const int fftI0 = int( int(m_fftlen) * int(m_f0) ) / int(m_fs2);
	const int fftI1 = int( int(m_fftlen) * int(m_f1) ) / int(m_fs2);
	const int fftwidth =  fftI1 - fftI0;
	//! Scroll the first line down
	gdi.scroll( c.x() + c_margin , c.y()+1, lwidth,
			c.cy() , -1, get_owner().get_layout().bg()
	);
	for( gfx::coord_t u = 0; u < lwidth; ++u ) {
		if( u == 0 ) {
			gdi.set_pixel_color( u + c.x() + c_margin, c.y()+1, ampl2color( m_data_ptr[fftI0] ) );
		}
		else {
			//! Average pixel algorithm
			int v = (u * fftwidth) / lwidth + fftI0;
			int a = (u * fftwidth) % lwidth; 
			auto level = ((lwidth-a)*int(m_data_ptr[v]))/lwidth + (a*m_data_ptr[v+1])/lwidth;
			gdi.set_pixel_color( u + c.x() + c_margin, c.y()+1, ampl2color( level ) );
		}
	}
	//Draw bottom gui frame
	draw_frame();
	//Draw selection waterfall line
	draw_select_line();
	m_data_ptr = nullptr;
}

}
}
