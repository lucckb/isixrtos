/*
 * peripheral_manager.hpp
 *
 *  Created on: 19 sty 2018
 *      Author: lucck
 */

#pragma once

#include <forward_list>
#include <memory>
#include <cstring>
#include <functional>
#include <isix.h>

namespace periph {
	class device;


	//! Peripheral manager class
	class peripheral_manager {
	public:
		using device_ptr = std::shared_ptr<device>;
		using factory_fn = device_ptr(*)();
	private:
		struct ditem {
			ditem(const char* _name,factory_fn _driver_factory)
				: driver_factory(_driver_factory)
			{
				std::strncpy(name,_name,sizeof name);
			}
			std::weak_ptr<device> dev;
			factory_fn driver_factory;
			char name[8] {};
		};
		friend bool operator==(const peripheral_manager::ditem& item, const char* const name);
		using container = std::forward_list<ditem>;
	public:
		//! Peripheral manager instance
		static peripheral_manager& instance();
		//! Non copy able
		peripheral_manager(peripheral_manager&) = delete;
		peripheral_manager& operator=(peripheral_manager&) = delete;
		//! Default constructor
		peripheral_manager() {
		}
		int register_driver(const char name[], factory_fn driver_factory);
		device_ptr access_device(const char name[]);
	private:
		container m_devs;
		isix::mutex m_mtx;
	};
	inline bool operator==(const peripheral_manager::ditem& item, const char* const name) {
		return !std::strcmp(item.name,name);
	}
}

