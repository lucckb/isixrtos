
/*
 * virtual_eeprom.hpp
 *
 *  Created on: 04-01-2011
 *      Author: lucck
 */

#ifndef VIRTUAL_EEPROM_HPP_
#define VIRTUAL_EEPROM_HPP_

#include "iflash_mem.hpp"

namespace fnd
{


class virtual_eeprom
{
public:
	static const unsigned value_size = sizeof(unsigned);
	enum errno
	{
		ERRNO_OK,
		ERRNO_INIT = -7000,
		ERRNO_VA_RANGE = -7001,
		ERRNO_NO_SPACE = -7002,
		ERRNO_NOT_FOUND = -7003
	};
	virtual_eeprom(iflash_mem &_flash, unsigned flash_sector=0);
	int read(unsigned address, unsigned &value ) const;
	int write(unsigned address, unsigned value);
	unsigned get_max_index() const { return va_max; }
protected:
	virtual void lock() const {}
	virtual void unlock() const {}
private:
	int find_free_slot() const;
	int sort_flash();
private:
	iflash_mem &flash;
	iflash_mem::paddr_t active_page;
	iflash_mem::paddr_t inactive_page;
	const unsigned va_max;
};

}

#endif /* VIRTUAL_EEPROM_HPP_ */
