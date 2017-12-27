/*
 * =====================================================================================
 *
 *       Filename:  ssd1306.cpp
 *
 *    Description:  SSD1306 display driver
 *
 *        Version:  1.0
 *        Created:  16.12.2017 22:29:26
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include <foundation/drv/lcd/ssd1306.hpp>
#include "ssd1306_cmds.hpp"

namespace fnd {
namespace drv {
namespace lcd {

//! Constructor
ssd1306::ssd1306(
	bus::ibus& bus, bus::gpio_out& d_i, bus::gpio_out& rst,
	uint8_t cs, uint8_t cols, uint8_t rows
)
	: display(cols,rows), m_bus(bus),
	  m_cs(cs), m_di(d_i), m_rst(rst)
{
}

// Enable the display
int ssd1306::enable(bool en) noexcept
{
	int err {};
	if(en) err = initialize();
	else err = deinitialize();
	return err;
}

// SSD set display brightness
int ssd1306::brightness(bright_t value) noexcept
{
}

// SSD put character on selected position
int ssd1306::putc(char ch) noexcept
{
}

// Clear the screen
int ssd1306::clear() noexcept
{
	int err {};
	static constexpr uint8_t buf[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	for(int row=0; row<m_rows/8; ++row) {
		if(err) break;
		err = setpos(0, row);
		if(err) break;
		for(int col=0; col<m_cols; ++col) {
			err = write_data(buf, sizeof buf);
			if(err) break;
		}
	}
	return err;
}


// Set display at position
int ssd1306::setpos(int x, int y) noexcept
{
	int err;
	do {
		if(x > m_cols) {
			err=ERR_OUT_RANGE;
			break;
		}
		if(y > m_rows) {
			err=ERR_OUT_RANGE;
			break;
		}
		err=command(ns1306::cmd::SET_COLUMN_ADDR);
		if(err) break;
		err=command(x);
		if(err) break;
		err=command(m_cols);
		if(err) break;
		err=command(ns1306::cmd::SET_PAGE_ADDR);
		if(err) break;
		err=command(y);
		if(err) break;
		err=command(m_rows/8-1);
		if(err) break;
	} while(0);
	return err;
}

// Go to new line using font
int ssd1306::endl() noexcept
{
}

//! Send command to the display
int ssd1306::command(uint8_t cmd) noexcept
{
	//! Data or instruction
	m_di(false);
	return m_bus.write(m_cs, &cmd, sizeof(cmd) );
}


//! Write data
int ssd1306::write_data( const uint8_t buf[], std::size_t len ) noexcept
{
	m_di(true);
	return m_bus.write(m_cs,buf,len);
}

// Initialize the display
int ssd1306::initialize() noexcept
{
	m_rst(false);
	m_bus.mdelay(10);
	m_rst(true);
	m_bus.mdelay(10);
	constexpr uint8_t init_cmds[] = {
		0xAE,						//Display off
		0xD5, 0x81,					//Clock
		0xA8, 0x3F,					//Mux ratio
		0xD3, 0x00, 0x00,			//Display offset
		0xD8, 0x14,					//Charge pump enable
		0x20, 0x00,					//Memory address mode - horiz
		0xA1,						//Segment remap
		0xA5,						//Display on
		0xC8, 						//Set com output scan direction
		0xDA, 0x12,					//Set com pins
		0x81, 0xFF,
		0xD9, 0x11,
		0xDB, 0x20,
		0xA6,
		0xA4, 0xAF
	};
	int err {};
	for( auto cmd : init_cmds ) {
		if( (err = command(cmd)) ) break;
	}
	return err;
}

// Deinitialize the display
int ssd1306::deinitialize() noexcept
{
	return 0;
}

}}}

