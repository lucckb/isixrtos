/*
 * usb_host.hpp
 *
 *  Created on: 27-06-2013
 *      Author: lucck
 */

#ifndef ISIX_USB_HOST_HPP_
#define ISIX_USB_HOST_HPP_
/* ------------------------------------------------------------------ */
#include <usbh_core.h>
#include <usbh_hid_core.h>
#include <isix.h>
#include <isix/dev/device.hpp>
#include <memory>
#include <functional>
/* ------------------------------------------------------------------ */
namespace isix {
namespace dev {
extern "C"
{
	void __attribute__((__interrupt__)) otg_fs_isr_vector(void);
}
/* ------------------------------------------------------------------ */
class usb_device;
/* ------------------------------------------------------------------ */
class usb_host	: public isix::task_base
{
	friend void  otg_fs_isr_vector(void);
public:
	explicit usb_host( int core_id );
	virtual ~usb_host() {}
	std::shared_ptr<usb_device> get_slot();
private:
	virtual void main();
public:	//TODO FIXME very ugly to C lib
	USB_OTG_CORE_HANDLE    usb_otg_dev;
	USBH_HOST stm32_host;
	void on_device_event_gen( bool connect, std::shared_ptr<usb_device> dev )
	{
		if( m_on_device_event ) m_on_device_event( connect, dev );
	}
private:
	isix::semaphore usb_ready_sem;
 	std::function<void( bool, std::shared_ptr<usb_device> )> m_on_device_event;
};

/* ------------------------------------------------------------------ */

} /* namespace dev */
} /* namespace isix */
/* ------------------------------------------------------------------ */
#endif /* USB_HOST_HPP_ */
/* ------------------------------------------------------------------ */
