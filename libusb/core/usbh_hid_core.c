#include <string.h>
#include <stdlib.h>
#include <usb/core/usbh_core.h>
#include <usb/core/usbh_error.h>
#include <usb/core/usbh_hid_core.h>
#include <usb/core/usbh_hid_req.h>
#include <usb/core/usbh_io.h>
#include <usb/core/usbh_std_req.h>

typedef enum {
	HID_EXIT = 0, /* usbh_hid_is_device_ready must return 0, if HID machine is not configured. */
	HID_INIT,
	HID_POLL_IN,
	HID_POLL_IN_WAIT,
	HID_STALL_IN,
	HID_REPORT_OUT
} usbh_hid_state_t;

struct usbh_hid_context {
	usb_speed_t      speed;
	usbh_hid_state_t state;
	int              errno;
	usb_pid_t        pid_in;
	int              ch_num_in;
	unsigned         timer_in;
	usbh_hid_report_callback_t callback;
	void*			 user_data;
	uint16_t         length_in;
	uint16_t		 length_out;
	uint16_t         min_length_in;
	uint8_t          dev_addr;
	uint8_t          iface;
	uint8_t          ep_addr_in;
	uint8_t          interval_in;
	uint8_t          buffer_out[MAX_FS_INTERRUPT_PACKET_SIZE];
	uint8_t          buffer_in[MAX_FS_INTERRUPT_PACKET_SIZE];
};

/** Full implementation of the HID core machine **/

static void HIDfreeChannels(usbh_hid_context_t *hd) {
	if (hd->ch_num_in >= 0) {
		/* Channel is halted after error in the interrupt routine.
		   USBHhaltChannel(hd->ch_num_in); */
		USBHfreeChannel(hd->ch_num_in);
		hd->ch_num_in = -1;
	}
}

/* The return value is non-zero, if the machine finished its work.
   The return value is zero, if the machine still works. */
static int HIDstateMachine(void *p) {
	usbh_hid_context_t *hd = p;

	usbh_transaction_result_t trr;
	int                       res;
	uint32_t                  len;

	switch (hd->state) {
		case HID_INIT:
			hd->ch_num_in = USBHallocChannel();
			if (hd->ch_num_in >= 0) {
				USBHopenChannel(hd->ch_num_in, hd->dev_addr, hd->ep_addr_in,
						hd->speed, INTERRUPT_TRANSFER, hd->length_in);
				hd->timer_in = 1;
				hd->errno = USBHLIB_SUCCESS;
				hd->pid_in = PID_DATA0;
				if (hd->length_out > 0 )
					hd->state = HID_REPORT_OUT;
				else 
					hd->state = HID_POLL_IN;
			}
			else {
				hd->errno = USBHLIB_ERROR_BUSY;
				hd->state = HID_EXIT;
			}
			break;
		case HID_POLL_IN:
			if (hd->timer_in == 0) {
				if (USBHstartTransaction(hd->ch_num_in, hd->pid_in,
							hd->buffer_in, hd->length_in) == 0) {
					if (hd->speed == HIGH_SPEED)
						hd->timer_in = 1 << (hd->interval_in - 1);
					else
						hd->timer_in = hd->interval_in;
					hd->errno = USBHLIB_IN_PROGRESS;
					hd->state = HID_POLL_IN_WAIT;
				}
				else {
					hd->errno = USBHLIB_ERROR_IO;
					hd->state = HID_EXIT;
				}
			}
			break;
		case HID_POLL_IN_WAIT:
			trr = USBHgetTransactionResult(hd->ch_num_in);
			if (trr == TR_DONE) {
				hd->pid_in = USBtoggleDataPid(hd->pid_in);
				len = USBHgetTransactionSize(hd->ch_num_in);
				hd->errno = USBHLIB_SUCCESS;
				hd->state = HID_POLL_IN;
				if (len >= hd->min_length_in) {
					hd->callback( hd, hd->user_data ,hd->buffer_in, len );
					if( hd->length_out > 0 ) {
						hd->state = HID_REPORT_OUT;
					}
				}
			}
			else if (trr == TR_NAK) {
				hd->state = HID_POLL_IN;
			}
			else if (trr == TR_STALL) {
				hd->state = HID_STALL_IN;
			}
			else if (trr == TR_ERROR) {
				hd->errno = USBHLIB_ERROR_IO;
				hd->state = HID_EXIT;
			}
			break;
		case HID_STALL_IN:
			res = usbh_clear_endpoint_halt(0, hd->ep_addr_in);
			hd->pid_in = PID_DATA0;
			hd->errno = res;
			if (res == USBHLIB_SUCCESS)
				hd->state = HID_POLL_IN;
			else if (res != USBHLIB_IN_PROGRESS)
				hd->state = HID_EXIT;
			break;
		case HID_REPORT_OUT:
			res = usbh_hid_set_report( 0, hd->iface, 0, hd->buffer_out, hd->length_out );
			hd->errno = res;
			if (res == USBHLIB_SUCCESS) {
				hd->state = HID_POLL_IN;
				hd->length_out = 0;
			}
			else if (res != USBHLIB_IN_PROGRESS)
				hd->state = HID_EXIT;
			break;
		default: /* HID_EXIT */
			break;
	}
	if (hd->state == HID_EXIT)
		HIDfreeChannels(hd);
	return hd->state == HID_EXIT;
}

