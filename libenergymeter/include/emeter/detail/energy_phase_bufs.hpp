/*
 * =====================================================================================
 *
 *       Filename:  energy_phase_bufs.hpp
 *
 *    Description:  Calculate energy on the N phase
 *
 *        Version:  1.0
 *        Created:  30.03.2016 21:57:06
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

/** Double buffer mechanism with check buffer protection
 *  when one buffer is used for sampling second buffer is used for
 *  fft calculate. We have four buffer one buffer for U and second buffer
 *  for I. unsigned atomic variable is used for bit mark operation, every
 *  bufer has 3 bit state encoded using atomic variable. */

#pragma once
#include <cstddef>
#include <array>
#include <atomic>
#include <complex>
#include "../types.hpp"
#include "config.hpp"


namespace emeter {
namespace detail {
	//! Calculate energy in the Nth phase
	class energy_phase_bufs
	{
		static constexpr auto nbufs=4;
		enum class state {
			free,				//buffer is free
			collect_u,			//buffer is reservered by ADC collect voltage
			collect_i,			//buffer is reserved by ADC collect current
			ready_u,			//buffer is ready for calculate voltage,
			ready_i				//buffer is ready for calculate current
		};
		static constexpr auto buflen = config::fftbuf_size;
	public:
		enum error {
			e_ok = 0,
			e_buff = -768,
			e_not_yet = -769
		};
		energy_phase_bufs( energy_phase_bufs& ) = delete;
		energy_phase_bufs& operator=(energy_phase_bufs&) = delete;
		energy_phase_bufs() {
		}
		//ADC sampling process current
		sample_t* sample_current_begin() noexcept {
			return sample_find_change_state( state::free, state::collect_i );
		}
		//ADC sampling process voltage
		sample_t* sample_voltage_begin() noexcept {
			return sample_find_change_state( state::free, state::collect_u );
		}
		// End current processing
		int sample_current_end() noexcept {
			return sample_find_change_state( state::collect_i, state::ready_i )?e_ok:e_buff;
		}
		// End voltage processing
		int sample_voltage_end() noexcept {
			return sample_find_change_state( state::collect_u, state::ready_u )?e_ok:e_buff;
		}
		//! Calculate after data get
		int calculate() noexcept;
	private:
		/** Get sample buffer */
		sample_t* sample_find_change_state( state find, state replace ) noexcept;
		/** Replace buffer by given index */
		void sample_change_state_idx( size_t idx, state newstate ) {
			unsigned newval;
			unsigned oldval;
			do {
				oldval = m_states.load();
				newval = buf_stat(oldval).set( idx, newstate );
			} while( !m_states.compare_exchange_weak(oldval,newval) );
		}
	protected:
		//!Calculate FFT voltage
		virtual void do_calculate( const sample_t* u, const sample_t* raw_i ) noexcept = 0;
	private:
		std::array<std::array<sample_t,buflen>,nbufs> m_buf {{{}}};
		std::atomic<unsigned> m_states {};
	private:
		//! Internal class for handle buffer status
		class buf_stat {
			static constexpr auto bits=3U;
			static constexpr auto mask=(1U<<bits)-1U;
		public:
			static constexpr auto npos = -1;
			//Constructor
			buf_stat( unsigned s = 0 )
				: m_s(s) {}
			//Find the buffer with selected state
			int find( state s ) const {
				for( unsigned i=0,m=mask; i<nbufs; ++i, m<<=bits ) {
					if( ( m_s & m ) == static_cast<unsigned>(s)<<(i*bits) ) {
						return i;
					}
				}
				return npos;
			}
			//Set the buffer pos to state s
			unsigned set( size_t pos, state s ) {
				m_s &= ~(mask<<(bits*pos));
				m_s |= static_cast<unsigned>(s) << (bits*pos);
				return m_s;
			}
			//Conversion to unsigned operator
			operator unsigned() const {
				return m_s;
			}
		private:
			unsigned m_s;
		};
	};
}}

