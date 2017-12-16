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
#include <foundation/drv/bus/ibus.hpp>
#include <foundation/drv/lcd/display.hpp>

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
	 * @param[in] cols Number of cols
	 * @param[in] rows Number of rows
	 */
	ssd1306(bus::ibus& bus, uint8_t cols, uint8_t rows)
		: display(cols,rows), m_bus(bus)
	{}
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

private:
	/* data */
	bus::ibus& m_bus;
};

}
}
}


