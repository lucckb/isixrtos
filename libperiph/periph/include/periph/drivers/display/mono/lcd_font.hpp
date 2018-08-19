/*
 * uc1601_font.hpp
 *
 *  Created on: 23 paź 2013
 *      Author: lucck
 */

#pragma once


namespace periph::display {

//Very simple font for lcd
struct font_t
{
	unsigned char height;
	char first_char;
	char last_char;
	unsigned char spc_width;
	struct chr_desc_t
	{
		unsigned char  width;
		unsigned short offset;
	} const *chr_desc;
	const unsigned char *bmp;
};


}
