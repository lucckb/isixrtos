/*
 * usb_device.hpp
 *
 *  Created on: 27-06-2013
 *      Author: lucck
 */

#ifndef ISIX_USB_DEVICE_HPP_
#define ISIX_USB_DEVICE_HPP_

/* ------------------------------------------------------------------ */
namespace isix {
namespace dev {
/* ------------------------------------------------------------------ */
class usb_host;

/* ------------------------------------------------------------------ */
class usb_device : public device
{
public:
	usb_device( usb_host & host,  device::class_id cid )
		: device( cid ), m_host( host )
	{}
private:
	usb_host& m_host;
};

/* ------------------------------------------------------------------ */
}}
/* ------------------------------------------------------------------ */
#endif /* USB_DEVICE_HPP_ */
