/*
 * =====================================================================================
 *
 *       Filename:  stm32_dma_v2.cpp
 *
 *    Description:  DMA controller version 2 for stm32 devices
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
#include <stm32f4xx_ll_dma.h>
#include <stm32f4xx_ll_bus.h>
#include <periph/dma/channel.hpp>
#include <periph/dma/dma_channels.hpp>
#include <isix/arch/irq.h>
#include <isix/arch/cache.h>
#include <foundation/sys/dbglog.h>
#include <periph/dma/dma_interrupt_handlers.hpp>

namespace {
	//! Convert stream id to dma controller
	inline auto strm2cntrl( int strm ) {
		return strm<=7?DMA1:DMA2;
	}
	//! Convert stream id to dma stream number
	inline constexpr auto strm2strm( int strm ) {
		return strm%8;
	}
	//! Convert stream id to interrupt controller
	inline auto constexpr strm2irqn( int strm ) {
		if( strm <= 7 ) strm+=DMA1_Stream0_IRQn;
		else  strm=strm-8+DMA2_Stream0_IRQn;
		return strm;
	}
	//! DMA controller flags
	enum class dmaflag {feif,_reserved_, dmeif,teif,htif,tcif};
	//! Check if flag is set
	inline bool dmaflag_isset(int chn, dmaflag fl) {
		static constexpr int index[] = { 0, 6, 16, 22 };
		static const volatile uint32_t* const sr[]
			={&DMA1->LISR,&DMA1->HISR,&DMA2->LISR,&DMA2->HISR};
		return (*sr[chn/4]&(1U<<(index[chn%4]+int(fl))))!=0;
	}
	//! Clear interrupt flag
	inline void dmaflag_clear(int chn, dmaflag fl) {
		static constexpr int index[] = { 0, 6, 16, 22 };
		static volatile uint32_t* const cr[]
			={&DMA1->LIFCR,&DMA1->HIFCR,&DMA2->LIFCR,&DMA2->HIFCR};
		*cr[chn/4] = 1U<<((index[chn%4]+int(fl)));
	}
	//! Translate channel number to phycical channel map
	inline decltype(LL_DMA_CHANNEL_0) chn2llchn(int chn) {
		if( chn > 7 || chn < 0) return 0;
		static constexpr decltype(LL_DMA_CHANNEL_0) chtab[] = {
			LL_DMA_CHANNEL_0, LL_DMA_CHANNEL_1, LL_DMA_CHANNEL_2,
			LL_DMA_CHANNEL_3, LL_DMA_CHANNEL_4, LL_DMA_CHANNEL_5,
			LL_DMA_CHANNEL_6, LL_DMA_CHANNEL_7
		};
		return chtab[chn];
	}
}

namespace periph::dma {
/** Controller create instance with stm32 version1 */
controller& controller::instance()
{
	static stm32_dma_v2 dmactrl;
	return std::ref(dmactrl);
}

// Default constructor
stm32_dma_v2::stm32_dma_v2()
{
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
#ifdef LL_AHB1_GRP1_PERIPH_DMA2
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2);
#endif
    for(int strm=0;strm<nchns;++strm)
		isix::request_irq(strm2irqn(strm));
}

//! Destructor
stm32_dma_v2::~stm32_dma_v2()
{
    for(int strm=0;strm<nchns;++strm)
		isix::free_irq(strm2irqn(strm));
  LL_AHB1_GRP1_DisableClock(LL_AHB1_GRP1_PERIPH_DMA1);
#ifdef LL_AHB1_GRP1_PERIPH_DMA2
  LL_AHB1_GRP1_DisableClock(LL_AHB1_GRP1_PERIPH_DMA2);
#endif
}

/** Find or wait for transmission */
std::tuple<int,int> stm32_dma_v2::find_empty_or_wait(unsigned device)
{
	int strm,chnm;
	for(;;)
	{
		std::tie(strm,chnm) = find_first(device,true);
		if(strm<0)
		{
			std::tie(strm,chnm) = find_first(device,false);
			if(strm<0)
			{
				dbg_err("Unable to find devmatching channel %i", strm);
				return std::make_tuple(strm,-1);
			}
			else {
				m_mtx.unlock();
				strm = wait();
				m_mtx.lock();
				if(strm<0) return std::make_tuple(strm,-1);
			}
		}
		else break;
	}
	return std::make_tuple(strm,chnm);
}

