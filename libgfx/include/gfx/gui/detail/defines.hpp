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
#include <deque>
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
namespace detail {
//template <typename T> using windows_stack = std::list<T, __gnu_cxx::array_allocator<T, std::array<T,CONFIG_GFX_GUI_MAX_CHILD_WINDOWS> > >;
template <typename T> using windows_container = std::deque<T>;

}
/* ------------------------------------------------------------------ */
}
}

/* ------------------------------------------------------------------ */
#endif /* GFX_GUI_DETAIL_DEFINES_HPP_ */
