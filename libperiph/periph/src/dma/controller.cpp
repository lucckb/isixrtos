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


namespace periph::dma {

//!Inline member for access controller data
void controller::channel_callback(channel& chn,mem_ptr mem) noexcept {
	if(chn.m_cb) chn.m_cb(mem);
}

//!Inline member for access controller data
const detail::controller_config& controller::channel_config(channel& chn) {
	return std::ref(chn.m_conf);
}

//Allocate and initialize channel
controller::channel_ptr_t
	controller::alloc_channel(chnid_t dev_id, flags_t flags, int irq_prio)
{
	return std::make_unique<channel>(std::ref(*this),dev_id,flags,irq_prio);
}

}
