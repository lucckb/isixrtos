#ifndef _USBH_HID_CORE_H
#define _USBH_HID_CORE_H 1

#include <usb/core/usb_def.h>

#ifdef __cplusplus
extern "C" {
#endif



//! Hid state machine context
struct usbh_hid_context;
typedef struct usbh_hid_context usbh_hid_context_t;

//!Report callback 
//! Null pointer and len 0 means disconnection event
typedef void (*usbh_hid_report_callback_t)( usbh_hid_context_t* ctx, 
		void *user_data, const uint8_t *rptin, uint8_t rptin_len );

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
 * @param[in] ifc_no Interface number used in requests
 * @param[in] min_len Minimum length in report
 * @param[in] ep_desc Endpoint descriptor
 * @param[in] ep_count Number of endpoint
 * @param[in] callback Report request callback
 * @param[in] user data pointer for callback
 * @return Error code
 */
int usbh_hid_set_machine( usbh_hid_context_t *hid_ctx,
				  usb_speed_t speed, uint8_t dev_addr,
				  uint8_t ifc_no, uint16_t min_len,
                  usb_endpoint_descriptor_t const *ep_desc, unsigned ep_count, 
				  usbh_hid_report_callback_t callback, void *user_data );

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

/**  Send report allowed only in interruppt contest 
 *  @param[in,out] hid_ctx Hid context
 *  @param[in] rptout Report pointer
 *  @param[in] size  Report size
 */
int usbh_hid_sent_report( usbh_hid_context_t* hid_ctx, const uint8_t *rptout, uint16_t size );


#ifdef __cplusplus
}
#endif

#endif
