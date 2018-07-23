/*
 * =====================================================================================
 *
 *       Filename:  dma.hpp
 *
 *    Description:  DMA architecture indenpendent driver
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

namespace periph {
namespace dma {

namespace detail {
	constexpr auto max_bounded_args = 8;
}

	//! Dma flags
// Flags
	enum dev_mode {
		mode_memory_to_memory = 0,
		mode_peripheral_to_memory = 1,
		mode_memory_to_peripheral = 2,
	};

	enum src_transfer_size {
		mode_src_size_byte = 0 << 3,
		mode_src_size_halfword = 1 << 3,
		mode_src_size_word = 2 << 3,
	};


	enum dest_transfer_size {
		mode_dst_size_byte = 0 << 6,
		mode_dst_size_halfword = 1 << 6,
		mode_dst_size_word = 2 << 6,
	};

	enum transfer_mode {
		mode_single = 0 << 9,
		mode_circural = 1 << 9,
	};


	/** Don't use std function it must be quite fast
	 * void* pointer to the memory which can be filled in double
	 * buffer mode
	 */
	using async_callback = fnd::estd::function<void(void*),detail::max_bounded_args*sizeof(size_t),fnd::estd::function_construct_t::copy_and_move>;



	//Open dma channel
	class channel
	{
	public:
	    /** @param[in] device_id Device identifer
	     * @param[in] flags DMA operation flags
		 * @param[in] irq_prio interrupt devfaul priority for handle
		 */
		channel(uintptr_t dev_id, int flags, int irq_prio = -1)
			: m_dev_id(dev_id), m_flags(flags), m_irq_prio(irq_prio)
		{}
		~channel() {}
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
		int single( void* dest, const void* src, std::size_t len );

		/** Start the double bufer continous DMA transfer. The mem0 and mem1 buffers will be swaped
		*  by DMA hardware during transfer. When transfer will be finished the free buffer
		*  will be passed to the callback or returned form the function
		*  @param[in] handle DMA handle
		*  @param mem0 First memory buffer
		*  @param mem1 Second memory buffer
		*  @param periph Peripheral address
		*  @return DMA free swapped out buffer
		*  */
		int continuous_start( void* mem0, void* mem1, std::size_t len );


		/** Stop the double transfer mode
		 * @return error code */
		int continous_stop();

	private:
		/* data */
		const uintptr_t m_dev_id;
		const int m_flags;
		const int m_irq_prio;
		async_callback m_cb;
	};

}}


