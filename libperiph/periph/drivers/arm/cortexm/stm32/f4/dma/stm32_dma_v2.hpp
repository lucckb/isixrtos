/*
 * =====================================================================================
 *
 *       Filename:  stm32_dma_v2.hpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  15.04.2019 13:58:35
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
#include <atomic>
#include <tuple>
#include <isix/mutex.h>
#include <isix/cpp/mutex.hpp>
#include <isix/semaphore.h>
#include <isix/cpp/semaphore.hpp>
#include <periph/dma/dma_common.hpp>

namespace periph::dma {

	class stm32_dma_v2 final : public controller {
		static constexpr auto nchns = 16;
		static constexpr auto wait_timeout = 5000U;
		enum class bsy_mode : char {
			idle,
			single,
			continous
		};
	public:
		//! Constructors and destructors
		stm32_dma_v2();
		virtual ~stm32_dma_v2();
		stm32_dma_v2(const stm32_dma_v2&) = delete;
		stm32_dma_v2& operator=(const stm32_dma_v2& ) = delete;
	private:
		/** Single tranfer from controller */
		int single(channel& chn, mem_ptr dest, cmem_ptr src, size len) override;
		/** Single Continous stop tranaction */
		int continuous_start(channel& chn, mem_ptr mem0,
				mem_ptr mem1, mem_ptr periph, size len, dblbuf_dir dir) override;
		/** Single start when non continous mode */
		int single_start(channel& chn) noexcept override;
		/** Abort pending transaction */
		int abort(channel& chn) override;
		/** Find first unused channel slot */
		std::tuple<int,int> find_first(unsigned device, bool unused);
		/** Find or wait for transmission */
		std::tuple<int,int> find_empty_or_wait(unsigned device);
		/** Configure interrupt and DMA according to flags */
		int dma_flags_configure(const detail::controller_config& cfg,
				detail::tmode mode, int strm);
		/** Configure dma address and speed addresses */
		void dma_addr_configure(mem_ptr dest, cmem_ptr src, size ntrans,
				int strm, detail::tmode mode, int chns, bool start);
		// Double buffer mode address
		void dma_addr_configure(mem_ptr mem0, mem_ptr mem1, mem_ptr periph,
				size ntrans, int strm, int chns);
		void broadcast_all() {
			m_brodcast.reset_isr(0);
		}
		int wait() {
			return m_brodcast.wait(wait_timeout);
		}
	private:
		std::atomic<bsy_mode> m_act_mode[nchns] {};
		isix::mutex m_mtx;
		isix::semaphore m_brodcast { 0 };
	};

}
