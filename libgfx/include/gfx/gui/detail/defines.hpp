/*
 * window_childs_container.hpp
 *
 *  Created on: 30 wrz 2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#ifndef GFX_GUI_DETAIL_DEFINES_HPP_
#define GFX_GUI_DETAIL_DEFINES_HPP_
/* ------------------------------------------------------------------ */
#include <gfx/types.hpp>
#include <list>
#include <memory>
/* ------------------------------------------------------------------ */
namespace gfx {
namespace disp {
	class gdi;
}}

/* ------------------------------------------------------------------ */

namespace gfx {
namespace gui {
/* ------------------------------------------------------------------ */
enum class keyboard : char
{
	//Control Code for ASCII
	select_all	= 0x1,
	copy		= 0x3,		//Ctrl+C
	backspace	= 0x8,	tab		= 0x9,
	enter_n		= 0xA,	enter	= 0xD,	enter_r = 0xD,
	alt			= 0x12,
	paste		= 0x16,		//Ctrl+V
	cut			= 0x18,		//Ctrl+X
	escape		= 0x1B,
	//System Code for OS
	os_pageup		= 0x21,	os_pagedown,
	os_arrow_left	= 0x25, os_arrow_up, os_arrow_right, os_arrow_down,
	os_insert		= 0x2D, os_del
};
/* ------------------------------------------------------------------ */
enum class mouse : char
{
	any_button, left_button, middle_button, right_button
};

/* ------------------------------------------------------------------ */
namespace detail {
//template <typename T> using windows_stack = std::list<T, __gnu_cxx::array_allocator<T, std::array<T,CONFIG_GFX_GUI_MAX_CHILD_WINDOWS> > >;
template <typename T> using windows_container = std::list<T>;

}
/* ------------------------------------------------------------------ */
}
}

/* ------------------------------------------------------------------ */
#endif /* GFX_GUI_DETAIL_DEFINES_HPP_ */
