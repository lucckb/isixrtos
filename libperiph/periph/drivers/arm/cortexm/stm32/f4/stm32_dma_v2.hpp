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

namespace periph::dma {

class stm32_dma_v2 final : public controller {

	private:
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
