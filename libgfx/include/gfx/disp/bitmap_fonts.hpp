/*
 * bitmap_fonts.hpp
 *
 *  Created on: 13-06-2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#ifndef GFX_BITMAP_FONTS_HPP_
#define GFX_BITMAP_FONTS_HPP_
/* ------------------------------------------------------------------ */
namespace gfx {
namespace disp {

/* ------------------------------------------------------------------ */
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
/* ------------------------------------------------------------------ */
//Available font lists
namespace fonts
{
	extern const font_t font_default;
}
/* ------------------------------------------------------------------ */
}} //Namespace end
/* ------------------------------------------------------------------ */
#endif /* BITMAP_FONTS_HPP_ */
/* ------------------------------------------------------------------ */
