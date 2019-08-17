/*
 * window_childs_container.hpp
 *
 *  Created on: 30 wrz 2013
 *      Author: lucck
 */
 
#pragma once
 
#include <gfx/types.hpp>
#include <list>
#include <memory>
#include <string>
#include <map>
 
namespace gfx {
namespace disp {
	class gdi;
}}
 
namespace gfx {
namespace gui {

 
//TODO: Constant strings allocator add
namespace detail {
//template <typename T> using windows_stack = std::list<T, __gnu_cxx::array_allocator<T, std::array<T,CONFIG_GFX_GUI_MAX_CHILD_WINDOWS> > >;
template <typename T> using container = std::list<T>;
using string = std::string;
template <typename K, typename V> using map = std::map<K,V>;

}
 
}
}

