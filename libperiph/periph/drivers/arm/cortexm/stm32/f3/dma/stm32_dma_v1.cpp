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

#include "stm32_dma_v1.hpp"
#include <periph/dma/controller.hpp>
#include <periph/dma/channel.hpp>
#include <periph/dma/dma_channels.hpp>
#include <periph/core/error.hpp>
#include <stm32f3xx_ll_dma.h>
#include <stm32f3xx_ll_bus.h>
#include <isix/arch/irq_platform.h>
#include <isix/arch/irq.h>
#include <isix/arch/cache.h>
#include <foundation/sys/dbglog.h>

namespace periph::dma {

namespace {
	auto chn2cntrl( int /*chn*/ ) {
		return DMA1;
	}
	auto chn2hwchn(int chn) {
		return chn + LL_DMA_CHANNEL_1;
	}
}

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

//! DMA configure
void stm32_dma_v1::dma_configure(const detail::controller_config& cfg, int chn)
{
	//Configure DMA according to the channel flags
	switch(cfg.flags & detail::mask::dev_mode) {
		case mode_memory_to_memory:
			LL_DMA_SetDataTransferDirection( chn2cntrl(chn), chn2hwchn(chn),
				LL_DMA_DIRECTION_MEMORY_TO_MEMORY );
			break;
		case mode_peripheral_to_memory:
			LL_DMA_SetDataTransferDirection( chn2cntrl(chn), chn2hwchn(chn),
				LL_DMA_DIRECTION_PERIPH_TO_MEMORY );
			break;
		case mode_memory_to_peripheral:
			LL_DMA_SetDataTransferDirection( chn2cntrl(chn), chn2hwchn(chn),
				LL_DMA_DIRECTION_MEMORY_TO_PERIPH );
			break;
	}
	switch(cfg.flags & detail::mask::src_transfer_size) {
		case mode_src_size_byte:
			LL_DMA_SetMemorySize
			break;
		case mode_src_size_halfword:
			break;
		case mode_src_size_word:
			break;
	}
	//Configure interrupt
	chn += DMA1_Channel1_IRQn;
	dbg_info("Set irq: %i prio: %i:%i", chn, cfg.irqh, cfg.irql);
	isix::set_irq_priority(chn, {uint8_t(cfg.irqh), uint8_t(cfg.irql)});
	isix::request_irq(chn);
}

} //periph::dma
