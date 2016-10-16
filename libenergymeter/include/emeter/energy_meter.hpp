/*
 * =====================================================================================
 *
 *       Filename:  energy_meter.hpp
 *
 *    Description:  Lib energy meter core class
 *
 *        Version:  1.0
 *        Created:  30.03.2016 20:11:34
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include <cstddef>
#include <array>
#include <atomic>
#include <utility>
#include <complex>
#include <functional>
#include "types.hpp"
#include "detail/config.hpp"
#include "detail/tags.hpp"
#include "energy_phase_n.hpp"

namespace emeter {
	//! Energy counting tresh
	static constexpr auto ethresh = config::energy_cnt_tresh;
	static constexpr auto ecnt_scale  = 128;
	//! How long takes a single sampling time
	static constexpr energymeas_t wnd_smp_time = energymeas_t(1) /
		(energymeas_t( config::sample_rate )/energymeas_t(config::fftbuf_size));
	//! Per hour sampling time divide
	static constexpr auto wnd_smp_time_hrs = wnd_smp_time/energymeas_t(3600);
	//! Main energy meter class library
	class energy_meter
	{
		// Hold all energies
		struct pwr_cnt {
			accum_t p_plus;
			accum_t p_minus;
			accum_t q_plus;
			accum_t q_minus;
		};
		using energy_storage = typename std::aligned_storage_t<
			sizeof(energy_phase_n), alignof(energy_phase_n)>::type;
		//Get energy phase helper
			auto energy( size_t phase ) {
				return reinterpret_cast<energy_phase_n*>(&m_energies[phase]);
			}
			auto energy( size_t phase ) const {
				return reinterpret_cast<const energy_phase_n*>(&m_energies[phase]);
			}
		// Get 64 bit value with interrupt nolock
		static inline accum_t read_atomic_accum_t( const accum_t& input )
		{
			accum_t v1, v2;
			do {
				v1 = input;
				v2 = input;
			} while( v1 != v2 );
			return v1;
		}

		//! Auto scale energy calculation
		template< typename TAG >
		static typename TAG::value_type rescale_pwr( accum_t val, const TAG& ) {
			using mtype = typename TAG::value_type;
			bool half = ( val % ecnt_scale > ecnt_scale / 2 );
			val /= ecnt_scale;
			if( half ) ++val;
			return mtype(val) * wnd_smp_time_hrs;
		}
	public:
		energy_meter( energy_meter& ) = delete;
		energy_meter& operator=( energy_meter& ) = delete;
		//! Constructor
		energy_meter() {
			//Placement new initialization
			for( auto &ph : m_energies ) {
				new (&ph) energy_phase_n { m_scratch };
			}
		}
		//! Destructor
		~energy_meter() {
			for( auto &ph : m_energies ) {
				reinterpret_cast<energy_phase_n*>(&ph)->~energy_phase_n();
			}
		}

		/**Calculate energy called from ISR vector
		 * @param[in] input Input buffer pointer with resampled data
		 * @return Buffer to fill by sampling procedure
		 */
		template<std::size_t PHASE>
			sample_t* sample_voltage_begin() noexcept {
				static_assert( PHASE<config::n_phases, "Invalid V phase num" );
				return energy(PHASE)->sample_voltage_begin();
		}

		template<std::size_t PHASE>
			int sample_voltage_end() noexcept {
				static_assert( PHASE<config::n_phases, "Invalid V phase num" );
				return energy(PHASE)->sample_voltage_end();
			}

		template<std::size_t PHASE>
			sample_t* sample_current_begin() noexcept {
				static_assert( PHASE<config::n_phases, "Invalid I phase num" );
				return energy(PHASE)->sample_current_begin();
		}

		template<std::size_t PHASE>
			int sample_current_end() noexcept {
				static_assert( PHASE<config::n_phases, "Invalid V phase num" );
				return energy(PHASE)->sample_current_end();
			}

		//! Process thread should be called after calculation
		int calculate() noexcept {
			int err {};
			for( std::size_t ph=0; ph<config::n_phases; ++ph ) {
				err = energy(ph)->calculate();
				if( !err ) {
					calculate_energies( m_ecnt[ph], *energy(ph) );
				}
			}
			return err;
		}

		//! Get phase defined type
		template<typename TAG>
			typename TAG::value_type operator()
			( const std::size_t phase, const TAG& p ) const noexcept
			{
				return std::ref(*energy(phase))( p );
			}
		// Wh consumed
		typename tags::detail::watt_h_pos::value_type operator()
			( const std::size_t phase, tags::detail::watt_h_pos p ) const noexcept;

		// Wh produced
		typename tags::detail::watt_h_neg::value_type operator()
			( const std::size_t phase, tags::detail::watt_h_neg p) const noexcept;

		// varh consumed
		typename tags::detail::var_h_pos::value_type operator()
			( const std::size_t phase, tags::detail::var_h_pos p) const noexcept;

		// Wh produced
		typename tags::detail::var_h_neg::value_type operator()
			( const std::size_t phase, tags::detail::var_h_neg p) const noexcept;
	private:
		//! Calculate energies based on the phase
		void calculate_energies( pwr_cnt& ecnt, const energy_phase_n& ephn ) noexcept;
		//Adjust input energy
		static measure_t adjust_energy( measure_t e );
	private:
		// Energy counter for 3phases
		std::array<pwr_cnt,config::n_phases> m_ecnt {{}};
	private:
		static constexpr auto scratch_siz =
			(config::fft_size+config::fft_size/2+1) * sizeof(cplxmeas_t);
		//Temporary scratch memory for FFT calculation
		alignas(8) char m_scratch[ scratch_siz ];
		std::array<energy_storage, config::n_phases> m_energies;
	};
};

