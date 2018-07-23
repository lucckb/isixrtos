/*
 * =====================================================================================
 *
 *       Filename:  i2c_bus.hpp
 *
 *    Description:  I2C bus implementation
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
#include <periph/bus/transfer.hpp>

namespace periph {
namespace bus {

//! Hardware bus interface for all devices
class ibus
{
public:
	//! Non copyable
	ibus(ibus&) = delete;
	ibus& operator=(ibus&) = delete;
	//! Get bus type
	enum class type : unsigned char {
		i2c,
		spi,
	};
	//! Constructor
	explicit ibus(type bus_type)
		: m_bus_type(bus_type)
	{
	}
	//! Destructor
	virtual ~ibus() {
	}
	/** Get bus type without rtti
	 */
	type bus_type() const noexcept {
		return m_bus_type;
	}
	/** Make bus specified transfer for single mode
	 * @parma[in] addr bus adddress
	 * @param[in] data transfer data in single mode
	 * @return error code for bus transfer type
	 */
	virtual int transaction(int addr, const transfer& data) = 0;

	/** Make bus transfer using vectors and multiple transfers
	 * @param[in] addr Bus address
	 * @param[in] items Transaction items
	 * @return error code
	 */
	template <typename T>
		int transaction( int addr, T items )
	{
		int ret {};
		for( const auto& item: items ) {
			if((ret=transaction(addr,item)))
				break;
		}
		return ret;
	}
private:
	type m_bus_type;
};


} }

