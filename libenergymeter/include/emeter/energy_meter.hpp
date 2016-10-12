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
	//! Main energy meter class library
	class energy_meter
	{
		using energy_storage = typename std::aligned_storage_t<
			sizeof(energy_phase_n), alignof(energy_phase_n)>::type;
		//Get energy phase helper
			auto energy( size_t phase ) {
				return reinterpret_cast<energy_phase_n*>(&m_energies[phase]);
			}
			auto energy( size_t phase ) const {
				return reinterpret_cast<const energy_phase_n*>(&m_energies[phase]);
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
			sample_t* sample_current_begin() noexcept {
				static_assert( PHASE<config::n_phases, "Invalid I phase num" );
				return energy(PHASE)->sample_current_begin();
		}

		//! Process thread should be called after calculation
		int calculate() noexcept {
			int err {};
			for( std::size_t ph=0; ph<config::n_phases; ++ph ) {
				auto err2 = energy(ph)->calculate();
				if( !err ) err = err2;
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
	private:
		static constexpr auto scratch_siz =
			(config::fft_size+config::fft_size/2+1) * sizeof(cplxmeas_t);
		//Temporary scratch memory for FFT calculation
		alignas(8) char m_scratch[ scratch_siz ];
		std::array<energy_storage, config::n_phases> m_energies;
	};
};

