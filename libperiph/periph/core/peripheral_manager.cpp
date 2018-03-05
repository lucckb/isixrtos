/*
 * peripheral_manager.cpp
 *
 *  Created on: 19 sty 2018
 *      Author: lucck
 */

#include <periph/peripheral_manager.hpp>
#include <functional>


namespace periph {

//Static create instance
peripheral_manager& peripheral_manager::instance()
{
	static peripheral_manager periph_mgr;
	return std::ref(periph_mgr);
}

}

