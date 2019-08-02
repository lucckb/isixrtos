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
	namespace detail {
		struct controller_config;
	}

	//! Setup dma controller class
	class controller
	{
		friend class channel;
	public:
		using irq_t = unsigned short;
		//! Instance the controller
		static controller& instance();
		using channel_ptr_t = std::unique_ptr<channel>;
		virtual ~controller() {}
		controller(controller&) = delete;
		controller& operator=(controller&) = delete;
		/** Create new dma channel for the device
		 *  @param[in] dev_id Device identifier
		 *  @param[in] flags Dma flags
		 *  @param[in] irq_prio IRQ priority for dma handler
		 *  @param
		 *  @return Allocated channel
		 */
		channel_ptr_t alloc_channel(chnid_t dev_id, flags_t flags,irq_t irqh=1, irq_t irql=7);
		/** Release previously allocated channel
		 * @paam[in] chn Channel pointer
		 * @return Error code
		 */
		int release_channel(channel_ptr_t& chn);
	protected:
		//! Private constructor by factory method only
		controller() {}
		//! Channel callback
		static void* channel_callback(channel& chn,mem_ptr mem) noexcept;
		//! Channel callback v2
		static void channel_callback(channel& chn, bool te) noexcept;
		//! Config callback
		static const detail::controller_config& channel_config(channel& chn);
		//! Configure channel id
		static void set_handled_channel(channel& chn, chnid_t chnid=-1) noexcept;
		static chnid_t get_handled_channel(channel& chn) noexcept;
	private:
		/** Single tranfer from controller */
		virtual int single(channel& chn, mem_ptr dest, cmem_ptr src, size len) = 0;
		/** Single Continous stop tranaction */
		virtual int continuous_start(channel& chn, mem_ptr mem0,
				mem_ptr mem1, mem_ptr periph, size len, dblbuf_dir dir) = 0;
		/** Single start when non continous mode */
		virtual int single_start(channel& chn) noexcept;
		/** Continous stop transaction */
		virtual int continous_stop(channel& chn) {
			return abort(chn);
		}
		/** Abort pending transaction */
		virtual int abort(channel& chn) = 0;
		//** Preconfigure channel when it is open */
		virtual void open_channel(channel& /*chn*/) {
		}
		//** Preconfigure channel when it is closed */
		virtual void close_channel(channel& /*chn*/) {
		}
	};
}

