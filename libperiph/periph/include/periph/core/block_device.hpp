/*
 * =====================================================================================
 *
 *       Filename:  i2c_blk.hpp
 *
 *    Description:  I2C blk implementation
 *
 *        Version:  1.0
 *        Created:  06.03.2014 18:54:21
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once
#include <cstddef>
#include <periph/blk/transfer.hpp>
#include <periph/core/device.hpp>

namespace periph {


//! Hardware blk interface for all devices
class block_device	: public device
{
public:
	//! Non copyable
	block_device(block_device&) = delete;
	block_device& operator=(block_device&) = delete;
	//! Get blk type
	enum class type : unsigned char {
		i2c,
		spi,
	};
	//! Constructor
	explicit block_device(type blk_type, uintptr_t base_addr)
		: device(device::block_dev,base_addr), m_blk_type(blk_type)
	{
	}
	//! Destructor
	virtual ~block_device() {
	}
	/** Get blk type without rtti
	 */
	type blk_type() const noexcept {
		return m_blk_type;
	}
	/** Make blk specified transfer for single mode
	 * @parma[in] addr blk adddress
	 * @param[in] data transfer data in single mode
	 * @return error code for blk transfer type
	 */
	virtual int transaction(int addr, const blk::transfer& data) = 0;

	/** Make blk transfer using vectors and multiple transfers
	 * @param[in] addr blk address
	 * @param[in] items Transaction items
	 * @return error code
	 */
	virtual int transaction(int addr, const blk::transfer_chain& items)
	{
		int ret {};
		for( const auto& item: items ) {
			if((ret=transaction(addr,item))) break;
		}
		return ret;
	}
private:
	const type m_blk_type;
};

}

