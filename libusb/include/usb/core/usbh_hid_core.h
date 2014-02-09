#ifndef _USBH_HID_CORE_H
#define _USBH_HID_CORE_H 1

#include <usb/core/usb_def.h>

#ifdef __cplusplus
extern "C" {
#endif



//! Hid state machine context
struct usbh_hid_context;
typedef struct usbh_hid_context usbh_hid_context_t;

/**
 *  Create new hid context
 *  @return New hid context
 */
usbh_hid_context_t* usbh_hid_core_new_ctx(void);

/** 
 * Setup hid state machine
 * @param[in,out] hid_ctx HID controller context
 * @param[in] speed USB device speed
 * @param[in] dev_addr USB debice address
 * @param[in] if_desc Interface Descriptor
 * @param[in] hid_desc HID main descriptor
 * @param[in] ep_desc Endpoint descriptor
 * @param[in] ep_count Number of endpoint
 * @return Error code
 */
int usbh_hid_set_machine( usbh_hid_context_t *hid_ctx,
				  usb_speed_t speed, uint8_t dev_addr,
                  usb_interface_descriptor_t const *if_desc,
                  usb_hid_main_descriptor_t const *hid_desc,
                  usb_endpoint_descriptor_t const *ep_desc,
                  unsigned ep_count);

/** Return positive value if device is ready
 *  @param[in] hid_ctx HID context
 *  @return Error code
 */
int usbh_hid_is_device_ready(const usbh_hid_context_t* hid_ctx);
/** Return hid machine error code
 *  @param[in,out] HID context
 *  @return Error code
 */
int usbh_hid_error(const usbh_hid_context_t* hid_ctx);

#ifdef __cplusplus
}
#endif

#endif
