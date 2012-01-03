/* ------------------------------------------------------------------ */
/*
 * stm32fmc.hpp
 *
 *  Created on: 04-01-2011
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#ifndef STM32FMC_HPP_
#define STM32FMC_HPP_
/* ------------------------------------------------------------------ */
#include "iflash_mem.hpp"

/* ------------------------------------------------------------------ */
namespace stm32
{
/* ------------------------------------------------------------------ */
class stm32fmc: public fnd::iflash_mem
{
public:
	enum errno
	{
	  ERRNO_COMPLETE = 0,
	  ERRNO_BUSY = -1,
	  ERRNO_PG = -2,
	  ERRNO_WRP = -3,
	  ERRNO_TIMEOUT = -4
	};
public:
	//Default constructor
	stm32fmc();
	//Virtual destructor
	virtual ~stm32fmc();
	//Get pagesize
	virtual poffs_t page_size() const;
	//Get numpages
	virtual paddr_t num_pages() const;
	//Erase page return status
	virtual int page_erase(paddr_t pa);
	//Write half word into flash
	virtual int write_halfword(paddr_t pg, poffs_t pa, unsigned short val);
	//Read halfword
	virtual int read_halfword(paddr_t pg, poffs_t pa, unsigned short &val) const;
	//Read one element from flash return bytes written or error
	virtual int read_word(paddr_t pg, poffs_t pa, unsigned &val) const;
	//Write one lement to flash return bytes written or error
	virtual int write_word(paddr_t pg, poffs_t pa, unsigned val);
private:
	errno wait_for_last_operation(unsigned timeout);
	errno get_status() const;
};
/* ------------------------------------------------------------------ */
}
/* ------------------------------------------------------------------ */
#endif /* STM32FMC_HPP_ */
/* ------------------------------------------------------------------ */