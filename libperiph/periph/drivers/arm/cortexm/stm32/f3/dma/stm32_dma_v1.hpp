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

namespace periph::dma {

	class stm32_dma_v1 final : public controller {
	public:
		/** Constructor */
		stm32_dma_v1() {
		}
		/** Destructor */
		virtual ~stm32_dma_v1() {
		}
		/** Single tranfer from controller */
		int single(channel& chn, mem_ptr dest, cmem_ptr src, size len) override;
		/** Single Continous stop tranaction */
		int continuous_start(channel& chn, mem_ptr mem0, mem_ptr mem1, size len) override;
		/** Continous stop transaction */
		int continous_stop(channel& chn) override;
		/** Abort pending transaction */
		int abort(channel& chn) override;
	};

}
