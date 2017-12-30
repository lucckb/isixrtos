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
#include <foundation/drv/lcd/lcd_font.hpp>
#include <foundation/drv/lcd/lcd_bitmap.hpp>
#include <foundation/sys/dbglog.h>
#include "ssd1306_cmds.hpp"

namespace fnd {
namespace drv {
namespace lcd {

namespace {
	constexpr uint8_t zero_buffer[32] {};
	inline size_t align( std::size_t value, std::size_t align ) {
		return (value+align-1) & ~(align-1);
	}
}

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
	if(en) m_error = initialize();
	else m_error = deinitialize();
	return m_error;
}

// SSD set display brightness
int ssd1306::brightness(bright_t value) noexcept
{
	return command( {ns1306::cmd::SET_CONTRAST, value} );
}

// SSD put character on selected position
int ssd1306::putc(char ch) noexcept
{
	//dbg_debug("putc(%c)", ch );
	do {
		if(!font()) {
			m_error = ERR_MISSING_FONT;
			break;
		}
		if( (ch < font()->first_char || ch > font()->last_char) && ch!=' ' ) {
			m_error = ERR_NO_CHAR;
			break;
		}
		const auto cho = ch - font()->first_char;
		const auto width = (ch!=' ')?(font()->chr_desc[int(cho)].width):(font()->spc_width);
		const auto height = align(font()->height,8);
		const auto offs = font()->chr_desc[int(cho)].offset;
		auto bmp_ptr = (ch!=' ')?(&font()->bmp[offs]):(zero_buffer);
		if(m_x+width+1 > m_cols) {
			m_error = ERR_OUT_RANGE;
			break;
		}
		//dbg_debug("Font width %i Font height %i", width, height );
		setpos( m_x, m_y, m_x + width-1, m_rows-1 );
		if(m_error) break;
		data( bmp_ptr, width*height/8 );
		if(m_error) break;
		setpos( m_x+width, m_y, m_cols-1, m_rows-1 );
		if(m_error) break;
	} while(0);
	return m_error;
}

// Clear the screen
int ssd1306::clear() noexcept
{
	using namespace ns1306;
	do {
		m_error = setpos(0,0); if(m_error) break;
		for( auto i = 0U; i < m_cols*m_rows/8/sizeof(zero_buffer); ++i ) {
			data( zero_buffer, sizeof zero_buffer );
			if(m_error) break;
		}
	} while(0);
	return m_error;
}

/** Setup cursor position with range */
int ssd1306::setpos( uint8_t x, uint8_t y, uint8_t maxx, uint8_t maxy ) noexcept
{
	//dbg_debug("setpos(x=%u, y=%u, maxx=%u, maxy=%u)", x,y,maxx,maxy );
	m_x = x;
	m_y = y;
	return command(
		{
			ns1306::cmd::SET_COLUMN_ADDR,
			x,
			maxx,
			ns1306::cmd::SET_PAGE_ADDR,
			uint8_t(y/8),
			uint8_t(maxy/8)
		}
	);
}

// Set display at position
int ssd1306::setpos(int x, int y) noexcept
{
	if(x > m_cols) {
		m_error = ERR_OUT_RANGE;
		return m_error;
	}
	if(y > m_rows) {
		m_error = ERR_OUT_RANGE;
		return m_error;
	}
	return setpos( x, y, m_cols-1, m_rows-1 );
}

// Go to new line using font
int ssd1306::endl() noexcept
{
	do {
		if( !font() ) {
			m_error = ERR_MISSING_FONT;
			break;
		}
		 setpos( 0, align(m_y+font()->height,8) );
	} while(0);
	return m_error;
}


//! Send command to display single byte

int ssd1306::command( uint8_t cmd )
{
	//! Data or instruction
	m_di(false);
	m_error = m_bus.write(m_cs, &cmd, sizeof(cmd));
	return m_error;
}

//! Send command to the display
int ssd1306::command(const std::initializer_list<uint8_t>& cmd) noexcept
{
	m_di(false);
	m_error = m_bus.write(m_cs, cmd.begin(), cmd.size());
	return m_error;
}


//! Write data
int ssd1306::data( const uint8_t buf[], std::size_t len ) noexcept
{
	m_di(true);
	m_error = m_bus.write(m_cs,buf,len);
	return m_error;
}


int ssd1306::data( const std::initializer_list<uint8_t>& cmd ) noexcept
{
	m_di(true);
	m_error = m_bus.write(m_cs, cmd.begin(), cmd.size());
	return m_error;
}


// Initialize the display
int ssd1306::initialize() noexcept
{
	namespace cmd = ns1306::cmd;
	constexpr uint8_t init_cmds[] = {
		cmd::DISPLAY_OFF,
		cmd::SET_CLK_DIV,		0x80,
		cmd::SET_MUX,			0x3F,
		cmd::SET_OFFSET,		0x00,
		cmd::SET_CHARGEPUMP,	0x14,
		cmd::ADDR_MODE,			cmd::addr_mode::horiz,
		cmd::SET_SEGMENT_REMAP,
		cmd::DISPLAY_ON,
		cmd::SET_COM_SCAN_DEC,
		cmd::SET_COM_PINS,		0x12,
		cmd::SET_CONTRAST,		0xAF,
		cmd::SET_PRECHARGE,		0x25,
		cmd::SET_VCOM_DESELECT, 0x20,
		cmd::NORMAL,
		cmd::DISPLAY_ALL_ON_RES,
		cmd::DISPLAY_ON
	};
	m_rst(false);
	m_bus.mdelay(10);
	m_rst(true);
	m_bus.mdelay(10);
	m_error = m_bus.write(m_cs, init_cmds, sizeof init_cmds);
	return m_error;
}

// Deinitialize the display
int ssd1306::deinitialize() noexcept
{
	return 0;
}

}}}

