#include <usb/core/usbh_configure.h>
#include <usb/core/usbh_core.h>
#include <usb/core/usbh_error.h>
#include <usb/core/usbh_lib.h>
//TODO: Fix it platform independent
#include <usb/drivers/controllers/stm32/timer.h>

int USBHopenDevice(usb_speed_t *speed, uint8_t *dev_addr,
                   usb_device_descriptor_t *dev_desc, int timeout) {
  while (--timeout >= 0)
    if (USBHgetDevice(speed, dev_addr, dev_desc) == USBHLIB_SUCCESS)
      return USBHLIB_SUCCESS;
  return USBHLIB_ERROR_TIMEOUT;
}

int USBHisDeviceReady() {
  /* Check, if a non-zero address is assigned to the device. */
  return (USBHgetVisibleDeviceState() & ADDRESS) == ADDRESS;
}

static void DeviceOn(void) {
  USBHvbus(1); /* Switch the device back on. */
}

void USBHdeviceHardReset(unsigned time) {
  (void)time;
  USBHvbus(0); /* Force the device to switch off. */
TimerStart(2, DeviceOn, time);
}
