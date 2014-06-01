#include <usb/core/usbh_configure.h>
#include <usb/core/usbh_core.h>
#include <usb/core/usbh_error.h>
#include <usb/core/usbh_lib.h>
//TODO: Fix it platform independent
#include <usb/drivers/controllers/stm32/timer.h>
#include <isix.h>

int usbh_open_device(usb_speed_t *speed, uint8_t *dev_addr,
                   usb_device_descriptor_t *dev_desc, unsigned timeout) {
    return usbh_get_device(speed, dev_addr, dev_desc, timeout );
}

int usbh_is_device_ready() {
  /* Check, if a non-zero address is assigned to the device. */
  return (usbh_get_visible_device_state() & ADDRESS) == ADDRESS;
}

static void DeviceOn(void* p) {
  (void)p;
  USBHvbus(1); /* Switch the device back on. */
}

void usbh_device_hard_reset(unsigned time) {
  (void)time;
  USBHvbus(0); /* Force the device to switch off. */
	usblibp_timer_start(2, DeviceOn, time);
}
