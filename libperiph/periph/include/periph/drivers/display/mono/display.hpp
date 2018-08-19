/*
 * =====================================================================================
 *
 *       Filename:  display.hpp
 *
 *    Description:  Base class for LCD display
 *
 *        Version:  1.0
 *        Created:  12/15/2017  4:56:07 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once

#include <cstdint>
#include <cstddef>
#include <periph/drivers/display/mono/display_operators.hpp>


namespace periph::display {



	//! Determine display position
	using pos_t = std::int16_t;
	using bright_t = std::uint8_t;
	using color_t = bool;

	//! Font definitions
	struct font_t;
	struct icon_t;

	//! Color class definition
	enum color : color_t {
		white,
		black
	};


	//! Base class for display driver
	class display
	{
	public:
		//! Display errors
		enum error	: int
		{
			ERR_OK				= 0,		//! No error
			ERR_ALIGN			= -4096,	//! Alignment error
			ERR_MISSING_FONT	= -4097,	//! Missing font
			ERR_NO_CHAR			= -4098,	//! No character in table
			ERR_OUT_RANGE		= -4099,	//! Out of range
			ERR_INVALID_ARG		= -4100,	//! Invalid argument
			ERR_NOT_SUPPORTED	= -4101		//! Command not supported
		};
		//! Box draw type
		enum class box_t
		{
			clear,	//!Clear only
			frame,	//!Clear + frame
			fill,	//!Fill only
		};
	public:
		/**
		 * Display constructor object
		 * @param cols Numbers of cols on the screen
		 * @param rows Numbers of rows on the screen
		 */
		display(pos_t cols, pos_t rows)
			: m_cols(cols), m_rows(rows)
		{}
		//! Display destructor
		virtual ~display() {}
		//! Noncopyable
		display(display&) = delete;
		display& operator=(display&) = delete;
		/**
		* Get error code
		* @return error code
		*/
		int error() const noexcept {
			return m_error;
		}
		/** Check if all is ok
		 *  @return true if ok
		 */
		operator bool() const noexcept {
			return m_error==ERR_NOT_SUPPORTED;
		}
		/**
		 * Enable and initialize the display
		 * @param en Enable or disable
		 * @return Error code
		 */
		virtual int enable( bool en ) noexcept = 0;
		/**
		 * Set display brightness
		 * @param value Brightness
		 * @return Error code
		 */
		virtual int brightness(bright_t /*value*/) noexcept
		{
			return ERR_NOT_SUPPORTED;
		}
		/**
		* Set font
		* @param font new font
		*/
		void set_font( const font_t * font ) noexcept {
			m_font = font;
		}
		/** Put character at current display position
		* @param ch char
		* @return error code
		*/
		virtual int putc(char ch) noexcept = 0;

		/** Put string at selected position
		 * @param[in] str Input string to print
		 */
		int puts(const char str[]) noexcept
		{
			while(*str)
			{
				m_error = putc(*str++);
				if(m_error) break;
			}
			return m_error;
		}
		/**
		 * Put character at position
		 * @param x Coord x
		 * @param y	Coord y
		 * @param str string position
		 * @return error code
		 */
		int putxy(int x, int y, const char str[])
		{
			do {
				m_error = setpos(x,y);
				if(m_error) break;
				m_error = puts(str);
				if(m_error) break;
			} while(0);
			return m_error;
		}
		/**
		* Clear the display
		* @return Error code
		*/
		virtual int clear() noexcept = 0;
		/**
		* Set cursor position
		* @param x cursor position
		* @param y cursor position
		* @return error code
		*/
		virtual int setpos(int x, int y) noexcept = 0;
		/**
		* Go to to endl using current font
		* @return Error code
		*/
		virtual int endl() noexcept = 0;
		/**
		*  draw box arround the area
		* @param x1	X pos
		* @param y1	Y pos
		* @param cx	box width
		* @param cy	box height
		* @param type display frame type @see box_t
		*/
		virtual
		int box(int /*x1*/, int /*y1*/, int /*cx*/, int /*cy*/,
				box_t /*type*/) noexcept
		{
			return ERR_NOT_SUPPORTED;
		}
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
		virtual
		int progress_bar(int /*x1*/, int /*y1*/, int /*cx*/,
				int /*cy*/, int /*value*/, int /*max*/) noexcept
		{
			return ERR_NOT_SUPPORTED;
		}
		/**
		* Show icon on screen
		* @param x1	Position X icon
		* @param y1	Position Y icon
		* @param icon	Icon Pointer
		* @return		Error code
		*/
		virtual
		int show_icon(int /*x1*/, int /*y1*/,
				const icon_t* /*icon*/) noexcept
		{
			return ERR_NOT_SUPPORTED;
		}
		/**
		 * Draw horizontal line
		 * @param x Start position X
		 * @param y Start position Y
		 * @param h Height
		 * @param color Input color
		 * @return Error code
		 */
		virtual
		int hline(int /*x*/, int /*y*/, int /*h*/, color_t /*color*/) noexcept {
			return ERR_NOT_SUPPORTED;
		}
		/**
		 * @param x Start position X
		 * @param y Start position Y
		 * @param h Height
		 * @param color Input color
		 * @return Error code
		 */
		virtual
		int vline(int /*x*/, int /*y*/, int /*h*/, color_t /*color*/) noexcept {
			return ERR_NOT_SUPPORTED;
		}
	protected:
		//! Get font
		auto font() noexcept { return m_font; }
	protected:
		int m_error {};		//! Error code
		const pos_t m_cols;	//! Number of cols
		const pos_t m_rows;	//! Number of rows
	private:
		const font_t* m_font {}; //! Selected font
	};

}
