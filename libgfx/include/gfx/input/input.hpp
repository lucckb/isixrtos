/*
 * input.hpp
 *
 *  Created on: 25-06-2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#ifndef ISIX_DEV_INPUT_HPP_
#define ISIX_DEV_INPUT_HPP_
/* ------------------------------------------------------------------ */
#include <isix/dev/device.hpp>
#include <cstring>
#include <memory>
#include <array>
#include <isix.h>
#include "event_info.hpp"
/* ------------------------------------------------------------------ */
namespace gfx {
namespace input {

/* ------------------------------------------------------------------ */

//Input queue type
typedef isix::fifo<gui::event_info> input_queue_t;

/* ------------------------------------------------------------------ */
class input_class : public isix::dev::device
{
public:
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
	input_class( device::class_id cl )
		: device( cl )
	{}
	virtual ~input_class() {}

	/* get repeat code */
	virtual int get_repeat_settings( int& /*delay */, int& /*period*/ ) const
	{
		return isix::ISIX_ENOTSUP;
	}
	/* get repeat code */
	virtual int set_repeat_settings( int /*delay */, int /*period*/ )
	{
		return isix::ISIX_ENOTSUP;
	}
	/* Get device name */
	const char* get_device_name() const
	{
		return m_desc;
	}
	virtual int hardware_led_enable(bool /*yes*/)
	{
		return isix::ISIX_ENOTSUP;
	}
	/* Get led status */
	virtual int get_led( led_ctl /*led_id */, bool& /*value*/ ) const
	{
		return isix::ISIX_ENOTSUP;
	}
	/* set led status */
	virtual int set_led( led_ctl /*led_id */, bool /*value*/ )
	{
		return isix::ISIX_ENOTSUP;
	}
	/* Set input queue */
	void set_queue( input_queue_t *queue )
	{
		m_queue = queue;
	}
protected:
		int input_report_key(  gui::detail::keyboard_tag::status status ,
				gui::detail::keyboard_tag::key_type key,
				gui::detail::keyboard_tag::control_key_type ctl  );
private:
 	char m_desc[32] {};
 	input_queue_t * m_queue {};
};


/* ------------------------------------------------------------------ */
}}
/* ------------------------------------------------------------------ */
#endif /* INPUT_HPP_ */
/* ------------------------------------------------------------------ */
