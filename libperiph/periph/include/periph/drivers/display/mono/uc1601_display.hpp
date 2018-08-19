/*
 * uc1601_display.hpp
 *
 *  Created on: 20 paź 2013
 *      Author: lucck
 */

#pragma once

#include <cstdint>
#include <cstddef>
#include <foundation/algo/noncopyable.hpp>
#include <periph/drivers/display/mono/display.hpp>

namespace periph::display {


//! UC1601 bus interface
class uc1601_bus
{
public:
	//! Destructor
	virtual ~uc1601_bus()
	{
	}
	//! Empty comand statement
	static constexpr auto CMD_EMPTY = 1;
	/** Write command to the uc1601 device
	 *
	 * @param cmd1 First command
	 * @param cmd2 Second command
	 * @return Error code
	 */
	int command( int cmd1, int cmd2 = CMD_EMPTY )
	{
		return command_( cmd1, cmd2 );
	}
	/**
	 * Write data to the bus
	 * @param buffer to write
	 * @param len buffer size
	 * @return Error code
	 */
	virtual int data_wr( const uint8_t *buffer, size_t len ) = 0;
	/**
	 * Read data from the bus
	 * @param buffer Bufer tp read
	 * @param len size of the buffer
	 * @return	Error code
	 */
	virtual int data_rd(  uint8_t *buffer, size_t len ) = 0;
	/**
	 *
	 * @param timeout timeout in ms
	 */
	virtual void mdelay( unsigned timeout ) = 0;
protected:
	/**
	 * Write command implementation
	 * @param cmd1 First Command
	 * @param cmd2 Second Command
	 * @return Error code
	 */
	virtual int command_( int cmd1, int cmd2 ) = 0;
};

//! UC1601 basic command display
class uc1601_display : public display
{
private:
	static constexpr auto uc1601_cols = 132;
public:

	using box_t = display::box_t;

	/**
	 * Display controler
	 * @param bus_ initialize and setup the display
	 * @param cols numer of cols
	 * @param rows number of rows
	 */
	uc1601_display( uc1601_bus &bus_, uint8_t cols, uint8_t rows );


	/** Destructor */
	~uc1601_display();


	/**
	 * Enable the display
	 * @param en Enabled
	 * @return error code
	 */
	 int enable( bool en ) noexcept override;


	/** Putchar
	 * @param c char
	 * @return error code
	 */
	int putc( char ch ) noexcept override;

	/**
	 * Set cursor position
	 * @param x cursor position
	 * @param y cursor position
	 * @return error code
	 */
	int setpos(int x, int y) noexcept override;

	/**
	 * Clear the display
	 * @return Error code
	 */
	int clear() noexcept override;


	/**
	 *  draw box arround the area
	 * @param x1	X pos
	 * @param y1	Y pos
	 * @param cx	box width
	 * @param cy	box height
	 * @param type display frame type @see box_t
	 */
	int box(int x1, int y1, int cx, int cy, box_t type=box_t::clear)
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
	int progress_bar(int x1, int y1, int cx, int cy, int value, int max=100)
	noexcept override;

	/**
	 * Show icon on screen
	 * @param x1	Position X icon
	 * @param y1	Position Y icon
	 * @param icon	Icon Pointer
	 * @return		Error code
	 */
	int show_icon(int x1, int y1, const icon_t* icon)
	noexcept override;
	/**
	 * Go to to endl using current font
	 * @return Error code
	 */
	int endl() noexcept override;

private:
	/**
	 *
	 * @param pa Page adres
	 * @param ca Row address
	 * @return Error code
	 */
	int address_set( uint8_t pa, uint8_t ca );
private:
	//! Bus interface
	uc1601_bus &bus;
	//Current page address and column address
	uint8_t m_pa {}, m_ca {};
};

}


