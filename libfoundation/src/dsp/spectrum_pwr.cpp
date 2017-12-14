/*
 * =====================================================================================
 *
 *       Filename:  spectrum_pwr.cpp
 *
 *    Description:  Spectrum calculator implementation
 *
 *        Version:  1.0
 *        Created:  20.11.2017 22:06:38
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#include <foundation/dsp/spectrum_pwr.hpp>
#include <foundation/dsp/fft.hpp>
#include <foundation/dsp/sqrt_int.hpp>
#include <foundation/dsp/log2_int.hpp>
#include <foundation/sys/dbglog.h>
#include <algorithm>
#include <limits>
#include <cstring>

namespace dsp
{


namespace {
	//Inline POW2
	constexpr inline unsigned pow2( unsigned u ) {
		return u * u;
	}
}


// Called on sample collect
bool spectrum_pwr::operator()( const short in[], std::size_t len ) noexcept
{
	if( len > bsize() ) len = bsize();
	unsigned short slen,tocopy;
	do {
		slen = m_sample_buf_cnt.load();
		if( slen>=bsize() ) return true;
		tocopy = bsize() - slen;
		switch(m_sampletype) {
		case smp_type::all:
			std::memcpy(&m_real[slen], in, sizeof(in[0])*tocopy);
			break;
		case smp_type::odd: //1,3,5,7,9
			for(std::size_t i=0; i<tocopy; i++)  m_real[slen+i]=in[i*2+1];
			break;
		case smp_type::even: //0,2,4,6
			for(std::size_t i=0; i<tocopy; i++)  m_real[slen+i]=in[i*2];
			break;
		};
	} while(m_sample_buf_cnt.compare_exchange_weak(slen, slen+tocopy));
	return m_sample_buf_cnt>=len;
}

// Called on sample get
const spectrum_pwr::pow_t* spectrum_pwr::operator()() const noexcept
{
	namespace fft = dsp::refft;
	namespace dint = dsp::integer;
	if( m_sample_buf_cnt < bsize() )
		return nullptr;
	//Calculate FFT real transform
	fft::fft_real( m_cplx.get(),m_real, m_fftpow );
	//Calculate energies
	for( unsigned i=0; i<bsize()/2; i++ ) {
		m_real[i] = dint::sqrt( pow2(m_cplx[i].real()) + pow2(m_cplx[i].imag()) );
	}
	//Normalize the graph and do log
	int max_sample = *std::max_element( m_real, m_real + bsize()/2 );
	if( max_sample == 0 ) max_sample = 1;
	const int max_value = (m_scale==scale::log)?(std::numeric_limits<pow_t>::max()):(m_factor);
	for( unsigned i=0; i<bsize()/2; i++ ) {
		m_real[i] = (int(m_real[i])* max_value) / max_sample;
		if(m_scale == scale::log ) {
			const int lsample = dint::log2_0_1<unsigned,LOG_SCALE>( m_real[i])>>LOGVAL_SCALE;
			if( m_factor == std::numeric_limits<pow_t>::max() )
				m_real[i] = lsample;
			else
				m_real[i] = (lsample*m_factor)/std::numeric_limits<pow_t>::max();
		}
	}
	m_sample_buf_cnt = 0;
	return m_real;
}

}
