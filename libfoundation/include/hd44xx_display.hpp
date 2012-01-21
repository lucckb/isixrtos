/*
 * hd44xx_bus.hpp
 *
 *  Created on: 08-01-2012
 *      Author: lucck
 */
/*----------------------------------------------------------*/
#ifndef HD44XX_DISPLAY_HPP_
#define HD44XX_DISPLAY_HPP_
/*----------------------------------------------------------*/
#include <stdint.h>
/*----------------------------------------------------------*/
namespace fnd
{
namespace lcd
{
	class hd44xx_display
	{
	public:
		enum err
		{
			EDISPLAY_OK,
			EDISPLAY_TIMEOUT=-675776
		};
		hd44xx_display();
		virtual ~hd44xx_display() {}
		int clear();
		int cursor(bool enable)
		{
			write_dev(CMD_DISP_ON|(enable?0x07:0x04),W_INSTR);
			return wait4dev();
		}
		int putc(char ch)
		{
			write_dev(ch,W_DATA);
			return wait4dev();
		}
		int setpos(int x, int y);
		int read_currpos();
		int show_icon(char chpos, const char *str);
	protected:
		virtual void write8_i(uint8_t addr, uint8_t value ) = 0;
		virtual void write8_n(uint8_t addr, uint8_t value ) = 0;
		virtual uint8_t read8( uint8_t addr )  = 0;
		virtual void mdelay( unsigned period)  = 0;
	private:
		int wait4dev();
		enum wtype { W_INSTR, W_DATA };
		void write_dev(unsigned char val, wtype command)
		{
			if( !m_initialized )
			{
				init_dev();
			}
			write8_n(command, val);
		}
		uint8_t read_dev( wtype command )
		{
			if( !m_initialized )
			{
				init_dev();
			}
			return read8( command );
		}
		int init_dev();
	private:
		static const unsigned CMD_DISP_ON = 8;
		bool m_initialized;
	private: 	//Noncopyable
		hd44xx_display(hd44xx_display &);
		hd44xx_display& operator=(const hd44xx_display&);
	};
/*----------------------------------------------------------*/
	class lfmt
	{
		friend hd44xx_display& operator<<(hd44xx_display &o,const lfmt &fmt);
	public:
		enum
		{
			dec = 10,
			hex = 16
		};
		lfmt(unsigned val_, int fmt_, char fmtch_='0', short base_=dec)
		: val(val_),fmt(fmt_),fmtch(fmtch_) , base(base_) {}
	private:
		unsigned val;
		int fmt;
		char fmtch;
		short base;
	};
/* ------------------------------------------------------------------ */
	struct icon
	{
		friend hd44xx_display& operator<<(hd44xx_display &o,const icon &fmt);
	protected:
		virtual const char* get_data() const = 0;
		virtual char get_char() const = 0;
	};
/* ------------------------------------------------------------------ */
	class pos
	{
		friend hd44xx_display& operator<<(hd44xx_display &o,const pos &fmt);
	public:
		pos(int x_, int y_)
		:x(x_), y(y_) {}
	private:
		unsigned char x;
		unsigned char y;
	};
/* ------------------------------------------------------------------ */
	hd44xx_display& operator<<(hd44xx_display &o,const char *str);

	hd44xx_display& operator<<(hd44xx_display &o,unsigned value);

	hd44xx_display& operator<<(hd44xx_display &o,const lfmt &fmt);

	hd44xx_display& operator<<(hd44xx_display &o,const icon &fmt);

	hd44xx_display& operator<<(hd44xx_display &o,const pos &fmt);
/* ------------------------------------------------------------------ */

}}
/*----------------------------------------------------------*/
#endif /* HD44XX_BUS_HPP_ */