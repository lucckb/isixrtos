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
#include <isix/scheduler.h>
#include <foundation/sys/dbglog.h>
#include <periph/dma/dma_interrupt_handlers.hpp>

namespace periph::dma {

namespace {
	auto chn2cntrl( int /*chn*/ ) {
		return DMA1;
	}
	auto chn2hwchn(int chn) {
		return chn + LL_DMA_CHANNEL_1;
	}
	auto chn2tcbit(int chn) {
		chn %= 8U;
		return 1U<<(1U+4U*chn);
	}
	auto chn2tebit(int chn) {
		chn %= 8U;
		return 1U<<(3U+4U*chn);
	}
	auto chn2irqn(int chn) {
		return chn + DMA1_Channel1_IRQn;
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
	LL_AHB0_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2);
#endif
    for(auto chn=0U;chn<nchns;++chn) {
		isix::request_irq(chn2irqn(chn));
	}
}

/** Destructor */
stm32_dma_v1::~stm32_dma_v1()
{
    for(auto chn=0U;chn<nchns;++chn) {
		isix::free_irq(chn2irqn(chn));
	}
#ifdef LL_AHB1_GRP1_PERIPH_DMA1
	LL_AHB1_GRP1_DisableClock(LL_AHB1_GRP1_PERIPH_DMA1);
#endif
#ifdef LL_AHB1_GRP1_PERIPH_DMA2
	LL_AHB1_GRP1_DisableClock(LL_AHB1_GRP1_PERIPH_DMA2);
#endif
	dbg_info("Destroy controller");
}

/** Single tranfer from controller */
int stm32_dma_v1::single(channel& chn, mem_ptr dest, cmem_ptr src, size len)
{
	m_mtx.lock();
	auto& cnf = channel_config(chn);
	int chnf;
	for(;;)
	{
		chnf = find_first(cnf.dev_id,true);
		if(chnf<0)
		{
			chnf = find_first(cnf.dev_id,false);
			if(chnf<0)
			{
				dbg_err("Unable to find devmatching channel %i", chnf);
				m_mtx.unlock();
				return chnf;
			}
			else
			{
				m_mtx.unlock();
				chnf = wait();
				if(chnf<0) return chnf;
				m_mtx.lock();
			}
		}
		else
		{
			break;
		}
	}
	const auto tmode = detail::transfer_mode(dest,src);
	int res = dma_flags_configure(cnf,tmode,chnf);
	if(res<0) {
		dbg_err("Dma flag configure error %i", chnf);
		m_mtx.unlock();
		return res;
	}
	remap_alt_channel(cnf.dev_id,chnf);
	m_act_chns[chnf] = true;
	set_handled_channel(chn,chnf);
	drivers::dma::_handlers::register_handler(chnf, [this, chnf, &chn]() {
		if(READ_BIT(chn2cntrl(chnf)->ISR, chn2tcbit(chnf)) == chn2tcbit(chnf)) {
			WRITE_REG(chn2cntrl(chnf)->IFCR,chn2tcbit(chnf));
			LL_DMA_DisableChannel(chn2cntrl(chnf),chn2hwchn(chnf));
			LL_DMA_DisableIT_TC(chn2cntrl(chnf),chn2hwchn(chnf));
			LL_DMA_DisableIT_TE(chn2cntrl(chnf),chn2hwchn(chnf));
			channel_callback(chn, false);
			set_handled_channel(chn);
			m_act_chns[chnf] = false;
			broadcast_all();
		}
		if(READ_BIT(chn2cntrl(chnf)->ISR, chn2tebit(chnf)) == chn2tebit(chnf)) {
			WRITE_REG(chn2cntrl(chnf)->IFCR,chn2tebit(chnf));
			LL_DMA_DisableChannel(chn2cntrl(chnf),chn2hwchn(chnf));
			LL_DMA_DisableIT_TC(chn2cntrl(chnf),chn2hwchn(chnf));
			LL_DMA_DisableIT_TE(chn2cntrl(chnf),chn2hwchn(chnf));
			channel_callback(chn, true);
			set_handled_channel(chn);
			m_act_chns[chnf] = false;
			broadcast_all();
		}
	});
	dma_addr_configure(dest,src,len/res,chnf,!(cnf.flags&mode_start_delayed));
	m_mtx.unlock();
	return error::success;
}
/** TODO: STM32v1 controller doesn't support double buffer mode transfer
 * so it is not implemented */
/** Single Continous stop tranaction */
int stm32_dma_v1::continuous_start(channel& , mem_ptr ,
		mem_ptr , mem_ptr , size , dblbuf_dir )
{
	return error::not_supported;
}
int stm32_dma_v1::continous_stop(channel& /*chn*/)
{
	return error::not_supported;
}

/** Abort pending transaction */
int stm32_dma_v1::abort(channel& chn)
{
	isix::mutex_locker _lck(m_mtx);
	int chnf = get_handled_channel(chn);
	if(chnf<0) {
		dbg_warn("Channel %i is not active", chnf);
		return error::noent;
	}
	LL_DMA_DisableChannel(chn2cntrl(chnf),chn2hwchn(chnf));
	LL_DMA_DisableIT_TC(chn2cntrl(chnf),chn2hwchn(chnf));
	LL_DMA_DisableIT_TE(chn2cntrl(chnf),chn2hwchn(chnf));
	m_act_chns[chnf] = false;
	set_handled_channel(chn);
	return error::success;
}

/** Single start when non continous mode */
int stm32_dma_v1::single_start(channel& chn) noexcept
{
	int chnf = get_handled_channel(chn);
	if(chnf<0) {
		return error::noent;
	}
	const auto& cnf = channel_config(chn);
	if(!(cnf.flags&mode_start_delayed)) {
		return error::inval;
	}
	LL_DMA_EnableChannel(chn2cntrl(chnf),chn2hwchn(chnf));
	return error::success;
}

/** Find first unused channel slot */
int stm32_dma_v1::find_first(unsigned device, bool unused)
{
	if(device >= sizeof devid::detail::dev_chn_map) {
		return error::inval;
	}
	const auto mask = devid::detail::dev_chn_map[device];
	for(auto i=0U; i<nchns; ++i) {
		if(mask&(1U<<i)) {
			if(unused) {
				if(!m_act_chns[i]) return i;
			} else {
				return i;
			}
		}
	}
	return error::noent;
}

//! DMA configure
int stm32_dma_v1::dma_flags_configure(const detail::controller_config& cfg, detail::tmode mode, int chn)
{
	size_t tsize {1};
	//dbg_info("DMA input mode cnf %i channel %i", mode, chn);
	switch(mode) {
		case detail::tmode::mem2mem:
			LL_DMA_SetDataTransferDirection(chn2cntrl(chn), chn2hwchn(chn),
				LL_DMA_DIRECTION_MEMORY_TO_MEMORY );
			break;
		case detail::tmode::periph2mem:
			LL_DMA_SetDataTransferDirection(chn2cntrl(chn), chn2hwchn(chn),
				LL_DMA_DIRECTION_PERIPH_TO_MEMORY );
			break;
		case detail::tmode::mem2periph:
			LL_DMA_SetDataTransferDirection(chn2cntrl(chn), chn2hwchn(chn),
				LL_DMA_DIRECTION_MEMORY_TO_PERIPH );
			break;
		case detail::tmode::error:
			dbg_info("tmode::error");
			return error::inval;
	}
	if(mode==detail::tmode::periph2mem||mode==detail::tmode::mem2mem) {
		switch(cfg.flags & detail::mask::src_transfer_size) {
			case mode_src_size_byte:
				LL_DMA_SetPeriphSize(chn2cntrl(chn),chn2hwchn(chn), LL_DMA_PDATAALIGN_BYTE);
				break;
			case mode_src_size_halfword:
				LL_DMA_SetPeriphSize(chn2cntrl(chn),chn2hwchn(chn), LL_DMA_PDATAALIGN_HALFWORD);
				tsize=2;
				break;
			case mode_src_size_word:
				LL_DMA_SetPeriphSize(chn2cntrl(chn),chn2hwchn(chn), LL_DMA_PDATAALIGN_WORD);
				tsize=4;
				break;
		}
		switch(cfg.flags & detail::mask::dst_transfer_size) {
			case mode_dst_size_byte:
				LL_DMA_SetMemorySize(chn2cntrl(chn),chn2hwchn(chn), LL_DMA_MDATAALIGN_BYTE);
				break;
			case mode_dst_size_halfword:
				LL_DMA_SetMemorySize(chn2cntrl(chn),chn2hwchn(chn), LL_DMA_MDATAALIGN_HALFWORD);
				break;
			case mode_dst_size_word:
				LL_DMA_SetMemorySize(chn2cntrl(chn),chn2hwchn(chn), LL_DMA_MDATAALIGN_WORD);
				break;
		}
		if(cfg.flags & mode_src_inc) {
			LL_DMA_SetPeriphIncMode(chn2cntrl(chn),chn2hwchn(chn), LL_DMA_PERIPH_INCREMENT);
		} else {
			LL_DMA_SetPeriphIncMode(chn2cntrl(chn),chn2hwchn(chn), LL_DMA_PERIPH_NOINCREMENT);
		}
		if(cfg.flags & mode_dst_inc) {
			LL_DMA_SetMemoryIncMode(chn2cntrl(chn),chn2hwchn(chn), LL_DMA_MEMORY_INCREMENT);
		} else {
			LL_DMA_SetMemoryIncMode(chn2cntrl(chn),chn2hwchn(chn), LL_DMA_MEMORY_NOINCREMENT);
		}
	} else if(mode==detail::tmode::mem2periph) {
		switch(cfg.flags & detail::mask::dst_transfer_size) {
			case mode_dst_size_byte:
				LL_DMA_SetPeriphSize(chn2cntrl(chn),chn2hwchn(chn), LL_DMA_PDATAALIGN_BYTE);
				break;
			case mode_dst_size_halfword:
				LL_DMA_SetPeriphSize(chn2cntrl(chn),chn2hwchn(chn), LL_DMA_PDATAALIGN_HALFWORD);
				break;
			case mode_dst_size_word:
				LL_DMA_SetPeriphSize(chn2cntrl(chn),chn2hwchn(chn), LL_DMA_PDATAALIGN_WORD);
				break;
		}
		switch(cfg.flags & detail::mask::src_transfer_size) {
			case mode_src_size_byte:
				LL_DMA_SetMemorySize(chn2cntrl(chn),chn2hwchn(chn), LL_DMA_MDATAALIGN_BYTE);
				break;
			case mode_src_size_halfword:
				tsize=2;
				LL_DMA_SetMemorySize(chn2cntrl(chn),chn2hwchn(chn), LL_DMA_MDATAALIGN_HALFWORD);
				break;
			case mode_src_size_word:
				tsize=4;
				LL_DMA_SetMemorySize(chn2cntrl(chn),chn2hwchn(chn), LL_DMA_MDATAALIGN_WORD);
				break;
		}
		if(cfg.flags & mode_dst_inc) {
			LL_DMA_SetPeriphIncMode(chn2cntrl(chn),chn2hwchn(chn), LL_DMA_PERIPH_INCREMENT);
		} else {
			LL_DMA_SetPeriphIncMode(chn2cntrl(chn),chn2hwchn(chn), LL_DMA_PERIPH_NOINCREMENT);
		}
		if(cfg.flags & mode_src_inc) {
			LL_DMA_SetMemoryIncMode(chn2cntrl(chn),chn2hwchn(chn), LL_DMA_MEMORY_INCREMENT);
		} else {
			LL_DMA_SetMemoryIncMode(chn2cntrl(chn),chn2hwchn(chn), LL_DMA_MEMORY_NOINCREMENT);
		}
	}
	if(cfg.flags & mode_circural) {
		LL_DMA_SetMode(chn2cntrl(chn),chn2hwchn(chn),LL_DMA_MODE_CIRCULAR);
	} else {
		LL_DMA_SetMode(chn2cntrl(chn),chn2hwchn(chn),LL_DMA_MODE_NORMAL);
	}
	switch(cfg.flags & detail::mask::transfer_mode) {
		case priority_low:
			LL_DMA_SetChannelPriorityLevel(chn2cntrl(chn),chn2hwchn(chn), LL_DMA_PRIORITY_LOW);
			break;
		case priority_med:
			LL_DMA_SetChannelPriorityLevel(chn2cntrl(chn),chn2hwchn(chn), LL_DMA_PRIORITY_MEDIUM);
			break;
		case priority_hi:
			LL_DMA_SetChannelPriorityLevel(chn2cntrl(chn),chn2hwchn(chn), LL_DMA_PRIORITY_HIGH);
			break;
		case priority_vhi:
			LL_DMA_SetChannelPriorityLevel(chn2cntrl(chn),chn2hwchn(chn), LL_DMA_PRIORITY_VERYHIGH);
			break;
		default:
			dbg_err("Invalid dma priority mode");
			return error::inval;
	}
	//dbg_info("Set irq: %i prio: %i:%i", chn, cfg.irqh, cfg.irql);
	isix::set_irq_priority(chn, {uint8_t(cfg.irqh), uint8_t(cfg.irql)});
	const auto rp=isix::irq_priority_to_raw_priority({uint8_t(cfg.irqh), uint8_t(cfg.irql)});
	if(isix::get_raw_irq_priority(chn2irqn(chn))!=rp) {
		isix::set_raw_irq_priority(chn2irqn(chn), rp);
	}
	return tsize;
}


/** Configure dma address and speed addresses */
void stm32_dma_v1::dma_addr_configure(mem_ptr dest, cmem_ptr src,
		size ntrans, int chn, bool start)
{
	LL_DMA_ConfigAddresses(chn2cntrl(chn),chn2hwchn(chn),
		reinterpret_cast<uintptr_t>(src),reinterpret_cast<uintptr_t>(dest),
		LL_DMA_GetDataTransferDirection(chn2cntrl(chn),chn2hwchn(chn))
	);
	LL_DMA_SetDataLength(chn2cntrl(chn),chn2hwchn(chn),ntrans);
	LL_DMA_EnableIT_TC(chn2cntrl(chn),chn2hwchn(chn));
	LL_DMA_EnableIT_TE(chn2cntrl(chn),chn2hwchn(chn));
	if(start) {
		LL_DMA_EnableChannel(chn2cntrl(chn),chn2hwchn(chn));
	}
}

/** Remap alternative channel when needed */
void stm32_dma_v1::remap_alt_channel(chnid_t chn,int num)
{
	++num;	//! Base channel number is n+1
	if(chn==devid::mem) return;
	if( __builtin_popcount(devid::detail::dev_chn_map[chn])<=1) {
		return;
	}
	constexpr auto array_size=
		sizeof(devid::detail::remaping_table)/sizeof(devid::detail::remaping_table[0]);
	for(auto i=0U;i<array_size;++i) {
		const auto& it = devid::detail::remaping_table[i];
		if(it.devid==chn&&it.chn==num) {
			isix::enter_critical();
			SYSCFG->CFGR3 |= it.set_mask;
	        SYSCFG->CFGR3 &= ~it.clr_mask;
			isix::exit_critical();
			return;
		}
	}
}


} //periph::dma
