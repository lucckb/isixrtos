/*
 * abstract_gpio.hpp
 *
 *  Created on: 01-12-2012
 *      Author: lucck
 */
/*----------------------------------------------------------*/
#ifndef IMMC_DET_PIN_HPP_
#define IMMC_DET_PIN_HPP_
/*----------------------------------------------------------*/
#include <foundation/algo/noncopyable.hpp>

/*----------------------------------------------------------*/
namespace drv {
namespace mmc {
/*----------------------------------------------------------*/
/* Abstraction layer for gpio class */
class immc_det_pin	: private fnd::noncopyable
{
public:
	virtual ~immc_det_pin() {
	}
	virtual bool get() const = 0;
};
/*----------------------------------------------------------*/
}
}
/*----------------------------------------------------------*/
#endif /* IMMC_DET_PIN_HPP_ */
