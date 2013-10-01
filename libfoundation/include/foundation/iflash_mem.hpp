/*
 * iflash_mem.hpp
 *
 *  Created on: 04-01-2011
 *      Author: lucck
 */

#ifndef IFLASH_MEM_HPP_
#define IFLASH_MEM_HPP_

/* ------------------------------------------------------------------ */
namespace fnd {
/* ------------------------------------------------------------------ */
class iflash_mem
{
public:
	//Page offset type
	typedef unsigned short poffs_t;
	//Page address type
	typedef unsigned short paddr_t;
	//Page len
	typedef unsigned short pglen_t;
	//Read one element from flash return bytes written or error
	virtual int read_halfword(paddr_t pg, poffs_t pa, unsigned short &val) const = 0;
	//Write one lement to flash return bytes written or error
	virtual int write_halfword(paddr_t pg, poffs_t pa, unsigned short val) = 0;
	//Read one element from flash return bytes written or error
	virtual int read_word(paddr_t pg, poffs_t pa, unsigned &val) const = 0;
	//Write one lement to flash return bytes written or error
	virtual int write_word(paddr_t pg, poffs_t pa, unsigned val) = 0;
	//Erase page return status
	virtual int page_erase(paddr_t pa) = 0;
	//Get pagesize
	virtual poffs_t page_size() const = 0;
	//Get numpages
	virtual paddr_t num_pages() const = 0;
};

/* ------------------------------------------------------------------ */
}
/* ------------------------------------------------------------------ */
#endif /* IFLASH_MEM_HPP_ */
