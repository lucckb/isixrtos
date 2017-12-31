/*
 * =====================================================================================
 *
 *       Filename:  ssd1306.hpp
 *
 *    Description:  ssd1306 controller driver
 *
 *        Version:  1.0
 *        Created:  12/14/2017 10:53:13 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once
#include <initializer_list>
#include <foundation/drv/bus/ibus.hpp>
#include <foundation/drv/lcd/display.hpp>
#include "../bus/gpioout.hpp"

namespace fnd {
namespace drv {
namespace lcd {


class ssd1306 final : public display
{
public:
	//Noncopyable
	~ssd1306() {}
	ssd1306(ssd1306&) = delete;
	ssd1306& operator=(ssd1306&) = delete;
	/** Create SSD1306 display
	 * @param[in] bus Bus object
	 * @param[in] d_i  DI gpio object
	 * @param[in] rst  DI gpio object
	 * @param[in] cols Number of cols
	 * @param[in] rows Number of rows
	 */
	ssd1306(
		bus::ibus& bus, bus::gpio_out& d_i, bus::gpio_out& rst,
		uint8_t cs, uint8_t cols, uint8_t rows );
	/**
	 * Enable and initialize the display
	 * @param en Enable or disable
	 * @return Error code
	 */
	int enable(bool en) noexcept override;
	/**
		* Set display brightness
		* @param value Brightness
		* @return Error code
		*/
	int brightness(bright_t value) noexcept override;
	/** Put character at current display position
	* @param ch char
	* @return error code
	*/
	int putc(char ch) noexcept override;
	/**
	* Clear the display
	* @return Error code
	*/
	int clear() noexcept override;
	/**
	* Set cursor position
	* @param x cursor position
	* @param y cursor position
	* @return error code
	*/
	int setpos(int x, int y) noexcept override;
	/**
	* Go to to endl using current font
	* @return Error code
	*/
	int endl() noexcept override;

	/** Draw horizontal line on the screen 
	 * @param x Position X
	 * @param y Position y
	 * @param h length
	 * @param color line color
	 * @return error code
	 */
	//int draw_hline(int /*x*/, int /*y*/, int /*h*/, color_t /*color*/)
		//noexcept override;

private:
	/** Write command
	 * @param[in] cmd Input command
	 * @return error code
	 */
	int command( const std::initializer_list<uint8_t>& cmd) noexcept;
	int command( uint8_t cmd );

	// Initialize the display
	int initialize() noexcept;
	//! Deinitialize the display
	int deinitialize() noexcept;
	/**
	 *
	 * @param buf Input buffer
	 * @param len Input buffer length
	 * @return Error code
	 */
	int data( const uint8_t buf[], std::size_t len ) noexcept;
	int data( const std::initializer_list<uint8_t>& cmd ) noexcept;

	/** Setup cursor position with range */
	int setpos( uint8_t x, uint8_t y, uint8_t maxx, uint8_t maxy ) noexcept;

private:

	/* data */
	bus::ibus& m_bus;
	uint8_t m_cs;
	uint8_t m_x {};
	uint8_t m_y {};
	bus::gpio_out& m_di;
	bus::gpio_out& m_rst;
};

}
}
}



