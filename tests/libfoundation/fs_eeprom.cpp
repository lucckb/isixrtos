#include <foundation/fs_eeprom.hpp>
#include <cstring>
#include <stdexcept>
#include <unistd.h>
  
namespace fnd {
  

/** @param[in] bus Input bus owner
	*  @param[in] bus_addr Bus memory address
	*  @param[in] dev_type Device type
	*  @param[in] filename Input filename used for testing
	*/
fs_eeprom::fs_eeprom( int n_pages, int page_size, bool emulate_flash, const char *filename )
	:  m_page_size( page_size ), m_num_pages( n_pages ), m_emulate_flash(emulate_flash)
{	
	{
		//Check if file not exist and create it
		std::ifstream iff ( filename );
		if( !iff.good() ) {
			std::ofstream off( filename , std::ofstream::out|std::ofstream::trunc );
		}
	}
	m_file.open( filename );
	m_file.exceptions( std::ios::failbit );
	m_file.seekp( 0, m_file.end );
	auto size = m_file.tellp();
	if( size < ( page_size* n_pages ) ) {
		auto remain_bytes = (page_size * n_pages) - size;
		char buf [remain_bytes];
		std::memset(buf, 0xff, sizeof buf);
		m_file.write( buf, sizeof buf );
	}
}
  
void fs_eeprom::check_range( paddr_t pg, poffs_t pa, size_t len ) const
{
	if( pg > m_num_pages ) {
		throw std::logic_error("Page addr");
	}
	if( pa > m_page_size ) {
		throw std::logic_error("Page offs");
	}
	if( pa + len > m_page_size ) {
		throw std::logic_error("Page len error");
	}
}
 
//Erase page return status
int fs_eeprom::page_erase( paddr_t pa ) 
{
	check_range( pa, 0, m_page_size );
	char empty_page [ m_page_size ];
	std::memset( empty_page, 0xff, m_page_size );
	m_file.seekp( pa*m_page_size, m_file.beg );
	m_file.write( empty_page, m_page_size );
	sleep(2);
	return 0;
}
  

int fs_eeprom::write( paddr_t pg, poffs_t pa ,const void* ptr , size_t len )
{
	check_range( pg, pa, len );
	//!If it is flash memory emulate flash behavior
	if( m_emulate_flash ) {
		unsigned char buf[len];
		m_file.seekg( pg*m_page_size + pa , m_file.beg );
		m_file.read( reinterpret_cast<char*>(buf), len );
		for( size_t s = 0; s < len; ++s ) {
			buf[s] &= reinterpret_cast<const unsigned char*>(ptr)[s];
		}
		m_file.seekp( pg*m_page_size + pa, m_file.beg );
		m_file.write( reinterpret_cast<const char*>(buf), len );
		usleep( 20 );
	} else {
		m_file.seekp( pg*m_page_size + pa, m_file.beg );
		m_file.write( reinterpret_cast<const char*>(ptr), len );
		usleep(7000 + 3*len);
	}
	return 0;
}
  
int fs_eeprom::read( paddr_t pg, poffs_t pa, void* ptr, size_t len ) const
{
	check_range( pg, pa, len );
	m_file.seekg( pg*m_page_size + pa , m_file.beg );
	m_file.read( reinterpret_cast<char*>(ptr), len );
	usleep( 5*len );
	return 0;
}
  
}

