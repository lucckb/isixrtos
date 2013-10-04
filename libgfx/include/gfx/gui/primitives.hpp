/* ------------------------------------------------------------------ */
/*
 * primitives.hpp
 *
 *  Created on: 3 paź 2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#ifndef GFX_GUI_PRIMITIVES_HPP_
#define GFX_GUI_PRIMITIVES_HPP_
/* ------------------------------------------------------------------ */
#include <gfx/types.hpp>
#include <gfx/disp/bitmap_fonts.hpp>
/* ------------------------------------------------------------------ */
namespace gfx {
namespace gui {
/* ------------------------------------------------------------------ */
using font_t = disp::font_t;
/* ------------------------------------------------------------------ */
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

/* ------------------------------------------------------------------ */
//Basic layout class
class layout
{
public:
	layout(color_t fg, color_t bg, color_t sel,const font_t* const font = &disp::fonts::font_default)
		: mbg(bg), mfg(fg), msel(sel),mfont(font),  minherited(false)
	{}
	layout() {}
	bool inherit() const { return minherited; }
	color_t bg() const { return mbg; }
	color_t fg() const { return mfg; }
	color_t sel() const { return msel; }
	const font_t* font() const { return mfont; }
private:
	color_t mbg{}, mfg{}, msel{};
	const font_t* mfont { &disp::fonts::font_default };
	bool minherited { true };
};

/* ------------------------------------------------------------------ */
}}

#endif /* PRIMITIVES_HPP_ */
