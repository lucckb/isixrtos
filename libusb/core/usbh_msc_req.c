#include <usbh_core.h>
#include <usbh_error.h>
#include <usbh_msc_req.h>
#include <usbh_std_req.h>

int MSCgetMaxLun(int synch, uint8_t iface, uint8_t *max_lun) {
  usb_setup_packet_t setup;

  setup.bmRequestType = DEVICE_TO_HOST | INTERFACE_RECIPIENT |
                        CLASS_REQUEST;
  setup.bRequest = MSC_GET_MAX_LUN;
  setup.wValue = 0;
  setup.wIndex = iface;
  setup.wLength = 1;

  return USBHcontrolRequest(synch, &setup, max_lun, 0);
}

int MSCresetBOT(int synch, uint8_t iface) {
  usb_setup_packet_t setup;

  setup.bmRequestType = HOST_TO_DEVICE | INTERFACE_RECIPIENT |
                        CLASS_REQUEST;
  setup.bRequest = MSC_BULK_ONLY_RESET;
  setup.wValue = 0;
  setup.wIndex = iface;
  setup.wLength = 0;

  return USBHcontrolRequest(synch, &setup, 0, 0);
}

int MSCresetRecovery(int synch, uint8_t interface_number,
                     uint8_t in_ep_addr, uint8_t out_ep_addr) {
  int res;

  if (synch) {
    res = MSCresetBOT(1, interface_number);
    if (res != USBHLIB_SUCCESS)
      return res;
    res = USBHclearEndpointHalt(1, in_ep_addr);
    if (res != USBHLIB_SUCCESS)
      return res;
    res = USBHclearEndpointHalt(1, out_ep_addr);
    if (res != USBHLIB_SUCCESS)
      return res;
  }
  else {
    /* TODO: Implementation of the asynchronous reset recovery */
    return USBHLIB_ERROR_NOT_SUPPORTED;
  }
  return USBHLIB_SUCCESS;
}
