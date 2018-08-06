/*
 * =====================================================================================
 *
 *       Filename:  spectrum_pwr.hpp
 *
 *    Description:  Spectrum calculator
 *
 *        Version:  1.0
 *        Created:  20.11.2017 21:33:39
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include <complex>
#include <limits>
#include <atomic>
#include <memory>

namespace dsp {

		class spectrum_pwr
		{
		public:
			//! FFT scale
			enum class scale : bool {		//! Type of FFT scale
				lin,	//! Normal linear
				log		//! Logarithm scale
			};
			//! Copy odd even or all elements
			enum class smp_type : char {
				all,	//! Copy all samples
				odd,	//! Compy odd sample
				even	//Copy even sample
			};
			//! Power factor file type
			using pow_t = short;
		private:
			//Number of samples
			static constexpr int LOG_SCALE =  std::numeric_limits<pow_t>::max() + 1;
			static constexpr int LOGVAL_SCALE = std::log2(std::log2( LOG_SCALE + 1 ) + 1);
			auto bsize() const noexcept {
				return 1U<<m_fftpow;
			}
		public:
			/** Construct fft object with selected width
			 * @param[in] fftpow spectrum width
			 */
			explicit spectrum_pwr( std::size_t fftpow, smp_type stype=smp_type::all,
					scale sc=scale::lin, pow_t factor = std::numeric_limits<pow_t>::max() )
				: m_cplx(new std::complex<pow_t>[1U<<fftpow] )
				, m_sampletype(stype), m_scale(sc), m_factor(factor), m_fftpow(fftpow)
			{}
			~spectrum_pwr() {}
			spectrum_pwr(spectrum_pwr&) = delete;
			spectrum_pwr& operator=(spectrum_pwr&) = delete;

			//! Buffer length
			auto size() const noexcept {
				return bsize()/2;
			}

			/** Call when you need to collect samples
			 * @param[in] in Input buffer
			 * @param[in] len Sample input length
			 * @return true when all samples completed
			 */
			bool operator()( const short in[], std::size_t len ) noexcept;
			/** Do FFT operation and return power
			 * @return pointer if FFT can be calculated
			 * or null when not
			 */
			const pow_t* operator()() const noexcept;
			//Check if can call next operator
			operator bool() const noexcept {
				return m_sample_buf_cnt>=bsize();
			}
			/** Commit bufer and release for othrt
			 * fft calculation
			 */
			auto commit() noexcept {
				unsigned short w { static_cast<unsigned short>(bsize()) };
				return m_sample_buf_cnt.compare_exchange_strong( w, 0);
			}
		private:
			//! Private buffer for data
			const std::unique_ptr<std::complex<pow_t>[]> m_cplx;
			//! Real cast to the outpt
			pow_t* const m_real { reinterpret_cast<pow_t*>(m_cplx.get()) };
			//! Sample type buffer to copy
			const smp_type m_sampletype;
			//! FFT scale
			const scale m_scale;
			//! FFT factor
			pow_t m_factor;
			//! Sample buffer counter
			mutable std::atomic<unsigned short> m_sample_buf_cnt {};
			//! FFT power factor
			const unsigned char m_fftpow;
		};
}

