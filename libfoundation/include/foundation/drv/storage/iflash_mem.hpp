/*
 * iflash_mem.hpp
 *
 *  Created on: 04-01-2011
 *      Author: lucck
 */

#pragma once

#include <cstddef>

namespace fnd {

class iflash_mem
{
public:
	//! Noncopyable
	iflash_mem( iflash_mem& ) = delete;
	iflash_mem& operator=( iflash_mem& ) = delete;
	iflash_mem() {

	}
	//! Virtual destructor
	virtual ~iflash_mem() {

	}
	static constexpr auto err_not_supported = -256;
	//! Capability bits
	enum cap_bits {
		cap_pg_no_erase = 0x01,	//! Can write page without erase
	};
	//Page offset type
	typedef unsigned int poffs_t;
	//Page address type
	typedef unsigned int paddr_t;
	//Page len
	typedef unsigned int pglen_t;
	// Sector size
	typedef unsigned int sectoffs_t;
	//Read one element from flash return bytes written or error
	virtual int read_halfword(paddr_t /*pg*/, poffs_t /*pa*/, unsigned short &/*val*/) const
	{
		return err_not_supported;
	}
	//Write one lement to flash return bytes written or error
	virtual int write_halfword(paddr_t /*pg*/, poffs_t /*pa*/, unsigned short /*val*/)
	{
		return err_not_supported;
	}
	//Read one element from flash return bytes written or error
	virtual int read_word(paddr_t /*pg*/, poffs_t /*pa*/, unsigned &/*val*/) const
	{
		return err_not_supported;
	}
	//Write one lement to flash return bytes written or error
	virtual int write_word(paddr_t /*pg*/, poffs_t /*pa*/, unsigned /*val*/)
	{
		return err_not_supported;
	}
	/** Read data from selected address
	 * @param[in] pg Page address
	 * @param[in] pa Page offset
	 * @param[in] ptr Pointer to write
	 * @param[in] len Size of buffer
	 * @return error code or 0 if success */
	virtual int write( paddr_t /*pg*/, poffs_t /*pa*/,const void* /*ptr*/, size_t /*len*/ ) {
		return err_not_supported;
	}
	/** Read data from selected address
	 * @param[in] pg Page address
	 * @param[in] pa Page offset
	 * @param[out] ptr Pointer to read
	 * @param[in] len Size of buffer
	 * @return error code or 0 if success
	 * */
	virtual int read( paddr_t /*pg*/, poffs_t /*pa*/,void* /*ptr*/ , size_t /*len*/) const {
		return err_not_supported;
	}
	//Erase page return status
	virtual int page_erase(paddr_t /* pa */) {
		return err_not_supported;
	}
	//Get capabilties bits
	virtual int get_capabilities() const {
		return err_not_supported;
	}
	//Get pagesize
	virtual poffs_t page_size() const = 0;
	//Get numpages
	virtual paddr_t num_pages() const = 0;
	//Get sector size
	virtual sectoffs_t sector_size() const = 0;
};


}

