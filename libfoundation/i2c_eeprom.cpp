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
#include <foundation/i2c_eeprom.hpp>
/* ------------------------------------------------------------------ */
namespace fnd {
/* ------------------------------------------------------------------ */
//!Internal private namespace
namespace {
	constexpr iflash_mem::poffs_t page_size_table[] = { 16 };
	constexpr iflash_mem::paddr_t page_number_table[] = { 128 };
}
/* ------------------------------------------------------------------ */ 
/** @param[in] bus Input bus owner
*  @param[in] bus_addr Bus memory address
*  @param[in] dev_type Device type
*/
i2c_eeprom::i2c_eeprom( fnd::bus::ibus &bus, unsigned bus_addr ,type dev_type )
	: m_bus(bus), m_addr( bus_addr ), m_type( dev_type )
{
}
/* ------------------------------------------------------------------ */ 
//Get pagesize
iflash_mem::poffs_t i2c_eeprom::page_size() const
{
	return page_size_table[ int(m_type) ];
}
/* ------------------------------------------------------------------ */
//Get numpages
iflash_mem::paddr_t i2c_eeprom::num_pages() const
{
	return page_number_table[ int(m_type) ];
}
/* ------------------------------------------------------------------ */ 
//! Destructor
i2c_eeprom::~i2c_eeprom()
{
}
/* ------------------------------------------------------------------ */ 
/** Read data from selected address 
	* @param[in] pg Page address
	* @param[in] pa Page offset
	* @param[in] ptr Pointer to write
	* @param[in] len Size of buffer
	* @return error code or 0 if success */
int i2c_eeprom::write( paddr_t pg, poffs_t pa, const void* ptr, size_t len )
{

}
/* ------------------------------------------------------------------ */ 
/** Read data from selected address 
	* @param[in] pg Page address
	* @param[in] pa Page offset
	* @param[out] ptr Pointer to read
	* @param[in] len Size of buffer
	* @return error code or 0 if success
	* */
int i2c_eeprom::read( paddr_t pg, poffs_t pa, void* ptr, size_t len ) const
{

}
/* ------------------------------------------------------------------ */ 
}

