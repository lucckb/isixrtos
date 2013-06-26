/*
 * usbhost.h
 *
 *  Created on: 20-06-2013
 *      Author: lucck
 */

#ifndef LIBISIX_DRV_USBHIDKBD_HPP_
#define LIBISIX_DRV_USBHIDKBD_HPP_
/* ------------------------------------------------------------------ */
#include <stddef.h>
#include <isix/dev/input.hpp>
#include <memory>
/* ------------------------------------------------------------------ */
namespace stm32 {
namespace dev {
/* ------------------------------------------------------------------ */
typedef unsigned char usb_dev_id;

/* ------------------------------------------------------------------ */
/* Initialize USB bus only one device allowed*/
int usb_bus_initialize( );
/* ------------------------------------------------------------------ */
/* Get current assigned device */
std::shared_ptr<isix::dev::device> usb_get_device_in_slot();
/* ------------------------------------------------------------------ */
class hid_keyboard	: public isix::dev::input_class
{
public:
	hid_keyboard()
		: input_class(isix::dev::device::cid_input_usb_kbd )
	{}
	virtual ~hid_keyboard() {}
	/* Get device identifier */
	virtual int get_device_id( isix::dev::input_class::id& id ) const;
	/* Read data from KBD */
	virtual int read( void* buf, std::size_t len, int timeout );
 	virtual int open( int flags );
 	virtual int get_repeat_settings( int& /*delay */, int& /*period*/ ) const;
 	virtual int set_repeat_settings( int /*delay */, int /*period*/ );
 	virtual int hardware_led_enable(bool /*yes*/);
 	virtual int get_led( led_ctl /*led_id */, bool& /*value*/ ) const;
 	virtual int set_led( led_ctl /*led_id */, bool /*value*/ );
protected:
 	virtual void generate_event( const uint8_t */*req*/, std::size_t /*len*/ );
};

/* ------------------------------------------------------------------ */
}}
/* ------------------------------------------------------------------ */
#endif /* USBHOST_H_ */
