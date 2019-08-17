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
#include <fstream>
  
namespace fnd {
  
//! I2C serial eeprom interface
class fs_eeprom : public iflash_mem {
public:
	/** @param[in] bus Input bus owner
	 *  @param[in] bus_addr Bus memory address
	 *  @param[in] dev_type Device type
	 *  @param[in] emulate_flash Emulate flash memory
	 *  @param[in] filename Filename used for testing 
	 */
	fs_eeprom( int n_pages, int page_size, bool emulate_flash, const char* filename );
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
	virtual ~fs_eeprom() {
	}
	//Get capabilties bits
	virtual int get_capabilities() const {
		return m_emulate_flash?0:cap_pg_no_erase;
	}
	//Get pagesize
	virtual poffs_t page_size() const {
		return m_page_size;
	}
	//Get numpages
	virtual paddr_t num_pages() const {
		return m_num_pages;
	}
	//Erase page return status
	virtual int page_erase( paddr_t pa );
private:
	void check_range( paddr_t pg, poffs_t pa, size_t len ) const;
private:
	const poffs_t m_page_size;
	const paddr_t m_num_pages;
	mutable std::fstream m_file; 
	const bool m_emulate_flash;
};
  
}

