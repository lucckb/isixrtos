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
#include "types.hpp"
#include "detail/tags.hpp"
#include "energy_phase_n.hpp"

namespace emeter {

	template <std::size_t PHASES = 3,unsigned FS = 4000,std::size_t FFTSIZE=256>
	//! Main energy meter class library
	class energy_meter 
	{
	
	public:
		energy_meter( energy_meter& ) = delete;
		energy_meter& operator=( energy_meter& ) = delete;
		energy_meter() {

		}

		/**Calculate energy called from ISR vector
		 * @param[in] input Input buffer pointer with resampled data
		 * @return Buffer to fill by sampling procedure
		 */
		template<std::size_t PHASE>
			sample_t* sample_voltage_begin() noexcept {
				static_assert( PHASE<PHASES, "Invalid V phase num" );
				return m_energies[PHASE].sample_voltage_begin();
		}

		template<std::size_t PHASE>
			sample_t* sample_current_begin() noexcept {
				static_assert( PHASE<PHASES, "Invalid I phase num" );
				return m_energies[PHASE].sample_current_begin();
		}
		
		//! Process thread should be called after calculation
		int calculate() noexcept {
			for( std::size_t ph=0; ph<PHASES; ++ph ) {
				auto err = m_energies[ph].calculate();
				if( err ) return err;
			}
			return 0;
		}
		
		//! Get phase defined type
		template<typename TAG>
			typename TAG::value_type operator()
			( const std::size_t phase, const TAG& p ) const noexcept 
			{
				return m_energies[phase]( p );
			}
	private:
		std::array<energy_phase_n<FFTSIZE>, PHASES> m_energies;
	};
};

