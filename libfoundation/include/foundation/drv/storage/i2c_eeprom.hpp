/*
 * =====================================================================================
 *
 *       Filename:  i2c_serial_eeprom.hpp
 *
 *    Description:  I2C serial eeprom interface support
 *
 *        Version:  1.0
 *        Created:  06.03.2014 19:19:45
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once
#include <foundation/drv/storage/iflash_mem.hpp>
#include <foundation/drv/bus/ibus.hpp>
  
namespace fnd {
  
//! I2C serial eeprom interface
class i2c_eeprom : public iflash_mem {
public:
	//! Memory type
	enum class type : unsigned char {
		m24c16,		//! 24c16 type 
		m24c128		//! 24c128 type
	};
	//! Error code
	enum error {
		err_addr_range = -2048,
		err_invalid_type = -2049,
		err_invalid_len = -2050
	};
	/** @param[in] bus Input bus owner
	 *  @param[in] bus_addr Bus memory address
	 *  @param[in] dev_type Device type
	 */
	i2c_eeprom( fnd::drv::bus::ibus &bus, unsigned bus_addr ,type dev_type );
	/** Read data from selected address 
	 * @param[in] pg Page address
	 * @param[in] pa Page offset
	 * @param[in] ptr Pointer to write
	 * @param[in] len Size of buffer
	 * @return error code or 0 if success */
	virtual int write( paddr_t pg, poffs_t pa ,const void* ptr , size_t len );
	/** Read data from selected address 
	 * @param[in] pg Page address
	 * @param[in] pa Page offset
	 * @param[out] ptr Pointer to read
	 * @param[in] len Size of buffer
	 * @return error code or 0 if success
	 * */
	virtual int read( paddr_t pg, poffs_t pa, void* ptr, size_t len ) const;
	//! Destructor
	virtual ~i2c_eeprom() {
	}
	//Get capabilties bits
	int get_capabilities() const override {
		return cap_pg_no_erase;
	}
	//Get pagesize
	poffs_t page_size() const override {
		return pg_size();
	}
	//Get numpages
	paddr_t num_pages() const override {
		return pg_count();
	}
	//! Get sector size default is 1
	sectoffs_t sector_size() const override {
		return 1;
	}
private:
	fnd::drv::bus::ibus& m_bus;			//! Bus controller
	const unsigned char m_addr;		//! Memory base addres
	const type m_type;				//! Memory type
private:
	static constexpr iflash_mem::poffs_t _page_size_table[] = { 16, 64 };
	static constexpr iflash_mem::paddr_t _page_count_table[] = { 128, 256 };
	paddr_t pg_size( ) const {
		return _page_size_table[int(m_type)];
	}
	poffs_t pg_count( ) const {
		return _page_count_table[int(m_type)];
	}
};
  
}

