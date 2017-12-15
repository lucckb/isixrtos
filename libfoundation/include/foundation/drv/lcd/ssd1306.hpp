/*
 * =====================================================================================
 *
 *       Filename:  ssd1306.hpp
 *
 *    Description:  ssd1306 controller driver
 *
 *        Version:  1.0
 *        Created:  12/14/2017 10:53:13 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once
#include <foundation/drv/bus/ibus.hpp>

namespace fnd {
namespace drv {
namespace lcd {


class ssd1306
{
public:
	/** Create SSD1306 display
	 * @param[in] bus Bus object
	 * @param[in] cols Number of cols
	 * @param[in] rows Number of rows
	 */
	ssd1306( bus::ibus& bus, uint8_t cols, uint8_t rows )
		: m_bus(bus)
	{}
	~ssd1306() {}
	ssd1306(ssd1306&) = delete;
	ssd1306& operator=(ssd1306&) = delete;

private:
	/* data */
	bus::ibus& m_bus;
};

}
}
}


