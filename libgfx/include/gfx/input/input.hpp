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
/* ------------------------------------------------------------------ */
namespace gfx {
namespace inp {
/* ------------------------------------------------------------------ */
class input_class;

/* ------------------------------------------------------------------ */
namespace input {
/* ------------------------------------------------------------------ */
/* Event type */
enum event_type
{
	EV_SW,		/* Plug unplug event */
	EV_KEY,		/* Keyboard event  */
	EV_REL,		/* Relative event  */
	EV_ABS		/* Absolute event  */
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
	isix::tick_t time;
	unsigned short type;
	unsigned short code;
	union
	{
		struct
		{
			unsigned char norm;
			union
			{
				unsigned char ctrl;
				struct
				{
					unsigned short code;
					unsigned char lctrl:1;
					unsigned char lshift:1;
					unsigned char lalt : 1;
					unsigned char lgui : 1;
					unsigned char rctrl: 1;
					unsigned char rshift:1;
					unsigned char ralt:1;
					unsigned char rgui: 1;

				} ctrlbits;
			};

		} key;
	};
};

/* ------------------------------------------------------------------ */
}	//Input internal namespace
/* ------------------------------------------------------------------ */
//Input queue type
typedef isix::fifo<input::event> input_queue_t;

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
	/* get device ID */
	const id& get_device_id() const
	{
		return m_identifier;
	}
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
	void set_identifier_callback( gfx::inp::input_class::id &id )
	{
		m_identifier = id;
	}
	void set_desc_callback( const char *desc )
	{
		std::strncpy( m_desc, desc, sizeof m_desc );
	}
	int input_report_key( input::key_code code, unsigned char key, unsigned char skeys );
private:
 	gfx::inp::input_class::id m_identifier;
 	char m_desc[32] {};
 	input_queue_t * m_queue {};
};


/* ------------------------------------------------------------------ */
}}
/* ------------------------------------------------------------------ */
#endif /* INPUT_HPP_ */
/* ------------------------------------------------------------------ */
