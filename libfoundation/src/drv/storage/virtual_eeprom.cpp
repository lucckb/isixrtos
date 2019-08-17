/*
 * virtual_eeprom.cpp
 *
 *  Created on: 04-01-2011
 *      Author: lucck
 */
 
#include "foundation/drv/storage/virtual_eeprom.hpp"
#include "foundation/drv/storage/iflash_mem.hpp"
#include <stdint.h>
#include <cstddef>

 
namespace fnd
{

 
//Unnamed namespace
namespace
{
	struct eeitem
	{
		uint16_t vaddr;
		uint32_t value;
	} __attribute__((packed));

	enum eehdr
	{
		EEHDR_EMPTY = 0xffff,	//Eeeprom empty
		EEHDR_VALID = 0xCDAB
	};
	const unsigned EEHDR_ADDRESS = 0;
}

 
virtual_eeprom::virtual_eeprom(iflash_mem &_flash, unsigned flash_sector)
	: flash(_flash), active_page(0), inactive_page(0), va_max(flash.page_size()/8)
{
	// TODO Auto-generated constructor stub
	lock();
	iflash_mem::paddr_t npages = flash.num_pages() - 2*flash_sector;
	unsigned short val1, val2;
	flash.read_halfword( npages-1, EEHDR_ADDRESS, val1 );
	flash.read_halfword( npages-2, EEHDR_ADDRESS, val2 );
	if(val1==EEHDR_VALID && val2==EEHDR_VALID)
	{
		if( flash.page_erase( npages-2 )<0 ) { unlock(); return; }
	}
	if(val1==EEHDR_VALID)
	{
		active_page = npages - 1;
		inactive_page = npages - 2;
	}
	else if(val2==EEHDR_VALID)
	{
		active_page = npages - 2;
		inactive_page = npages - 1;
	}
	else	//Total empty
	{
		int res = flash.page_erase(npages-1);
		if(res>=0)
			res = flash.page_erase(npages-2);
		if(res>=0)
			res = flash.write_halfword( npages-1, EEHDR_ADDRESS, EEHDR_VALID );
		if(res>=0)
		{
			active_page = npages - 1;
			inactive_page = npages - 2;
		}
	}
	unlock();
}
 
int virtual_eeprom::find_free_slot() const
{
	const uint32_t eaddr = (flash.page_size() / sizeof(eeitem) ) * sizeof(eeitem)
				- sizeof(eeitem) + sizeof(uint16_t);
	for( iflash_mem::poffs_t poffs=sizeof(uint16_t); poffs<=eaddr; poffs += sizeof(eeitem) )
	{
		uint16_t va;
		int res = flash.read_halfword( active_page, poffs, va );
		if(res<0) return res;
		if(va==EEHDR_EMPTY)  return poffs;
	}
	return ERRNO_NO_SPACE;
}
 
//Try sort the flash and switch bank
int virtual_eeprom::sort_flash()
{
	int res = ERRNO_OK;
	for(unsigned va=0, pa=sizeof(uint16_t); va<=va_max; va++)
	{
		unsigned val;
		res = read( va, val );
		if( res == ERRNO_OK )
		{
			res = flash.write_halfword( inactive_page, pa, va );
			if(res<0) return res;
			pa+= sizeof(uint16_t);
			res = flash.write_word( inactive_page, pa, val );
			if(res<0) return res;
			pa += sizeof(uint32_t);
		}
		else if( res != ERRNO_NOT_FOUND )
		{
			return res;
		}
	}
	//Format the rest of the pages
	res = flash.write_halfword(inactive_page, EEHDR_ADDRESS, EEHDR_VALID );
	if(res<0) return res;
	res = flash.page_erase( active_page );
	{
		iflash_mem::paddr_t tmp;
		tmp = inactive_page;
		inactive_page = active_page;
		active_page = tmp;
	}
	return res;
}

 
int virtual_eeprom::read(unsigned addr, unsigned &value ) const
{
	if( active_page<=0 || inactive_page<=0 )
		return ERRNO_INIT;
	if( addr>va_max )
		return ERRNO_VA_RANGE;
	lock();
	const int eaddr = (flash.page_size() / sizeof(eeitem) ) * sizeof(eeitem)
			- sizeof(eeitem) + sizeof(uint16_t);
	for(int a=eaddr; a>=static_cast<int>(sizeof(uint16_t)); a-=sizeof(eeitem) )
	{
		uint16_t vaddr;
		flash.read_halfword( active_page, a, vaddr );
		if( vaddr == addr)
		{
			flash.read_word( active_page, a+sizeof(vaddr), value );
			unlock();
			return ERRNO_OK;
		}
	}
	unlock();
	return ERRNO_NOT_FOUND;
}

 
int virtual_eeprom::write(unsigned address, unsigned value)
{
	if(active_page<=0 || inactive_page<=0)
		return ERRNO_INIT;
	if(address>va_max)
		return ERRNO_VA_RANGE;
	lock();
	int fsaddr = find_free_slot();
	if(fsaddr<0)	//No free slot try sort it
	{
		int sf = sort_flash();
		if(sf<0) { unlock(); return sf; }
		fsaddr = find_free_slot();
		if(fsaddr>0) { unlock(); return fsaddr; }
	}
	int res = flash.write_halfword( active_page, fsaddr, address );
	if(res>=0) res = flash.write_word(active_page, fsaddr+sizeof(uint16_t), value );
	unlock();
	return res;
}

 
}
 
