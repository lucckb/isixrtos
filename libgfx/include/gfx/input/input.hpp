/*
 * input.hpp
 *
 *  Created on: 25-06-2013
 *      Author: lucck
 */
 
#pragma once

#include <cstring>
#include <memory>
#include <array>
#include <functional>
#include "event_info.hpp"

namespace gfx::input {

class input_class 
{
public:
	typedef std::function<void( const event_info& )> input_evt_t;
	enum led_ctl
	{
		LED_NUML,
		LED_CAPSL,
		LED_SCROLLL
	};
	enum bus_type
	{
		BUS_GPIO,
		BUS_USB = 3
	};
	struct id
	{
		unsigned short bustype;
		unsigned short vendor;
		unsigned short product;
		unsigned short version;
	};
	/*
	 * @value: latest reported value for the axis.
	 * @minimum: specifies minimum value for the axis.
	 * @maximum: specifies maximum value for the axis.
	 * @fuzz: specifies fuzz value that is used to filter noise from
	 *	the event stream.
	 * @flat: values that are within this value will be discarded by
	 *	joydev interface and reported as 0 instead.
	 * @resolution: specifies resolution for the values reported for
	 *	the axis.
	 *
	 * Note that input core does not clamp reported values to the
	 * [minimum, maximum] limits, such task is left to userspace.
	 *
	 * Resolution for main axes (ABS_X, ABS_Y, ABS_Z) is reported in
	 * units per millimeter (units/mm), resolution for rotational axes
	 * (ABS_RX, ABS_RY, ABS_RZ) is reported in units per radian.
	 */
	struct absinfo {
		int value;
		int minimum;
		int maximum;
		int fuzz;
		int flat;
		int resolution;
	};
	input_class()
	{}
	virtual ~input_class() {}

	/** get repeat code 
        @param[in] delay Delay value
        @param[in] period Period value
        @return Error codes
    */
	virtual int get_repeat_settings( int& /*delay */, int& /*period*/ ) const
	{
		//FIXME: Todo
		return -1;
	}
	/** get repeat code 
        @param[in] delay Delay value
        @param[in] period Period value
    */
	virtual int set_repeat_settings( int /*delay */, int /*period*/ )
	{
		//FIXME: Todo
		return -1;
	}
	/** Get device name 
        @return Return device descriptor
    */
	const char* get_device_name() const
	{
		return m_desc;
	}
    /** Enable disable hardware led control
        @param[in] yes Enable hardware flow control
        @return Error code */
	virtual int hardware_led_enable(bool /*yes*/)
	{
		//FIXME: Todo
		return -1;
	}
	/* Get led status */
	virtual int get_led( led_ctl /*led_id */, bool& /*value*/ ) const
	{
		//FIXME: Todo
		return -1;
	}
	/* set led status */
	virtual int set_led( led_ctl /*led_id */, bool /*value*/ )
	{
		//FIXME: Todo
		return -1;
	}
	/* Set input queue */
	void connect( input_evt_t evt )
	{
		m_evt_report = evt;
	}
protected:
		int input_report_key(  detail::keyboard_tag::status status ,
				detail::keyboard_tag::key_type key,
				detail::keyboard_tag::control_key_type ctl  );
private:
 	char m_desc[32] {};
 	input_evt_t m_evt_report;
};


}