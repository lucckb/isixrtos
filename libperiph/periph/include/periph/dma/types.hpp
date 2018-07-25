/*
 * =====================================================================================
 *
 *       Filename:  types.hpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  25.07.2018 21:21:11
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once
#include <cstddef>

namespace periph::dma {
	using mem_ptr = void*;
	using cmem_ptr = const void*;
	using size = std::size_t;
}
