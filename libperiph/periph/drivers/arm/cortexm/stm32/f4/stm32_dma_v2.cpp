/*
 * =====================================================================================
 *
 *       Filename:  stm32_dma_v2.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  15.04.2019 13:59:53
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include "stm32_dma_v2.hpp"
#include <periph/core/error.hpp>

//TODO: Implement DMA controller version2

namespace periph::dma {
/** Controller create instance with stm32 version1 */
controller& controller::instance()
{
	static stm32_dma_v2 dmactrl;
	return std::ref(dmactrl);
}


/** Single tranfer from controller */
int stm32_dma_v2::single(channel& chn, mem_ptr dest, cmem_ptr src, size len) {
	(void)chn;
	(void)dest;
	(void)src;
	(void)len;
	return error::unimplemented;
}
/** Single Continous stop tranaction */
int stm32_dma_v2::continuous_start(channel& chn, mem_ptr mem0, mem_ptr mem1, size len)
{
	(void)chn;
	(void)mem0;
	(void)mem1;
	(void)len;
	return error::unimplemented;
}
/** Continous stop transaction */
int stm32_dma_v2::continous_stop(channel& chn)
{
	(void)chn;
	return error::unimplemented;
}
/** Abort pending transaction */
int stm32_dma_v2::abort(channel& chn)
{
	(void)chn;
	return error::unimplemented;
}

}
