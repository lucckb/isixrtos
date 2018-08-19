/*
 * lcd_bitmap.hpp
 *
 *  Created on: 24 paź 2013
 *      Author: lucck
 */


#pragma once


namespace periph::display {

struct icon_t
{
	unsigned char pg_width;
	unsigned char height;
	const unsigned char* data;
};

//! Operator for determine the icon has diffrent size
static inline bool operator!=( const icon_t& i1, const icon_t& i2 )
{
	return (i1.pg_width != i2.pg_width) || (i1.height != i2.height );
}

//! Operator for determine the icon has diffrent size
static inline bool operator==( const icon_t& i1, const icon_t& i2 )
{
	return (i1.pg_width == i2.pg_width) && (i1.height == i2.height );
}

}

