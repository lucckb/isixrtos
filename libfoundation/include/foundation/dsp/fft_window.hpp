/*
 * =====================================================================================
 *
 *       Filename:  fft_window.hpp
 *
 *    Description:  FFT window function
 *
 *        Version:  1.0
 *        Created:  08.10.2016 22:26:56
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include <type_traits>
#include <algorithm>

namespace dsp {
namespace window {

	/** Generic aplay window function
	 * @param[in] input  Input data transform from i
	 * @param[out] output Output data transform to
	 * @return none */
	template <size_t Size, typename Window_Func, typename In_Type, typename Out_Type >
		void apply_generic( const In_Type input[],  Out_Type output[] ) {
			static_assert( std::is_function<Window_Func>::value,
					"Function is required as template Window_Func"
			);
			//TODO: Window generation function 
			std::transform( input, input+Size, output, []( auto inp ) { return inp; } );
		}
}}


