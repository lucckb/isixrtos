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
#include <periph/core/block_device.hpp>
#include <periph/drivers/display/mono/display.hpp>
#include <periph/core/block_device.hpp>
#include <periph/core/block_device.hpp>

namespace periph::display {

class ssd1306 final : public display
{
	enum class pos : bool {
		x, y
	};
	static int dts_pos(const char* name, pos xy);
public:
	//Noncopyable
	~ssd1306() {}
	ssd1306(ssd1306&) = delete;
	ssd1306& operator=(ssd1306&) = delete;

	/** Create SSD1306 display
	 * @param[in] display_name Driver name
	 * @param[in] parent Parent driver
	 */
	ssd1306(const char* display_name, periph::block_device& parent);

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
	int hline(int x, int y, int h, color_t color)
		noexcept override;

	/**
		* @param x Start position X
		* @param y Start position Y
		* @param h Height
		* @param color Input color
		* @return Error code
		*/
	int vline(int x, int y, int h, color_t color)
		noexcept override;


	/**
	*  draw box arround the area
	* @param x1	X pos
	* @param y1	Y pos
	* @param cx	box width
	* @param cy	box height
	* @param type display frame type @see box_t
	*/
	int box(int x1, int y1, int cx, int cy, box_t type)
		noexcept override;

	/**
	* Display progress bar
	* @param x1	Start position X
	* @param y1	Start position Y
	* @param cx	Bar width
	* @param cy	Bar height
	* @param value	Progress value
	* @param max	Maximum value
	* @return Error code
	*/
	int progress_bar(int x1, int y1, int cx,
		int cy, int value, int max) noexcept override;

	/**
	* Show icon on screen
	* @param x1	Position X icon
	* @param y1	Position Y icon
	* @param icon	Icon Pointer
	* @return		Error code
	*/
	int show_icon(int x1, int y1, const icon_t* icon) noexcept override;
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
	uint8_t m_cs {};
	uint8_t m_x {};
	uint8_t m_y {};
	int m_gpio_di {};
	int m_gpio_rst {};
	periph::block_device& m_parent;
};

}

