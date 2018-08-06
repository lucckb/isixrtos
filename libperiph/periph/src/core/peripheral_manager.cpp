/*
 * peripheral_manager.cpp
 *
 *  Created on: 19 sty 2018
 *      Author: lucck
 */

#include <periph/core/peripheral_manager.hpp>
#include <periph/core/error.hpp>
#include <periph/core/device.hpp>
#include <algorithm>

namespace periph {

//Static create instance
peripheral_manager& peripheral_manager::instance()
{
	static peripheral_manager periph_mgr;
	return std::ref(periph_mgr);
}


int peripheral_manager::register_driver(const char name[], factory_fn driver_factory)
{
	isix::mutex_locker lck(m_mtx);
	const auto it = std::find(std::begin(m_devs),std::end(m_devs),name);
	if(it!=std::end(m_devs)) {
		return error::exists;
	}
	m_devs.push_front( {name,driver_factory} );
	return error::success;
}

peripheral_manager::device_ptr peripheral_manager::access_device(const char name[])
{
	isix::mutex_locker lck(m_mtx);
	const auto it = std::find(std::begin(m_devs),std::end(m_devs),name);
	if(it==std::end(m_devs)) {
		return nullptr;
	}
	if(auto spt=it->dev.lock()) {
		return spt;
	} else {
		auto cpt = (it->driver_factory)();
		it->dev = cpt;
		return cpt;
	}
}

}
