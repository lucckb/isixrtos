/*
 * =====================================================================================
 *
 *       Filename:  dma_common.hpp
 *
 *    Description:  DMA common
 *
 *        Version:  1.0
 *        Created:  19.04.2019 20:14:42
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once
#include <periph/dma/types.hpp>

namespace periph::dma::detail {

	enum class tmode {
		error,
		mem2mem,
		periph2mem,
		mem2periph
	};
	/** Determine transfer mode */
	tmode transfer_mode(cmem_ptr dst, cmem_ptr src);
}

