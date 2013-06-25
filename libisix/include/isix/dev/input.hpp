/*
 * input.hpp
 *
 *  Created on: 25-06-2013
 *      Author: lucck
 */

#ifndef ISIX_DEV_INPUT_HPP_
#define ISIX_DEV_INPUT_HPP_
/* ------------------------------------------------------------------ */
namespace isix {
namespace dev {

/* ------------------------------------------------------------------ */
class input_class
{
public:
	/* Event type */
	enum event_type
	{
		EV_KEY,		/*Keyboard event */
		EV_REL,		/*Relative event */
		EV_ABS		/*Absolute event */
	};
	enum key_code
	{
		KEY_PRESS,
		KEY_RELEASE,
		KEY_REPEAT
	};
	enum rel_code
	{
		REL_X			,
		REL_Y			,
		REL_Z			,
		REL_RX			,
		REL_RY			,
		REL_RZ			,
		REL_HWHEEL		,
		REL_DIAL		,
		REL_WHEEL		,
		REL_MISC
	};
	enum abs_code
	{
		ABS_X			,
		ABS_Y			,
		ABS_Z			,
		ABS_RX			,
		ABS_RY			,
		ABS_RZ			,
		ABS_THROTTLE	,
		ABS_RUDDER		,
		ABS_WHEEL		,
		ABS_GAS			,
		ABS_BRAKE		,
		ABS_HAT0X		,
		ABS_HAT0Y		,
		ABS_HAT1X		,
		ABS_HAT1Y		,
		ABS_HAT2X		,
		ABS_HAT2Y		,
		ABS_HAT3X		,
		ABS_HAT3Y		,
		ABS_PRESSURE	,
		ABS_DISTANCE	,
		ABS_TILT_X		,
		ABS_TILT_Y		,
		ABS_TOOL_WIDTH	,
		ABS_MISC
	};
	struct event
	{
		tick_t time;
		unsigned short type;
		unsigned short code;
		unsigned value;
	};
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

	/* get device ID */
	virtual int get_device_id( id& /*id*/ ) const = 0;
	/* Is device operational */
	virtual int is_connected() const = 0;
	/* get repeat code */
	virtual int get_repeat_settings( int& /*delay */, int& /*period*/ ) const
	{
		return ISIX_ENOTSUP;
	}
	/* get repeat code */
	virtual int set_repeat_settings( int /*delay */, int /*period*/ )
	{
		return ISIX_ENOTSUP;
	}
	/* Get device name */
	virtual const char* get_device_name() const
	{
		return 0;
	}
	virtual int hardware_led_enable(bool /*yes*/)
	{
		return ISIX_ENOTSUP;
	}
	/* Get led status */
	virtual int get_led( led_ctl /*led_id */, bool& /*value*/ ) const
	{
		return ISIX_ENOTSUP;
	}
	/* set led status */
	virtual int set_led( led_ctl /*led_id */, bool /*value*/ )
	{
		return ISIX_ENOTSUP;
	}
	/* Read data from virtual device */
	virtual int read( event &ev, int timeout ) const = 0;
};

/* ------------------------------------------------------------------ */
}}
/* ------------------------------------------------------------------ */
#endif /* INPUT_HPP_ */
/* ------------------------------------------------------------------ */
