/*
 * =====================================================================================
 *
 *       Filename:  waterfall.hpp
 *
 *    Description:  Waterfall implementation for example fft watterfall
 *
 *        Version:  1.0
 *        Created:  19.05.2014 20:56:11
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once

/* ------------------------------------------------------------------ */
#include <gfx/gui/widget.hpp>
/* ------------------------------------------------------------------ */ 
namespace gfx {
namespace gui {
/* ------------------------------------------------------------------ */ 

/** Watterfall class widget implementation. 
 *  Watterfall is an waterfall update widget for 
 *  example for FFT watterfal function for detect frequencies
 */

class waterfall : public widget {
	static constexpr auto c_margin = 2;
public:
	/** Waterfall constructor 
	 * @param[in] rect Widget size
	 * @param[in] layout Widget layout colors and fonts
	 * @param[in] win Parent window owner
	 * @param[in] in_len Message input size 
	 */
	waterfall( rectangle const& rect, layout const& layout, 
		window &win, size_t fft_len, unsigned short fsample_2,
		unsigned short f0, unsigned short f1 )
		: widget( rect, layout, win ), m_fftlen( fft_len ), 
		m_f0( f0 ), m_f1( f1 ), m_fs2( fsample_2 )
	{
	}
	//! Virtual destructor
	virtual ~waterfall() {
	}
	 //! Report an event
	virtual void report_event( const input::event_info& ev );
	//! New fft data
	void new_data( const  short* ptr ) {
		m_data_ptr = ptr;
	}
protected:
	 //! On repaint the widget return true when changed
	 virtual void repaint();
private:
	 //! Draw GUI frame
	 void draw_frame();
	 //! Draw frequency selection line
	 void draw_select_line();
private:
	const size_t m_fftlen;					//! Handle length of input waterfal msg
	const short *m_data_ptr {};				//! Data pointer
	const unsigned short m_f0;				//! Frequency Low
	const unsigned short m_f1;				//! Frequency high
	const unsigned short m_fs2;				//! Sample frequency/2
	unsigned short m_freq_sel { };		//! Frequency selection line
};
/* ------------------------------------------------------------------ */ 
}	//gui
}	//gfx
