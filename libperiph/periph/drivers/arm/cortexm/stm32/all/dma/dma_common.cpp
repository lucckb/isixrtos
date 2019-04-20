/*
 * =====================================================================================
 *
 *       Filename:  dma_common.cpp
 *
 *    Description:  DMA common API
 *
 *        Version:  1.0
 *        Created:  20.04.2019 18:16:24
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include <periph/dma/dma_common.hpp>
#include <isix/cortexm/memorymap.h>

namespace periph::dma::detail {
	/** Determine transfer mode */
	tmode transfer_mode(cmem_ptr dst, cmem_ptr src) {
		const bool src_periph = memmap_is_periph_addr(reinterpret_cast<uintptr_t>(src));
		const bool dst_periph = memmap_is_periph_addr(reinterpret_cast<uintptr_t>(dst));
		if(!src_periph && dst_periph) {
			return tmode::mem2periph;
		} else if(src_periph && !dst_periph) {
			return tmode::periph2mem;
		} else if(!src_periph && !dst_periph) {
			return tmode::mem2mem;
		} else {
			return tmode::error;
		}
	}
}



