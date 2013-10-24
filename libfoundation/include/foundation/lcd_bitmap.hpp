/*
 * lcd_bitmap.hpp
 *
 *  Created on: 24 paź 2013
 *      Author: lucck
 */

#ifndef FOUNDATION_LCD_BITMAP_HPP_
#define FOUNDATION_LCD_BITMAP_HPP_


/* ------------------------------------------------------------------ */
namespace fnd {
namespace lcd {
/* ------------------------------------------------------------------ */
struct icon_t
{
	unsigned char pg_width;
	unsigned char height;
	const unsigned char* data;
};

/* ------------------------------------------------------------------ */
}}
/* ------------------------------------------------------------------ */
#endif /* LCD_BITMAP_HPP_ */
