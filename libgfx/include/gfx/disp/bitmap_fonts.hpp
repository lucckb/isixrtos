/*
 * bitmap_fonts.hpp
 *
 *  Created on: 13-06-2013
 *      Author: lucck
 */
 
#pragma once
 
namespace gfx {
namespace disp {

 
/*
 * Proportional/fixed font structure.
 */
struct font_t {
        const char *    name;           /* font name*/
        int             maxwidth;       /* max width in pixels*/
        unsigned int    height;         /* height in pixels*/
        int             ascent;         /* ascent (baseline) height*/
        int             firstchar;      /* first character in bitmap*/
        int             size;           /* font size in characters*/
        const unsigned short *bits;     /* 16-bit right-padded bitmap data*/
        const unsigned short *offset;   /* offsets into bitmap data*/
        const unsigned char *width;     /* character widths or 0 if fixed*/
        int             defaultchar;    /* default char (not glyph index)*/
        long            bits_size;      /* # words of bits*/
};
 
//Available font lists
namespace fonts
{
	extern const font_t font_default;
	extern const font_t font_small;
}
 
}} //Namespace end
 
/* Howto generate ISO8859-2 font
 * convbdf -c -s 0x20 -l 0xF3 -x 0x80-0xA0 -x 0xA2-0xA2
 * -x 0xA4-0xA5 -x 0xA7-0xAB -x 0xAD-0xAE -x 0xB0-0xB0
 * -x 0xB2-0xB2 -x 0xB4-0xB5 -x 0xB7-0xBB -x 0xBD-0xBE
 * -x 0xC0-0xC5 -x 0xC7-0xC9 -x 0xCB-0xD0 -x 0xD2-0xD2
 * -x 0xD4-0xE5 -x 0xE7-0xE9 -x 0xEB-0xF0 -x 0xF2-0xF2 -o default.cpp default.bdf
 */
