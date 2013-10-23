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
#include <foundation/display_operators.hpp>
/* ------------------------------------------------------------------ */
namespace fnd {
namespace lcd {
/* ------------------------------------------------------------------ */
struct font_t;

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
/* ------------------------------------------------------------------ */
//! UC1601 basic command display
class uc1601_display : private fnd::noncopyable
{
private:
	static constexpr auto uc1601_cols = 132;
public:
	enum error	: int
	{
		ERR_OK = 0,
		ERR_ALIGN = -1,
		ERR_MISSING_FONT = -2,
		ERR_NO_CHAR = -3,		//Missing char
		ERR_OUT_RANGE = -4
	};

	enum class box_t
	{
		clear,	//Clear only
		frame,	//Clear + frame
		fill,	//Fill only
	};

	/**
	 * Display controler
	 * @param bus_ initialize and setup the display
	 * @param cols numer of cols
	 * @param rows number of rows
	 */
	uc1601_display( uc1601_bus &bus_, uint8_t cols, uint8_t rows );
	/** Putchar
	 * @param c char
	 * @return error code
	 */
	int putc( char ch );

	/**
	 * Set cursor position
	 * @param x cursor position
	 * @param y cursor position
	 * @return error code
	 */
	int setpos( int x, int y );

	/**
	 * Clear the display
	 * @return Error code
	 */
	int clear();

	/**
	 * Get error code
	 * @return error code
	 */
	int error() const
	{
		return m_error;
	}

	/**
	 *  draw box arround the area
	 * @param x1	X pos
	 * @param y1	Y pos
	 * @param cx	box width
	 * @param cy	box height
	 * @param type display frame type @see box_t
	 */
	int box( int x1, int y1, int cx, int cy, box_t type = box_t::clear );

	/**
	 * Set font
	 * @param font new font
	 */
	void set_font( const font_t * font )
	{
		m_font = font;
	}
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
	//! Error code
	int m_error;
	//! Selected font
	const font_t* m_font {};
	//Current page address and column address
	uint8_t m_pa {}, m_ca {};
	const uint8_t m_cols, m_rows;
};
/* ------------------------------------------------------------------ */
} /* namespace lcd */
} /* namespace fnd */
/* ------------------------------------------------------------------ */
#endif /* UC1601_DISPLAY_HPP_ */
/* ------------------------------------------------------------------ */
