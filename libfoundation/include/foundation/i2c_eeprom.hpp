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
#include <foundation/iflash_mem.hpp>
#include <foundation/ibus.hpp>
/* ------------------------------------------------------------------ */ 
namespace fnd {
/* ------------------------------------------------------------------ */ 
//! I2C serial eeprom interface
class i2c_eeprom : public iflash_mem {
public:
	//! Memory type
	enum class type : unsigned char {
		m24c16
	};
	/** @param[in] bus Input bus owner
	 *  @param[in] bus_addr Bus memory address
	 *  @param[in] dev_type Device type
	 */
	i2c_eeprom( fnd::bus::ibus &bus, unsigned bus_addr ,type dev_type );
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
	virtual ~i2c_eeprom();
	//Get capabilties bits
	virtual int get_capabilities() const {
		return cap_pg_no_erase;
	}
	//Get pagesize
	virtual poffs_t page_size() const = 0;
	//Get numpages
	virtual paddr_t num_pages() const = 0;
private:
	fnd::bus::ibus& m_bus;			//! Bus controller
	const unsigned char m_addr;		//! Memory base addres
	const type m_type;				//! Memory type
};
/* ------------------------------------------------------------------ */ 
}

