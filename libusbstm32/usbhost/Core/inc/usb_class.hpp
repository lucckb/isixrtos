/*
 * usb_class.hpp
 *
 *  Created on: 24-06-2013
 *      Author: lucck
 */
#ifndef __cplusplus
#error C++ only header
#endif

/* -------------------------------------------------------------------------- */
#ifndef USB_CLASS_HPP_
#define USB_CLASS_HPP_
/* -------------------------------------------------------------------------- */
#include <usbh_core.h>
/* -------------------------------------------------------------------------- */
class usb_class_base
{
public:
	/* USB host init */
	virtual USBH_Status init()
	{}
	/* USB host deinit */
	virtual void deinit()
	{}
	/* USB host request */
	virtual void requests()
	{}
	/* USB host machine */
	virtual void machine()
	{}
private:
	USBH_class_ctx &m_control;
};

/* -------------------------------------------------------------------------- */

#endif /* USB_CLASS_HPP_ */
