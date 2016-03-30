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
#include "types.hpp"

namespace emeter {

	template <std::size_t PHASES = 3, std::size_t FFTSIZE=256>
	//! Main energy meter class library
	class energy_meter 
	{
	public:
		energy_meter( energy_meter& ) = delete;
		energy_meter& operator=(energy_meter&)=delete;
		/**   Calculate energy called from ISR vector
		 * @param[in] input Input buffer pointer with resampled data
		 * @return Buffer to fill by sampling procedure
		 */
		template< std::size_t PHASE>
			sample_t* process_voltage( sample_t* input ) noexcept {
				std::static_assert( PHASE<PHASES, "Invalid V phase num" );
				m_energies[PHASE].process_voltage( input );
		}

		template< std::size_t PHASE>
			sample_t* process_current( sample_t* input ) noexcept {
				std::static_assert( PHASE<PHASES, "Invalid I phase num" );
				m_energies[PHASE].process_current( input );
		}
		
		//! Get phase defined type
		template< std::size_t PHASE, typename TAG >
			TAG operator()( const TAG& tag ) const noexcept {
				return m_energies<TAG>();
			}
		template< typename TAG > 
			decltype() operator( const TAG& tag ) const noexcept {
				
			}
	private:
		std::array<energy_phase_n<FFTSIZE>, PHASES> m_energies;
	};
};

