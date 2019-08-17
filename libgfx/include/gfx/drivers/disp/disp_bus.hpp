/*
 * dev_bus.hpp
 * Device bus file containts the bus interface
 *  Created on: 10-06-2013
 *      Author: lucck
 */

#ifndef ISIX_GFX_DISP_BUS_HPP_
#define ISIX_GFX_DISP_BUS_HPP_
 
#include <cstddef>
#include <cstdint>
 
namespace gfx {
namespace drv {
 
class disp_bus
{
	//Make object noncopyable
	disp_bus(const disp_bus&) = delete;
	disp_bus& operator=(const disp_bus&) = delete;
public:
	//Constructor
	disp_bus() {}
	//Destructor
	virtual ~disp_bus() {}
	// Lock bus and set addres
	virtual void set_ctlbits( int bit, bool val ) = 0;
	/* Read transfer */
	virtual void read( void *buf, std::size_t len ) =   0;
	/* Write transfer */
	virtual void write( const void *buf, size_t len ) = 0;
	/* Fill the value with native order */
	virtual void fill( unsigned value, size_t nelms ) = 0;
	/* Wait ms long delay */
	virtual void delay( unsigned timeout ) = 0;
	/* Set PWM value */
	virtual void set_pwm( int percent ) = 0;
};

 
}}
 
#endif /* DEV_BUS_HPP_ */
