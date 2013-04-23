/*
 * fir_decimate.hpp
 *
 *  Created on: 22-04-2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------------- */
#ifndef DSP_FIR_DECIMATE_HPP_
#define DSP_FIR_DECIMATE_HPP_
/* ------------------------------------------------------------------------- */
namespace dsp {
/* ------------------------------------------------------------------------- */
/**
 * DT - data type
 * CT - coefficient type
 * N  - decimate length
 */
template<typename DT, typename CT, size_t LEN> class fir_decimate
{
public:
	fir_decimate( constexpr CT )
	{

	}
	DT operator()( DT value )
	{
	}
private:
	DT m_filter[LEN];
	constexpr CT m_coefs[];
};

/* ------------------------------------------------------------------------- */
}
/* ------------------------------------------------------------------------- */
#endif /* DSP_FIR_DECIMATE_HPP_ */

/* ------------------------------------------------------------------------- */
