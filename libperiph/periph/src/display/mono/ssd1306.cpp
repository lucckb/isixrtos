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
#include <isix.h>
#include <periph/core/error.hpp>
#include <periph/drivers/display/mono/ssd1306.hpp>
#include <periph/drivers/display/mono/lcd_font.hpp>
#include <periph/drivers/display/mono/lcd_bitmap.hpp>
#include <periph/drivers/display/mono/display_config.hpp>
#include <periph/gpio/gpio.hpp>
#include <periph/blk/transfer.hpp>
#include <periph/dt/dts.hpp>
#include <foundation/sys/dbglog.h>
#include "ssd1306_cmds.hpp"

namespace periph::display {

namespace {
	constexpr uint8_t zero_buffer[32] {};
	constexpr uint8_t ff_buffer[32]
		{ 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff };
	constexpr uint8_t pattern_buffer[8][16] = {
		{ 0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01 },
		{ 0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02 },
		{ 0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04 },
		{ 0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08 },
		{ 0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10 },
		{ 0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20 },
		{ 0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40 },
		{ 0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80 },
	};
	inline size_t align( std::size_t value, std::size_t align ) {
		return (value+align-1) & ~(align-1);
	}
}

//! Get display config by dts
int ssd1306::dts_pos(const char* name, pos xy)
{
	const periph::dt::device_conf_base* cfg;
	periph::error::expose<periph::error::generic_exception>(dt::get_periph_devconf(name,cfg));
	auto dc = static_cast<const periph::display::config&>(*cfg);
	dbg_info("x: %i y: %i", dc.max_x, dc.max_y);
	return xy==pos::x?dc.max_x:dc.max_y;
}

//! Constructor
ssd1306::ssd1306(const char* disp_name, periph::block_device& parent)
	: display(dts_pos(disp_name,pos::x),dts_pos(disp_name,pos::y)),
	  m_parent(parent)
{
	//GPIOS configure
	m_gpio_di = dt::get_periph_pin(disp_name, dt::pinfunc::rw);
	if(m_gpio_di<0) {
		dbg_err("Unable to get RW pin func for dev %s",disp_name);
		periph::error::expose<periph::error::generic_exception>(m_gpio_di);
	}
	dbg_info("RWPIN is %i", m_gpio_di);
	gpio::setup(m_gpio_di, gpio::mode::out{gpio::outtype::pushpull,gpio::speed::medium} );
	m_gpio_rst = dt::get_periph_pin(disp_name, dt::pinfunc::rst);
	if(m_gpio_rst<0) {
		dbg_err("Unable to get RST pin func for dev %i",disp_name);
		periph::error::expose<periph::error::generic_exception>(m_gpio_rst);
	}
	dbg_info("RSTPIN is %i", m_gpio_rst);
	gpio::setup(m_gpio_rst, gpio::mode::out{gpio::outtype::pushpull,gpio::speed::medium} );
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
		// Single space between chars
		auto smx = m_x;
		setpos( smx+width, m_y, smx+width+1, m_rows-1 );
		if(m_error) break;
		data( zero_buffer, height/8U );
		if(m_error) break;
		// Single space between chars
		setpos( smx+width+1, m_y, m_cols-1, m_rows-1 );
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
	periph::gpio::set(m_gpio_di, false);
	periph::blk::tx_transfer trans(&cmd,sizeof cmd);
	m_error = m_parent.transaction(m_cs, trans);
	return m_error;
}

//! Send command to the display
int ssd1306::command(const std::initializer_list<uint8_t>& cmd) noexcept
{
	periph::gpio::set(m_gpio_di, false);
	periph::blk::tx_transfer trans(cmd.begin(),cmd.size());
	m_error = m_parent.transaction(m_cs, trans);
	return m_error;
}


//! Write data
int ssd1306::data( const uint8_t buf[], std::size_t len ) noexcept
{
	periph::gpio::set(m_gpio_di, true);
	periph::blk::tx_transfer trans(buf,len);
	m_error = m_parent.transaction(m_cs, trans);
	return m_error;
}


int ssd1306::data( const std::initializer_list<uint8_t>& cmd ) noexcept
{
	periph::gpio::set(m_gpio_di, true);
	periph::blk::tx_transfer trans(cmd.begin(),cmd.size());
	m_error = m_parent.transaction(m_cs, trans);
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
	periph::gpio::set(m_gpio_rst, false);
	isix::wait_ms(10);
	periph::gpio::set(m_gpio_rst, true);
	isix::wait_ms(10);
	periph::blk::tx_transfer trans(init_cmds, sizeof init_cmds);
	m_error = m_parent.transaction(m_cs, trans);
	return m_error;
}

// Deinitialize the display
int ssd1306::deinitialize() noexcept
{
	return command( ns1306::cmd::DISPLAY_OFF );
}


//! Draw horizontal line
int ssd1306::hline(int x, int y, int h, color_t color) noexcept
{
	do {
		if(color==color::white) {
			m_error = ERR_INVALID_ARG;
			dbg_err("White line without FB not supported");
			break;
		}
		setpos(x,y); if(m_error) break;
		for( auto n=0U; n<h/sizeof(pattern_buffer[0]); ++n) {
			data(pattern_buffer[y%8],sizeof pattern_buffer[0] );
			if(m_error) break;
		}
		if(m_error) break;
		if( h%sizeof(pattern_buffer[0]) ) {
			data( pattern_buffer[y%8], h%sizeof(pattern_buffer[0]) );
			if(m_error) break;
		}
	} while(0);
	return m_error;
}


//! Draw vertical line
int ssd1306::vline(int x, int y, int h, color_t color) noexcept
{
	do {
		if(color==color::white) {
			m_error = ERR_INVALID_ARG;
			dbg_err("White line without FB not supported");
			break;
		}
		// Switch address mode to vertical
		command( {ns1306::cmd::ADDR_MODE,ns1306::cmd::addr_mode::vert} );
		if(m_error) break;
		setpos(x,y); if(m_error) break;
		uint8_t htb;
		if(y%8) {
			htb = 0xFF<<(y%8);
			data({htb}); if(m_error) break;
			h-=y%8;
		}
		data( ff_buffer, h/8 );
		if(m_error) break;
		if( h%8 ) {
			htb = 0xFF>>(h%8);
			data({htb}); if(m_error) break;
		}
		command( {ns1306::cmd::ADDR_MODE,ns1306::cmd::addr_mode::horiz} );
		if(m_error) break;
	} while(0);
	return m_error;
}


//! Draw a box
int ssd1306::box(int x1, int y1, int cx, int cy, box_t type) noexcept
{
	do {
		if(x1>m_cols || y1>m_rows || (x1+cx)>m_cols || (y1+cy)>m_rows) {
			m_error = ERR_OUT_RANGE;
			break;
		}
		if(y1%8 || cy%8 || cy<16 ) {
			m_error = ERR_ALIGN;
			break;
		}
		setpos(x1,y1,x1+cx-1,y1+cy-1); if(m_error) break;
		for( auto i = 0U; i < m_cols*m_rows/8/sizeof(zero_buffer); ++i ) {
			if(type==box_t::fill)
				data(ff_buffer,sizeof ff_buffer);
			else
				data(zero_buffer,sizeof zero_buffer);
			if(m_error) break;
		}
		if(type==box_t::frame) {
			hline(x1,y1,cx,color::black);
			if(m_error) break;
			hline(x1,y1+cy-1,cx,color::black);
			if(m_error) break;
			vline(x1,y1,cy,color::black);
			if(m_error) break;
			vline(x1+cx,y1,cy,color::black);
			if(m_error) break;
		}
	} while(0);
	return m_error;
}


//! Display progress bar
int ssd1306::progress_bar(int x1, int y1, int cx, int cy, int value, int max) noexcept
{
	do
	{
		const int cx1 = ( cx * value ) / max;
		const int cx2 = cx - cx1;
		box( x1+cx1, y1, cx2, cy, box_t::frame );
		if( m_error ) break;
		box( x1, y1, cx1, cy, box_t::fill );
		if( m_error ) break;
	}
	while(0);
	return m_error;
}


//! Display icon on the screen
int ssd1306::show_icon(int x1, int y1, const icon_t* icon) noexcept
{
	do {
		if( !icon )
		{
			m_error = ERR_INVALID_ARG;
			break;
		}
		if( y1%8 )
		{
			m_error = ERR_ALIGN;
			break;
		}
		if(y1/8+icon->pg_width>m_rows/8 || x1+icon->height>m_cols )
		{
			m_error = ERR_OUT_RANGE;
			break;
		}
		setpos(x1,y1,x1+icon->height-1,y1+icon->pg_width*8-1);
		if(m_error) break;
		data( icon->data, icon->pg_width*icon->height );
		if(m_error) break;
	} while(0);
	return m_error;
}

}

