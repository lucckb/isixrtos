/*
 * hd44xx_display.cpp
 *
 *  Created on: 08-01-2012
 *      Author: lucck
 */

#include "hd44xx_display.hpp"
#include <stm32f10x_lib.h>
#include <system.h>
#include <utils.h>
/* ------------------------------------------------------------------ */
namespace fnd
{
namespace lcd
{
/* ------------------------------------------------------------------ */
namespace //CFG
{
	const unsigned WAIT_DELAY_TOUT = 32000;
    const char bar_patterns[] = { 0x00, 0x10, 0x18, 0x1c, 0x1E, 0x1F };
}
/* ------------------------------------------------------------------ */
hd44xx_display::hd44xx_display()
	: m_initialized(false)
{
}
/* ------------------------------------------------------------------ */
//Wait4lcd
int hd44xx_display::wait4dev()
{
    uint8_t r;
	int tout = WAIT_DELAY_TOUT;
	//Sprawdzenie czy jest wolny wyswietlacz
	do
	{
        r = read_dev( W_INSTR );
		if(--tout == 0) return EDISPLAY_TIMEOUT;
	}
	while(r & 0x80);
	return EDISPLAY_OK;
}

/* ------------------------------------------------------------------ */
int hd44xx_display::init_dev()
{

    int ret = EDISPLAY_OK;
    m_initialized = true;
    mdelay(50);
    //First initialization
    write8_i(0, 0x30);
    mdelay(10);
    write8_i(0, 0x30);
    mdelay(5);
    write8_i(0, 0x30);
    mdelay(5);
    write8_i(0, 0x20);
    mdelay(5);
    //4 bit bus 2 lines
    write_dev(0x28,W_INSTR);
    if((ret=wait4dev())<0) return ret;
    //Work mode
    write_dev(0x6,W_INSTR);
    if((ret=wait4dev())<0) return ret;
	//Enable display
    write_dev(0x0c,W_INSTR);
    if((ret=wait4dev())<0) return ret;
	//Clear display
    write_dev(1,W_INSTR);
    if((ret=wait4dev())<0) return ret;
    return ret;
}


/* ------------------------------------------------------------------ */
int hd44xx_display::setpos(int x, int y)
{
	x-=1;
	if(y==2) x |= 0x40;
	write_dev(0x80 | x, W_INSTR);
	return wait4dev();
}

/* ------------------------------------------------------------------ */
int hd44xx_display::show_icon(char chpos, const char *str)
{
	int ret;
	uint8_t tmp = read_currpos();
	write_dev( 0x40|((chpos & 7)<<3), W_INSTR );
	if((ret=wait4dev())<0) return ret;
	for(int i=0; i<8; i++)
	{
		write_dev( str[i], W_DATA );
		if((ret=wait4dev())<0) return ret;
	}
	write_dev( 0x80|tmp, W_INSTR );
	if((ret=wait4dev())<0) return ret;
	return putc( chpos & 7 );
}
/* ------------------------------------------------------------------ */
int hd44xx_display::clear()
{
	write_dev(1,W_INSTR);
	mdelay(2);
	return wait4dev();
}
/* ------------------------------------------------------------------ */
int hd44xx_display::read_currpos()
{
	  uint8_t r;
	  int tout = WAIT_DELAY_TOUT;
		//Sprawdzenie czy jest wolny wyswietlacz
	  do
	  {
	        r = read_dev( W_INSTR );
			if(--tout == 0) return EDISPLAY_TIMEOUT;
	  }
	  while(r & 0x80);
	  return r;
}
/* ------------------------------------------------------------------ */
int hd44xx_display::progress_bar(int x, int y, int width, char chpos)
{
    char cgram[8];
    int ret = 0;
    if((ret=setpos( x, y ))!=0) return ret;
    static const int full_fill_len = 5;
    static const char full_char = 0xff;
    for( int i = 0; i < width/full_fill_len; ++i )
        if((ret = putc( full_char ))!=0) return ret;
    for( unsigned i=0; i < sizeof(cgram)/sizeof(cgram[0]); ++i)
       cgram[i] = bar_patterns[ width % full_fill_len ];
    if((ret=show_icon( chpos, cgram ))!=0) return ret;
    return ret;
}
/* ------------------------------------------------------------------ */
hd44xx_display& operator<<(hd44xx_display &o,const char *str)
{

	while(*str)
		o.putc(*str++);
	return o;
}

/* ------------------------------------------------------------------ */
hd44xx_display& operator<<(hd44xx_display &o,unsigned value)
{
	char buf[11];
	fnd::fnd_uitoa(buf,value,1,'0',10);
	o << buf;
	return o;
}
/* ------------------------------------------------------------------ */
hd44xx_display& operator<<(hd44xx_display &o,int value)
{
	char buf[12];
	fnd::fnd_itoa(buf,value,1,'0');
	o << buf;
	return o;
}
/* ------------------------------------------------------------------ */
hd44xx_display& operator<<(hd44xx_display &o,const lfmt &fmt)
{
	char buf[11];
	fnd::fnd_uitoa(buf,fmt.val,fmt.fmt,fmt.fmtch,fmt.base);
	o << buf;
	return o;
}

/* ------------------------------------------------------------------ */
hd44xx_display& operator<<(hd44xx_display &o,const icon &fmt)
{
	o.show_icon(fmt.get_char(),fmt.get_data());
	return o;
}

/* ------------------------------------------------------------------ */
hd44xx_display& operator<<(hd44xx_display &o,const pos &fmt)
{
	o.setpos(fmt.x,fmt.y);
	return o;
}
/* ------------------------------------------------------------------ */
}
}



