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


#include <gfx/gui/widget.hpp>

namespace gfx {
namespace gui {


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
	 * @param[in] fft_len FFT calcululator lenghth
	 * @param[in] fsample_2 Sampling frequency
	 * @param[in] f0 Starting frequency
	 * @param[in] f1 End frequency
	 * @param[in] f_sel Selected frequency
	 */
	waterfall( rectangle const& rect, layout const& layout, 
		window &win, size_t fft_len, unsigned short fsample_2,
		unsigned short f0, unsigned short f1, unsigned short f_sel )
		: widget( rect, layout, win ), m_fftlen( fft_len ), 
		m_f0( f0 ), m_f1( f1 ), m_fs2( fsample_2 ), m_freq_sel( f_sel )
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
	//! Get selected frequency
	unsigned short freq_sel() const {
		return m_freq_sel;
	}
	//! Set watterfall readonly
	void readonly( bool ro ) {
		m_readonly = ro;
		modified();
	}
	/** Set default frequency stepping
	 * @param[in] slow Slow frequency step path
	 * @param[in] fast Fast frequency step path
	 */
	void set_step( unsigned short slow, unsigned short fast ) noexcept {
		m_freq_step = slow;
		m_freq_fast_step = fast;
	}
protected:
	 //! On repaint the widget return true when changed
	 virtual void repaint( bool focus );
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
	unsigned short m_freq_sel;				//! Frequency selected
	gfx::coord_t m_last_line_pos {};		//! Last line position
	bool m_readonly {};						//! If component is RO
	unsigned short m_freq_step { 8 };		//! Default frequency step
	unsigned short m_freq_fast_step { 50 };		//! Default frequency step
};

}	//gui
}	//gfx
