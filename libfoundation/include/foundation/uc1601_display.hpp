/*
 * uc1601_display.hpp
 *
 *  Created on: 20 paź 2013
 *      Author: lucck
 */

#ifndef UC1601_DISPLAY_HPP_
#define UC1601_DISPLAY_HPP_
/* ------------------------------------------------------------------ */
#include <cstdint>
#include <cstddef>
#include <foundation/noncopyable.hpp>
/* ------------------------------------------------------------------ */
namespace fnd {
namespace lcd {

/* ------------------------------------------------------------------ */
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
protected:
	/**
	 * Write command implementation
	 * @param cmd1 First Command
	 * @param cmd2 Second Command
	 * @return Error code
	 */
	virtual int command_( int cmd1, int cmd2 ) = 0;
};
/* ------------------------------------------------------------------ */
//! UC1601 basic command display
class uc1601_display : private fnd::noncopyable
{
public:
	/**
	 * Display controler
	 * @param bus_ initialize and setup the display
	 * @param rows number of rows
	 */
	uc1601_display( uc1601_bus &bus_, int rows);
	/** Putchar
	 * @param c char
	 * @return error code
	 */
	int putchar( char c );

	/**
	 * Set cursor position
	 * @param x cursor position
	 * @param y cursor position
	 * @return
	 */
	int setpos( int x, int y );
private:
	//! Bus interface
	uc1601_bus &bus;
};
/* ------------------------------------------------------------------ */
} /* namespace lcd */
} /* namespace fnd */
/* ------------------------------------------------------------------ */
#endif /* UC1601_DISPLAY_HPP_ */
/* ------------------------------------------------------------------ */
