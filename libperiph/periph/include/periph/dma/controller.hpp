/*
 * =====================================================================================
 *
 *       Filename:  controller.hpp
 *
 *    Description:  DMA controller
 *
 *        Version:  1.0
 *        Created:  24.07.2018 21:13:32
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */


#pragma once

#include <memory>
#include "types.hpp"

namespace periph::dma {
	class channel;

	//! Setup dma controller class
	class controller
	{
		friend class channel;
	public:
		//! Instance the controller
		static controller& instance();
		using channel_ptr_t = std::unique_ptr<channel>;
		virtual ~controller() {}
		controller(controller&) = delete;
		controller& operator=(controller&) = delete;
		/** Create new dma channel for the device
		 *  @param[in] dev_id Device identifier
		 *  @param[in] flags  Dma flags
		 *  @param[in] irq_prio IRQ priority for dma handler
		 *  @return Allocated channel
		 */
		channel_ptr_t alloc_channel(chnid_t dev_id, flags_t flags, int irq_prio=-1);
		/** Release previously allocated channel
		 * @paam[in] chn Channel pointer
		 * @return Error code
		 */
		int release_channel(channel_ptr_t& chn);
	protected:
		//! Private constructor by factory method only
		controller() {}
	private:
		/** Single tranfer from controller */
		virtual int single(channel& chn, mem_ptr dest, cmem_ptr src, size len) = 0;
		/** Single Continous stop tranaction */
		virtual int continuous_start(channel& chn, mem_ptr mem0, mem_ptr mem1, size len) = 0;
		/** Continous stop transaction */
		virtual int continous_stop(channel& chn) = 0;
		/** Abort pending transaction */
		virtual int abort(channel& chn) = 0;
	};
}

