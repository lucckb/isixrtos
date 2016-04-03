/*
 * =====================================================================================
 *
 *       Filename:  energy_phase_n.hpp
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
#pragma once
#include <cstddef>
#include <array>
#include <atomic>
#include "detail/tags.hpp"
#include "emeter_phase_impl.hpp"

namespace emeter {

	//! Calculate energy in the Nth phase
	template <std::size_t FFTLEN>
	class energy_phase_n : public detail::emeter_phase_impl
	{	
		enum { U_IDX, I_IDX };
	protected:
		static constexpr auto buflen = FFTLEN+FFTLEN/2;
		static constexpr auto fftlen = FFTLEN+FFTLEN/2;
	public:
		enum error {
			e_ok = 0,
			e_buff = -768
		};
		energy_phase_n( energy_phase_n& ) = delete;
		energy_phase_n& operator=(energy_phase_n&) = delete;
		energy_phase_n() {
		}
		// Get URMS
		typename tags::detail::u_rms::value_type 
			operator()( const tags::detail::u_rms& ) const noexcept
		{
			return m_u.load();
		}
		// Get IRMS
		typename tags::detail::i_rms::value_type 
			operator()( const tags::detail::i_rms& ) const noexcept
		{
			return m_i.load();
		}
		//ADC sampling process current
		sample_t* sample_current_begin() noexcept 
		{
			return sample_select_buf( I_IDX );
		}
		//ADC sampling process voltage
		sample_t* sample_voltage_begin() noexcept {
			return sample_select_buf( U_IDX );
		}
		// End current processing	
		int sample_current_end() noexcept 
		{
			const auto state = m_free[I_IDX].load();
			if( state>>2 != 0b10 && state>>2 !=0b01 )
				return e_buff;
			m_rdy.fetch_or( ((state>>2) & 0b11)<<I_IDX*2 );
			m_free[I_IDX].fetch_and( 0b0011 );
			return e_ok;
		}
		// End voltage processing
		int sample_voltage_end() noexcept 
		{
			const auto state = m_free[U_IDX].load();
			if( state>>2 != 0b10 && state>>2 !=0b01 ) 
				return e_buff;
			m_rdy.fetch_or( ((state>>2) & 0b11)<<U_IDX*2 );
			m_free[U_IDX].fetch_and( 0b0011 );
			return e_ok;
		}
		//! Calculate after data get
		int calculate( ) noexcept 
		{
			const auto rdy = m_rdy.load();
			int err;
			if( rdy&0b0001 && rdy&0b0100 ) {
				err = do_calculate( m_buf[U_IDX*2].data(), m_buf[I_IDX*2].data(), buflen );
				m_rdy.fetch_and( 0b1010 );
				m_free[U_IDX].fetch_or( 0b01 );
				m_free[I_IDX].fetch_or( 0b01 );
			}
			if( !err ) {
				if( rdy&0b0010 && rdy&0b1000 ) {
					err = do_calculate( m_buf[U_IDX*2+1].data(), m_buf[I_IDX*2+1].data(), buflen );
					m_rdy.fetch_and( 0b0101 );
					m_free[U_IDX].fetch_or( 0b10 );
					m_free[I_IDX].fetch_or( 0b10 );
				}
			}
			return err;
		}
	private:
		/** Get sample buffer */
		sample_t* sample_select_buf( size_t idx ) noexcept
		{
			sample_t* ret {};
			unsigned char oldval,newval;
			do {
				oldval = m_free[idx].load();
				if( oldval & 0b1100 ) {
					return nullptr;
				}
				if( oldval & 0b01 ) {
					ret = m_buf[idx*2].data();
					newval = ( oldval & 0b10 ) | 0b0100;
				} else if( oldval & 0b10 ) {
					ret = m_buf[idx*2+1].data();
					newval = ( oldval & 0b01 ) | 0b1000;
				} else {
					break;
				}
			} while( !m_free[idx].compare_exchange_weak(oldval,newval) );
			return ret;
		}
	protected:
		std::atomic<typename tags::detail::u_rms::value_type> m_u;
		std::atomic<typename tags::detail::i_rms::value_type> m_i;
		std::atomic<typename tags::detail::thd_u::value_type> m_thdu;
		std::atomic<typename tags::detail::thd_i::value_type> m_thdi;
	private:
		std::array<std::array<sample_t,buflen>,4> m_buf {{{}}};
		std::atomic<unsigned char> m_free[2] { { 0b11 }, { 0b11 } };
		std::atomic<unsigned char> m_rdy { 0x0 };
	};
};

