/*
 * hd44xx_bus.hpp
 *
 *  Created on: 08-01-2012
 *      Author: lucck
 */

#pragma once

#include <stdint.h>
#include <periph/drivers/display/mono/display_operators.hpp>

namespace periph::display {

	class hd44xx_display
	{
	public:
		enum err
		{
			EDISPLAY_OK,
			EDISPLAY_TIMEOUT=-675776,
			EDISPLAY_INVALID_PARAM
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
		int erase(int n_chars)
		{
			int ret;
			for(int i=0; i<n_chars; i++)
				if((ret = putc(' ')) != 0) return ret;
			return 0;
		}
		int setpos(int x, int y);
		int read_currpos();
		int show_icon(char chpos, const char *str);
        int progress_bar(int x, int y, int value, int width, char chpos=0);
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



}

