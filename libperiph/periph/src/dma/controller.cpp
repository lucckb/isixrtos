/*
 * =====================================================================================
 *
 *       Filename:  controller.cpp
 *
 *    Description:  DMA controller implementation
 *
 *        Version:  1.0
 *        Created:  10.09.2018 20:44:04
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include <periph/dma/controller.hpp>
#include <periph/dma/channel.hpp>
#include <periph/core/error.hpp>
#include <foundation/sys/dbglog.h>


namespace periph::dma {

//!Inline member for access controller data
void* controller::channel_callback(channel& chn,mem_ptr mem) noexcept {
	if(auto pval = std::get_if<asyncdbl_callback>(&chn.m_cb) )
		return (*pval)?(*pval)(mem):nullptr;
	else
		return nullptr;
}

//!Inline member for access controller data
void controller::channel_callback(channel& chn,bool te) noexcept {
	if(auto pval = std::get_if<async_callback>(&chn.m_cb) )
		if(*pval) (*pval)(te);
}

//!Inline member for access controller data
const detail::controller_config& controller::channel_config(channel& chn) {
	return std::ref(chn.m_conf);
}

//Allocate and initialize channel
controller::channel_ptr_t
	controller::alloc_channel(chnid_t dev_id, flags_t flags, irq_t irqh, irq_t irql)
{
	auto ret = std::make_unique<channel>(std::ref(*this),dev_id,flags,irqh,irql);
	open_channel(std::ref(*ret));
	return ret;
}

//! Set handled channel
void controller::set_handled_channel(channel& chn, chnid_t chnid) noexcept
{
	chn.m_chn_id = chnid;
}

//! Get handled channel
chnid_t controller::get_handled_channel(channel& chn) noexcept
{
	return chn.m_chn_id;
}

//Release controller channel
int controller::release_channel(channel_ptr_t& chn)
{
	if(chn->busy()) {
		return error::busy;
	}
	close_channel(std::ref(*chn));
	chn.reset();
	return error::success;
}

}
