/*
 * =====================================================================================
 *
 *       Filename:  i2c_eeprom.cpp
 *
 *    Description:  I2C eeprom generic implementation
 *
 *        Version:  1.0
 *        Created:  06.03.2014 19:41:05
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include <foundation/drv/storage/i2c_eeprom.hpp>
#include <cstdint>

namespace fnd {

constexpr iflash_mem::poffs_t i2c_eeprom::_page_size_table[];
constexpr iflash_mem::paddr_t i2c_eeprom::_page_count_table[];

namespace {
	 #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		inline uint16_t addr_order( uint16_t val ) {
			return __builtin_bswap16( val );
		}
	 #else
		inline uint16_t addr_order( uint16_t val ) {
			return val;
		}
	 #endif
}

/** @param[in] bus Input bus owner
*  @param[in] bus_addr Bus memory address
*  @param[in] dev_type Device type
*/
i2c_eeprom::i2c_eeprom( fnd::drv::bus::ibus &bus, unsigned bus_addr ,type dev_type )
	: m_bus(bus), m_addr( bus_addr ), m_type( dev_type )
{
}

/** Read data from selected address 
	* @param[in] pg Page address
	* @param[in] pa Page offset
	* @param[in] ptr Pointer to write
	* @param[in] len Size of buffer
	* @return error code or 0 if success */
int i2c_eeprom::write( paddr_t pg, poffs_t pa, const void* ptr, size_t len )
{
	if ( pa + len > pg_size() ) {
		return err_addr_range;
	}
	if( pg  > pg_count() ) {
		return err_addr_range;
	}
	const unsigned offset = pg * pg_size() + pa;
	int ret = err_invalid_type;
	for( int retry=0; retry<5; ++retry ) {
		if( m_type <= type::m24c16 ) {
			uint8_t i2c_a = ((offset >> 8)<<1) + m_addr;
			uint8_t addr = offset;
			ret  = m_bus.write( i2c_a, &addr, sizeof addr, ptr, len );
		} else {
			uint16_t addr = addr_order(offset);
			ret = m_bus.write( m_addr, &addr, sizeof addr, ptr, len );
		}
		//!Busy wait for write op
		if( ret == drv::bus::ibus::err_ack_failure ) {
			m_bus.mdelay( 2 );
		} else {
			break;
		}
	}
	return ret;
}

/** Read data from selected address 
	* @param[in] pg Page address
	* @param[in] pa Page offset
	* @param[out] ptr Pointer to read
	* @param[in] len Size of buffer
	* @return error code or 0 if success
	* */
int i2c_eeprom::read( paddr_t pg, poffs_t pa, void* ptr, size_t len ) const
{
	if ( pa + len > pg_size() ) {
		return err_addr_range;
	}
	if( pg  > pg_count() ) {
		return err_addr_range;
	}
	const unsigned offset = pg * pg_size() + pa;
	int ret = err_invalid_type;
	for( int retry=0; retry<5; ++retry ) {
		if( m_type <= type::m24c16 ) {
			uint8_t i2c_a = ((offset >> 8)<<1) + m_addr;
			uint8_t addr = offset;
			ret = m_bus.transfer( i2c_a, &addr, sizeof addr, ptr, len );
		} else {
			uint16_t addr = addr_order(offset);
			ret = m_bus.transfer( m_addr, &addr, sizeof addr, ptr, len );
		}
		//!Busy wait for write op
		if( ret == drv::bus::ibus::err_ack_failure ) {
			m_bus.mdelay( 2 );
		} else {
			break;
		}
	}
	return ret;
}

}