/** Single tranfer from controller */
int stm32_dma_v2::single(channel& chn, mem_ptr dest, cmem_ptr src, size len)
{
	int strm,chnm;
	m_mtx.lock();
	const auto& cnf = channel_config(chn);
	std::tie(strm,chnm) = find_empty_or_wait(cnf.dev_id);
	if(strm<0) {
		dbg_err("Slot error %i",strm);
		m_mtx.unlock();
		return strm;
	}
	//dbg_info("Stream %i, channel %i", strm, chnm);
	const auto tmode = detail::transfer_mode(dest,src);
	int res = dma_flags_configure(cnf,tmode,strm);
	if(res<0) {
		dbg_err("Dma flag configure error %i", strm);
		m_mtx.unlock();
		return res;
	}
	m_act_mode[strm] = bsy_mode::single;
	set_handled_channel(chn,strm);
	drivers::dma::_handlers::register_handler(strm, [this, strm, &chn]() {
		bool error {};
		if(dmaflag_isset(strm,dmaflag::teif)) {
			dmaflag_clear(strm,dmaflag::teif);
			channel_callback(chn, nullptr, true);
			m_act_mode[strm] = bsy_mode::idle;
			set_handled_channel(chn);
			broadcast_all();
			error=true;
		}
		if(dmaflag_isset(strm,dmaflag::tcif)) {
			dmaflag_clear(strm,dmaflag::tcif);
			if(!error) {
				channel_callback(chn, nullptr, false);
				m_act_mode[strm] = bsy_mode::idle;
				set_handled_channel(chn);
				broadcast_all();
			}
		}
		LL_DMA_DisableIT_TC(strm2cntrl(strm),strm2strm(strm));
		LL_DMA_DisableIT_TE(strm2cntrl(strm),strm2strm(strm));
		LL_DMA_DisableStream(strm2cntrl(strm),strm2strm(strm));
	});
	dma_addr_configure(dest,src,len/res,strm,tmode,chnm);
	m_mtx.unlock();
	return error::success;
}
/** Single Continous stop tranaction */
int stm32_dma_v2::continuous_start(channel& chn, mem_ptr mem0, mem_ptr mem1, size len)
{

	(void)mem0;
	(void)mem1;
	(void)len;
	(void)chn;
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


/** Find first unused channel slot */
std::tuple<int,int> stm32_dma_v2::find_first(unsigned device, bool unused)
{
	if(device >= sizeof(devid::detail::dev_chn_map)/sizeof(devid::detail::dev_chn_map[0])){
		return std::make_tuple(error::inval,-1);
	}
	const auto mask = devid::detail::dev_chn_map[device];
	for(auto i=0U; i<nchns; ++i) {
		if(mask.chn_msk()&(1U<<i)) {
			if(unused) {
				if(m_act_mode[i]==bsy_mode::idle)
					return std::make_tuple(i,mask.chn);
			} else {
				//! We can wait only for single xmit when wait for bsy
				if(m_act_mode[i]==bsy_mode::single)
					return std::make_tuple(i,mask.chn);
			}
		}
	}
	return std::make_tuple(error::noent,-1);
}
//! DMA configure
int stm32_dma_v2::dma_flags_configure(const detail::controller_config& cfg,
		detail::tmode mode, int strm)
{
	uint32_t config_transfer {};
	size_t tsize {1};
	switch(mode) {
		case detail::tmode::mem2mem:
			config_transfer|=LL_DMA_DIRECTION_MEMORY_TO_MEMORY;
			break;
		case detail::tmode::periph2mem:
			config_transfer|=LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
			break;
		case detail::tmode::mem2periph:
			config_transfer|=LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
			break;
		case detail::tmode::error:
			dbg_info("tmode::error");
			return error::inval;
	}
	if(mode==detail::tmode::periph2mem||mode==detail::tmode::mem2mem) {
		switch(cfg.flags & detail::mask::src_transfer_size) {
			case mode_src_size_byte:
				config_transfer|=LL_DMA_PDATAALIGN_BYTE;
				break;
			case mode_src_size_halfword:
				config_transfer|=LL_DMA_PDATAALIGN_HALFWORD;
				tsize=2;
				break;
			case mode_src_size_word:
				config_transfer|=LL_DMA_PDATAALIGN_WORD;
				tsize=4;
				break;
		}
		switch(cfg.flags & detail::mask::dst_transfer_size) {
			case mode_dst_size_byte:
				config_transfer|=LL_DMA_MDATAALIGN_BYTE;
				break;
			case mode_dst_size_halfword:
				config_transfer|=LL_DMA_MDATAALIGN_HALFWORD;
				break;
			case mode_dst_size_word:
				config_transfer|=LL_DMA_MDATAALIGN_WORD;
				break;
		}
		if(cfg.flags & mode_src_inc) {
			config_transfer|=LL_DMA_PERIPH_INCREMENT;
		} else {
			config_transfer|=LL_DMA_PERIPH_NOINCREMENT;
		}
		if(cfg.flags & mode_dst_inc) {
			config_transfer|=LL_DMA_MEMORY_INCREMENT;
		} else {
			config_transfer|=LL_DMA_MEMORY_NOINCREMENT;
		}
	} else if(mode==detail::tmode::mem2periph) {
		switch(cfg.flags & detail::mask::dst_transfer_size) {
			case mode_dst_size_byte:
				config_transfer|=LL_DMA_PDATAALIGN_BYTE;
				break;
			case mode_dst_size_halfword:
				config_transfer|=LL_DMA_PDATAALIGN_HALFWORD;
				break;
			case mode_dst_size_word:
				config_transfer|=LL_DMA_PDATAALIGN_WORD;
				break;
		}
		switch(cfg.flags & detail::mask::src_transfer_size) {
			case mode_src_size_byte:
				config_transfer|=LL_DMA_MDATAALIGN_BYTE;
				break;
			case mode_src_size_halfword:
				config_transfer|=LL_DMA_MDATAALIGN_HALFWORD;
				tsize=2;
				break;
			case mode_src_size_word:
				config_transfer|=LL_DMA_MDATAALIGN_WORD;
				tsize=4;
				break;
		}
		if(cfg.flags & mode_dst_inc) {
			config_transfer|=LL_DMA_PERIPH_INCREMENT;
		} else {
			config_transfer|=LL_DMA_PERIPH_NOINCREMENT;
		}
		if(cfg.flags & mode_src_inc) {
			config_transfer|=LL_DMA_MEMORY_INCREMENT;
		} else {
			config_transfer|=LL_DMA_MEMORY_NOINCREMENT;
		}
	}
	if(cfg.flags & mode_circural) {
		config_transfer|=LL_DMA_MODE_CIRCULAR;
	} else {
		config_transfer|=LL_DMA_MODE_NORMAL;
	}
	switch(cfg.flags & detail::mask::transfer_mode) {
		case priority_low:
			config_transfer|=LL_DMA_PRIORITY_LOW;
			break;
		case priority_med:
			config_transfer|=LL_DMA_PRIORITY_MEDIUM;
			break;
		case priority_hi:
			config_transfer|=LL_DMA_PRIORITY_HIGH;
			break;
		case priority_vhi:
			config_transfer|=LL_DMA_PRIORITY_VERYHIGH;
			break;
		default:
			dbg_err("Invalid dma priority mode");
			return error::inval;
	}
	LL_DMA_DeInit(strm2cntrl(strm),strm2strm(strm));
	LL_DMA_ConfigTransfer(strm2cntrl(strm),strm2strm(strm),config_transfer);
	LL_DMA_ConfigFifo(strm2cntrl(strm),strm2strm(strm),
			LL_DMA_FIFOMODE_ENABLE, LL_DMA_FIFOTHRESHOLD_1_2);
	LL_DMA_SetMemoryBurstxfer(strm2cntrl(strm),strm2strm(strm),LL_DMA_MBURST_SINGLE);
	LL_DMA_SetPeriphBurstxfer(strm2cntrl(strm),strm2strm(strm),LL_DMA_PBURST_SINGLE);
	//Configure interrupt
	const auto rp=isix::irq_priority_to_raw_priority({uint8_t(cfg.irqh), uint8_t(cfg.irql)});
	if(isix::get_raw_irq_priority(strm2irqn(strm))!=rp) {
		isix::set_raw_irq_priority(strm2irqn(strm), rp);
	}
	return tsize;
}

/** Configure dma address and speed addresses */
void stm32_dma_v2::dma_addr_configure(mem_ptr dest, cmem_ptr src, size ntrans,
		int strm, detail::tmode mode, int chns)
{
	if(mode==detail::tmode::periph2mem||mode==detail::tmode::mem2mem) {
		LL_DMA_SetMemoryAddress(strm2cntrl(strm),strm2strm(strm),
			reinterpret_cast<uintptr_t>(dest) );
		LL_DMA_SetPeriphAddress(strm2cntrl(strm),strm2strm(strm),
			reinterpret_cast<uintptr_t>(src) );
	} else if(mode==detail::tmode::mem2periph) {
		LL_DMA_SetMemoryAddress(strm2cntrl(strm),strm2strm(strm),
			reinterpret_cast<uintptr_t>(src) );
		LL_DMA_SetPeriphAddress(strm2cntrl(strm),strm2strm(strm),
			reinterpret_cast<uintptr_t>(dest) );
	}
	LL_DMA_SetDataLength(strm2cntrl(strm),strm2strm(strm),ntrans);
	LL_DMA_SetChannelSelection(strm2cntrl(strm),strm2strm(strm),chn2llchn(chns));
	LL_DMA_EnableIT_TC(strm2cntrl(strm),strm2strm(strm));
	LL_DMA_EnableIT_TE(strm2cntrl(strm),strm2strm(strm));
	LL_DMA_EnableStream(strm2cntrl(strm),strm2strm(strm));
}



}
