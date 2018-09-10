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
#include <periph/dma/channel.hpp>
#include <periph/dma/dma_channels.hpp>
#include <periph/core/error.hpp>
#include "stm32_dma_v1.hpp"
#include <stm32f3xx_ll_dma.h>
#include <stm32f3xx_ll_bus.h>

namespace periph::dma {

/** Controller create instance with stm32 version1 */
controller& controller::instance()
{
	static stm32_dma_v1 dmactrl;
	return std::ref(dmactrl);
}

/** Constructor */
stm32_dma_v1::stm32_dma_v1() {
#ifdef LL_AHB1_GRP1_PERIPH_DMA1
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
#endif
#ifdef LL_AHB1_GRP1_PERIPH_DMA2
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2);
#endif
}

/** Destructor */
stm32_dma_v1::~stm32_dma_v1()
{
#ifdef LL_AHB1_GRP1_PERIPH_DMA1
	LL_AHB1_GRP1_DisableClock(LL_AHB1_GRP1_PERIPH_DMA1);
#endif
#ifdef LL_AHB1_GRP1_PERIPH_DMA2
	LL_AHB1_GRP1_DisableClock(LL_AHB1_GRP1_PERIPH_DMA2);
#endif
}

/** Single tranfer from controller */
int stm32_dma_v1::single(channel& chn, mem_ptr dest, cmem_ptr src, size len)
{
	isix::mutex_locker _lck(m_mtx);
	auto& cnf = channel_config(chn);
	const auto chnf = find_first_unused(cnf.dev_id);
	if(chnf<0) return chnf;
	(void)dest; (void)src; (void)len;
	return 0;
}
/** Single Continous stop tranaction */
int stm32_dma_v1::continuous_start(channel& chn, mem_ptr mem0, mem_ptr mem1, size len)
{
	isix::mutex_locker _lck(m_mtx);
	(void)chn; (void)mem0; (void)mem1; (void)len;
	return 0;
}
/** Continous stop transaction */
int stm32_dma_v1::continous_stop(channel& chn)
{
	isix::mutex_locker _lck(m_mtx);
	(void)chn;
	return 0;
}

/** Abort pending transaction */
int stm32_dma_v1::abort(channel& chn)
{
	isix::mutex_locker _lck(m_mtx);
	(void)chn;
	return 0;
}


/** Find first unused channel slot */
int stm32_dma_v1::find_first_unused(unsigned device)
{
	if(device >= sizeof devid::detail::dev_chn_map) {
		return error::inval;
	}
	const auto mask = devid::detail::dev_chn_map[device];
	for(auto i=0U; i<nchns; ++i) {
		if((mask&(1U<<i)) && m_act_chns[i]==nullptr) {
			return i;
		}
	}
	return error::noent;
}

} //periph::dma
