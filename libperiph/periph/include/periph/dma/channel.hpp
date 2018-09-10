/*
 * =====================================================================================
 *
 *       Filename:  channel.hpp
 *
 *    Description:  DMA channel function
 *
 *        Version:  1.0
 *        Created:  23.07.2018 18:56:54
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once
#include <foundation/algo/fixed_size_function.hpp>
#include "types.hpp"
#include "controller.hpp"

namespace periph::dma {

namespace detail {
	constexpr auto max_bounded_args = 8;
	struct controller_config {
		controller_config(chnid_t id, flags_t fl, int irqp)
			: dev_id(id),flags(fl),irq_prio(irqp) {
			}
		chnid_t dev_id;
		flags_t flags;
		int irq_prio;
	};

}

	//! Dma flags
// Flags
	enum dev_mode : flags_t {
		mode_memory_to_memory = 0,
		mode_peripheral_to_memory = 1,
		mode_memory_to_peripheral = 2,
	};

	enum src_transfer_size : flags_t {
		mode_src_size_byte = 0 << 3,
		mode_src_size_halfword = 1 << 3,
		mode_src_size_word = 2 << 3,
	};


	enum dest_transfer_size : flags_t {
		mode_dst_size_byte = 0 << 6,
		mode_dst_size_halfword = 1 << 6,
		mode_dst_size_word = 2 << 6,
	};

	enum transfer_mode : flags_t {
		mode_single = 0 << 9,
		mode_circural = 1 << 9,
	};


	/** Don't use std function it must be quite fast
	 * void* pointer to the memory which can be filled in double
	 * buffer mode
	 */
	//using async_callback = fnd::estd::function<void(mem_ptr),detail::max_bounded_args*sizeof(size_t),fnd::estd::function_construct_t::copy_and_move>;

	using async_callback = std::function<void(mem_ptr)>;

	//Open dma channel
	class channel
	{
		friend class controller;
	public:
	    /** @param[in] device_id Device identifer
	     * @param[in] flags DMA operation flags
		 * @param[in] irq_prio interrupt devfaul priority for handle
		 */
		channel( controller& owner, chnid_t dev_id, flags_t flags, int irq_prio = -1)
			: m_owner(owner), m_conf(dev_id, flags, irq_prio)
		{}
		//! Destructor
		~channel() {
			m_owner.abort(std::ref(*this));
		}
		channel(channel&) = delete;
		channel& operator=(channel&) = delete;

		/**  DMA set callback
		* @param[in] cb Transfer callback
		* @return dma error
		*/
		void callback( async_callback& cb ) noexcept {
			m_cb = cb;
		}
		/** Start the single DMA transfer on selected dma_handle
			* @param[in] DMA api handle
			* @param[in] dest Destination address
			* @param[in] src Source transfer address
			* @param[in] size Transfer size in bytes
		*/
		int single( mem_ptr dest, cmem_ptr src, size len ) {
			return m_owner.single(std::ref(*this),dest,src,len);
		}

		/** Start the double bufer continous DMA transfer
		* The mem0 and mem1 buffers will be swaped
		*  by DMA hardware during transfer. When transfer will be finished the free buffer
		*  will be passed to the callback or returned form the function
		*  @param[in] handle DMA handle
		*  @param mem0 First memory buffer
		*  @param mem1 Second memory buffer
		*  @param periph Peripheral address
		*  @return DMA free swapped out buffer
		*  */
		int continuous_start( mem_ptr mem0, mem_ptr mem1, size len ) {
			return m_owner.continuous_start(std::ref(*this), mem0, mem1, len);
		}

		/** Stop the double transfer mode
		 * @return error code */
		int continous_stop() {
			return m_owner.continous_stop(std::ref(*this));
		}

	private:
		/* data */
		controller& m_owner;
		const detail::controller_config m_conf;
		async_callback m_cb;
	};

}


