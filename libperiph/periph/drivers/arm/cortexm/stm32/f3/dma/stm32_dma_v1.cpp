/*
 * =====================================================================================
 *
 *       Filename:  controller.cpp
 *
 *    Description:   DMA controller for libperiph
 *
 *        Version:  1.0
 *        Created:  09/10/2018 15:46:18
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#include <periph/dma/controller.hpp>
#include "stm32_dma_v1.hpp"

namespace periph::dma {

/** Controller create instance with stm32 version1 */
controller& controller::instance()
{
	static stm32_dma_v1 dmactrl;
	return std::ref(dmactrl);
}

/** Single tranfer from controller */
int stm32_dma_v1::single(channel& chn, mem_ptr dest, cmem_ptr src, size len)
{
	(void)chn; (void)dest; (void)src; (void)len;
	return 0;
}
/** Single Continous stop tranaction */
int stm32_dma_v1::continuous_start(channel& chn, mem_ptr mem0, mem_ptr mem1, size len)
{
	(void)chn; (void)mem0; (void)mem1; (void)len;
	return 0;
}
/** Continous stop transaction */
int stm32_dma_v1::continous_stop(channel& chn)
{
	(void)chn;
	return 0;
}

/** Abort pending transaction */
int stm32_dma_v1::abort(channel& chn)
{
	(void)chn;
	return 0;
}

}
