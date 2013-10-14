/*
 * editbox.hpp
 *
 *  Created on: 14 paź 2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#ifndef GFX_GUI_EDITBOX_HPP_
#define GFX_GUI_EDITBOX_HPP_
/* ------------------------------------------------------------------ */
#include <gfx/gui/widget.hpp>
/* ------------------------------------------------------------------ */
namespace gfx {
namespace gui {
/* ------------------------------------------------------------------ */
/* Edit box widget for editing values */
class editbox: public widget
{
public:
	//Destructor
	virtual ~editbox()
	{}
	enum class type	 : char
	{
		floating_pos,		//Floating point type
		floating_neg,		//Negative floating point
		integer_pos,		//Integer positive
		integer_neg,		//Integer negative
		text,				//Text edit
		text_small,			//Text small letter
		text_large			//Text large letters
	};
	//Get value
	template <typename T> T get() const;
	//Set mask character
	void mask( char mask_ch = '*');
};
/* ------------------------------------------------------------------ */

} /* namespace gui */
} /* namespace gfx */

#endif /* EDITBOX_HPP_ */
/* ------------------------------------------------------------------ */
