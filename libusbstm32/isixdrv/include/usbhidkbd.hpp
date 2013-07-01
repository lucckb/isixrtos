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
#include <gfx/input/input.hpp>
#include <memory>
#include <usb_host.hpp>
#include <usb_device.hpp>
#include <functional>
#include <array>
/* ------------------------------------------------------------------ */
namespace stm32 {
namespace dev {

/* ------------------------------------------------------------------ */
class hid_keyboard	: public usb_input_device
{
public:
	hid_keyboard( isix::dev::usb_host &host)
		: usb_input_device( host, isix::dev::device::cid_input_usb_kbd )
	{}
	virtual ~hid_keyboard() {}
	/* Read data from KBD */
 	virtual int get_repeat_settings( int& /*delay */, int& /*period*/ ) const;
 	virtual int set_repeat_settings( int /*delay */, int /*period*/ );
 	virtual int hardware_led_enable(bool /*yes*/);
 	virtual int get_led( led_ctl /*led_id */, bool& /*value*/ ) const;
 	virtual int set_led( led_ctl /*led_id */, bool /*value*/ );
protected:
 	void generate_event( const uint8_t */*req*/, std::size_t /*len*/ );
private:
 	static constexpr size_t KEY_REPORT_LEN = 6;
 	std::array<uint8_t, KEY_REPORT_LEN>   m_keys_last  {{}};
 	uint8_t   m_nbr_keys_last {};
};
/* ------------------------------------------------------------------ */
}}
/* ------------------------------------------------------------------ */
#endif /* USBHOST_H_ */