static void HIDatSoF(void *p, uint16_t frnum) {
	(void)frnum;
	usbh_hid_context_t *hd = p;

	if (hd->timer_in > 0)
		--(hd->timer_in);
}

static void HIDatDisconnect(void *p) {
	usbh_hid_context_t *hd = p;

	HIDfreeChannels(hd);
	hd->errno = USBHLIB_ERROR_NO_DEVICE;
	hd->state = HID_EXIT;
	hd->callback( hd, hd->user_data ,NULL, 0 );
}


int usbh_hid_set_machine( usbh_hid_context_t *hid_ctx,
				  usb_speed_t speed, uint8_t dev_addr,
				  uint8_t ifc_no, uint16_t min_len,
                  usb_endpoint_descriptor_t const *ep_desc, unsigned ep_count, 
				  usbh_hid_report_callback_t callback, void *user_data )
{

	unsigned i;
	if( !hid_ctx ) {
		return USBHLIB_ERROR_NO_MEM;
	}
	if( !callback ) {
		return USBHLIB_ERROR_INVALID_PARAM;
	}
	hid_ctx->min_length_in = min_len;

	for (i = 0; i < ep_count; ++i) {
		if ((ep_desc[i].bEndpointAddress & ENDP_DIRECTION_MASK) == ENDP_IN &&
				ep_desc[i].bmAttributes == INTERRUPT_TRANSFER_BM) {
			break;
		}
	}

	if (i >= ep_count)
		return USBHLIB_ERROR_NOT_FOUND;

	/* Independent on speed, the size of in_buffer is 8 bytes. */
	if (ep_desc[i].wMaxPacketSize > MAX_FS_INTERRUPT_PACKET_SIZE ||
			ep_desc[i].wMaxPacketSize < hid_ctx->min_length_in)
		return USBHLIB_ERROR_INVALID_PARAM;

	if (ep_desc[i].bInterval == 0 ||
			(speed == HIGH_SPEED && ep_desc[i].bInterval > 16))
		return USBHLIB_ERROR_INVALID_PARAM;

	hid_ctx->speed = speed;
	hid_ctx->state = HID_INIT;
	hid_ctx->errno = USBHLIB_SUCCESS;
	hid_ctx->pid_in = PID_DATA0;
	hid_ctx->ch_num_in = -1;
	hid_ctx->timer_in = 1;
	hid_ctx->length_in = ep_desc[i].wMaxPacketSize;
	hid_ctx->dev_addr = dev_addr;
	hid_ctx->iface = ifc_no;
	hid_ctx->ep_addr_in = ep_desc[i].bEndpointAddress;
	hid_ctx->interval_in = ep_desc[i].bInterval;
	hid_ctx->length_out = 0;
	hid_ctx->callback = callback;
	hid_ctx->user_data = user_data;
	return usbh_set_class_machine(HIDstateMachine, HIDatSoF,
			HIDatDisconnect, hid_ctx );
}

int usbh_hid_is_device_ready(const usbh_hid_context_t* hid_ctx) {
	if( !hid_ctx ) {
		return 0;
	}
	return hid_ctx->state != HID_EXIT;
}


int usbh_hid_error(const usbh_hid_context_t* hid_ctx) {
	if( !hid_ctx ) {
		return USBHLIB_ERROR_INVALID_PARAM;
	}
	return hid_ctx->errno;
}

usbh_hid_context_t* usbh_hid_core_new_ctx(void) {
	void* mem =  malloc( sizeof( usbh_hid_context_t ) );
	memset( mem, 0, sizeof( usbh_hid_context_t ));
	return mem;
}

/*  Send report allowed only in interruppt contest  */
int usbh_hid_sent_report( usbh_hid_context_t* hid_ctx, 
		const uint8_t *rptout, uint16_t size )
{
	if( hid_ctx->length_out > 0 ) {
		return USBHLIB_ERROR_BUSY;
	}
	if( size > sizeof( hid_ctx->length_out ) ) {
		size = sizeof( hid_ctx->length_out );
	}
	memcpy( hid_ctx->buffer_out, rptout, size );
	hid_ctx->length_out = size;
	return USBHLIB_SUCCESS;
}
