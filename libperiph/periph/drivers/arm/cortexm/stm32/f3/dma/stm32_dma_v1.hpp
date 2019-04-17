/*
 * =====================================================================================
 *
 *       Filename:  stm32_dma_v1.hpp
 *
 *    Description:  STM32 DMA controller simple version 1
 *
 *        Version:  1.0
 *        Created:  09/10/2018 15:59:52
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once

#include <periph/dma/controller.hpp>
#include <isix/mutex.h>
#include <isix/cpp/mutex.hpp>
#include <isix/semaphore.h>
#include <isix/cpp/semaphore.hpp>
#include <atomic>

namespace periph::dma {

	class stm32_dma_v1 final : public controller {
	public:
		static constexpr auto nchns = 7U;
		static constexpr auto wait_timeout = 5000U;
		/** Constructor */
		stm32_dma_v1();
		/** Destructor */
		virtual ~stm32_dma_v1();
	private:
		enum class tmode {
			error,
			mem2mem,
			periph2mem,
			mem2periph
		};
		/** Determine transfer mode */
		static tmode transfer_mode(cmem_ptr dst, cmem_ptr src);
		/** Single tranfer from controller */
		int single(channel& chn, mem_ptr dest, cmem_ptr src, size len) override;
		/** Single Continous stop tranaction */
		int continuous_start(channel& chn, mem_ptr mem0, mem_ptr mem1, size len) override;
		/** Continous stop transaction */
		int continous_stop(channel& chn) override;
		/** Abort pending transaction */
		int abort(channel& chn) override;
		/** Find first unused channel slot */
		int find_first(unsigned device, bool unused);
		/** Configure interrupt and DMA according to flags */
		int dma_flags_configure(const detail::controller_config& cfg, tmode mode, int chn);
		/** Configure dma address and speed addresses */
		void dma_addr_configure(mem_ptr dest, cmem_ptr src, size ntrans, int chn);
		void remap_alt_channel(chnid_t chn,int num);
		void broadcast_all() {
			m_brodcast.reset_isr(0);
		}
		int wait() {
			return m_brodcast.wait(wait_timeout);
		}
	private:
		std::atomic<bool> m_act_chns[nchns] {};
		isix::mutex m_mtx;
		isix::semaphore m_brodcast { 0 };
	};
}
