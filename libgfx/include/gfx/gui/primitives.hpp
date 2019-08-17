 
/*
 * primitives.hpp
 *
 *  Created on: 3 paź 2013
 *      Author: lucck
 */
 
#pragma once
 
#include <gfx/types.hpp>
#include <gfx/disp/bitmap_fonts.hpp>
#include <gfx/disp/static_bitmap.hpp>
#include <algorithm>
#include <utility>
 
namespace gfx {
namespace gui {
 
using font_t = disp::font_t;
using bitmap_t = disp::cmem_bitmap_t;
 
//Basic rectangle class
class rectangle
{
public:
	rectangle(coord_t x, coord_t y, coord_t cx, coord_t cy )
		: mx(x), my(y), mcx(cx), mcy(cy)
	{}
	coord_t x() const { return mx; }
	coord_t y() const { return my; }
	coord_t cx() const { return mcx; }
	coord_t cy() const { return mcy; }
	bool empty() const { return mcx==0||mcy==0; }
private:
	const coord_t mx{},my{}, mcx {}, mcy{};
};


 
//Basic layout class
class layout {
public:
	layout( color_t fg, color_t bg, color_t sel = 0, const font_t* const font = &disp::fonts::font_default )
		: mbg(bg), mfg(fg), msel(sel),mfont(font), minherited(false)
	{}
	layout() {}
	bool inherit() const { return minherited; }
	color_t bg() const { return mbg; }
	void bg( color_t color ) { mbg = color; }
	color_t fg() const { return mfg; }
	void fg( color_t color ) { mfg = color; }
	color_t sel() const { return msel; }
	void sel( color_t color ) { msel = color; }
	const font_t* font() const { return mfont; }
private:
	color_t mbg{}, mfg{}, msel{};
	const font_t* mfont { &disp::fonts::font_default };
	bool minherited { true };
};

 
//Operator - beetween two windows
static inline rectangle operator+( const rectangle &r1, const rectangle &r2 )
{
	return std::move(  
		rectangle( r1.x()+r2.x(), r1.y()+r2.y(), 
			std::min(r1.cx(),r2.cx()), std::min(r1.cy(),r2.cy()) ) 
	);
}
 
//Operator - beetween two windows
static inline rectangle operator+( const rectangle &r1, coord_t size )
{
	if( r1.x() < size ) {
		size = r1.x();
	}
	if( r1.y() < size ) {
		size = r1.y();
	}
	return std::move( 
		rectangle(r1.x()-size, r1.y()-size, 
		r1.cx()+size, r1.cy()+size) 
	);
}
 
}}

