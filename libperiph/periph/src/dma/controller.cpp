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
void controller::channel_callback(channel& chn,mem_ptr mem, bool te) noexcept {
	if(chn.m_cb) chn.m_cb(mem,te);
}

//!Inline member for access controller data
const detail::controller_config& controller::channel_config(channel& chn) {
	return std::ref(chn.m_conf);
}

//Allocate and initialize channel
controller::channel_ptr_t
	controller::alloc_channel(chnid_t dev_id, flags_t flags, irq_t irqh, irq_t irql)
{
	return std::make_unique<channel>(std::ref(*this),dev_id,flags,irqh,irql);
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
	chn.reset();
	return error::success;
}

}
