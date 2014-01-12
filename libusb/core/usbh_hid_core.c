#include <string.h>
#include <usbh_core.h>
#include <usbh_error.h>
#include <usbh_hid_core.h>
#include <usbh_hid_req.h>
#include <usbh_io.h>
#include <usbh_std_req.h>

typedef enum {
  HID_EXIT = 0, /* HIDisDeviceReady must return 0, if HID machine is not configured. */
  HID_INIT,
  HID_POLL_IN,
  HID_POLL_IN_WAIT,
  HID_STALL_IN,
  HID_REPORT_OUT
} usbh_hid_state_t;

typedef struct {
  usb_speed_t      speed;
  usbh_hid_state_t state;
  int              errno;
  usb_pid_t        pid_in;
  int              ch_num_in;
  unsigned         timer_in;
  uint16_t         length_in;
  uint16_t         min_length_in;
  uint8_t          dev_addr;
  uint8_t          iface;
  uint8_t          protocol;
  uint8_t          ep_addr_in;
  uint8_t          interval_in;
  uint8_t          report_out;
  uint8_t          buffer_in[MAX_LS_INTERRUPT_PACKET_SIZE];
} usbh_hid_data_t;

/** Simplified mouse and kayboard application interface **/

int new_mouse_data;
unsigned mouse_buttons;
int mouse_x, mouse_y;

int new_keyboard_data;
unsigned keyboard_modifiers;
uint8_t keyboard_scan_code[KEYBOARD_MAX_PRESSED_KEYS];

/** Simplified implementation of mouse and kayboard events **/

static void MouseInit() {
  new_mouse_data = 1; /* Position (0, 0) should be displayed. */
  mouse_buttons = 0;
  mouse_x = mouse_y = 0;
}

static void MouseAction(hid_mouse_boot_report_t const *data) {
  mouse_buttons = data->buttons;
  mouse_x += data->x;
  mouse_y += data->y;
  new_mouse_data = 1;
}

static int num_lock, caps_lock;

static void KeyboardInit(void) {
  new_keyboard_data = 0;
  keyboard_modifiers = 0;
  num_lock = caps_lock = 0;
}

static uint8_t KeyboardAction(uint8_t const *pbuf, uint8_t report) {
  uint32_t i;
  int new_num_lock, new_caps_lock;

  for (i = 2; i < 2 + KEYBOARD_MAX_PRESSED_KEYS; ++i) {
    if (pbuf[i] == 1 || pbuf[i] == 2 || pbuf[i] == 3) {
      return report; /* error */
    }
  }
  keyboard_modifiers = pbuf[0];
  new_caps_lock = new_num_lock = 0;
  for (i = 0; i < KEYBOARD_MAX_PRESSED_KEYS; ++i) {
    keyboard_scan_code[i] = pbuf[i + 2];
    if (keyboard_scan_code[i] == NUM_LOCK_SCAN_CODE)
      new_num_lock = 1;
    if (keyboard_scan_code[i] == CAPS_LOCK_SCAN_CODE)
      new_caps_lock = 1;
  }
  new_keyboard_data = 1;

  if (num_lock == 0 && new_num_lock == 1)
    report ^= KEYBOARD_NUM_LOCK_LED;
  if (caps_lock == 0 && new_caps_lock == 1)
    report ^= KEYBOARD_CAPS_LOCK_LED;
  num_lock = new_num_lock;
  caps_lock = new_caps_lock;

  return report;
}

/** Full implementation of the HID core machine **/

static void HIDfreeChannels(usbh_hid_data_t *hd) {
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
  usbh_hid_data_t *hd = p;

  usbh_transaction_result_t trr;
  int                       res;
  uint32_t                  len;
  uint8_t                   report;

  switch (hd->state) {
    case HID_INIT:
      hd->ch_num_in = USBHallocChannel();
      if (hd->ch_num_in >= 0) {
        USBHopenChannel(hd->ch_num_in, hd->dev_addr, hd->ep_addr_in,
                        hd->speed, INTERRUPT_TRANSFER, hd->length_in);
        hd->timer_in = 1;
        hd->report_out = 0; /* all LEDs off */
        hd->errno = USBHLIB_SUCCESS;
        hd->pid_in = PID_DATA0;
        if (hd->protocol == KEYBOARD_PROTOCOL)
          hd->state = HID_REPORT_OUT;
        else /* MOUSE_PROTOCOL */
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
          if (hd->protocol == KEYBOARD_PROTOCOL) {
            report = KeyboardAction(hd->buffer_in, hd->report_out);
            if (hd->report_out != report) {
              hd->report_out = report;
              hd->state = HID_REPORT_OUT;
            }
          }
          else { /* MOUSE_PROTOCOL */
            MouseAction((hid_mouse_boot_report_t const *)hd->buffer_in);
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
      res = USBHclearEndpointHalt(0, hd->ep_addr_in);
      hd->pid_in = PID_DATA0;
      hd->errno = res;
      if (res == USBHLIB_SUCCESS)
        hd->state = HID_POLL_IN;
      else if (res != USBHLIB_IN_PROGRESS)
        hd->state = HID_EXIT;
      break;
    case HID_REPORT_OUT:
      res = HIDsetReport(0, hd->iface, 0, &hd->report_out, 1);
      hd->errno = res;
      if (res == USBHLIB_SUCCESS)
        hd->state = HID_POLL_IN;
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
  usbh_hid_data_t *hd = p;

  if (hd->timer_in > 0)
    --(hd->timer_in);
}

static void HIDatDisconnect(void *p) {
  usbh_hid_data_t *hd = p;

  HIDfreeChannels(hd);
  hd->errno = USBHLIB_ERROR_NO_DEVICE;
  hd->state = HID_EXIT;
}

static usbh_hid_data_t HIDdata;

int HIDsetMachine(usb_speed_t speed, uint8_t dev_addr,
                  usb_interface_descriptor_t const *if_desc,
                  usb_hid_main_descriptor_t const *hid_desc,
                  usb_endpoint_descriptor_t const *ep_desc,
                  unsigned ep_count) {
  unsigned i;

  if (if_desc->bInterfaceClass != HUMAN_INTERFACE_DEVICE_CLASS ||
      if_desc->bInterfaceSubClass != BOOT_INTERFACE_SUBCLASS) {
    return USBHLIB_ERROR_INVALID_PARAM;
  }

  if (if_desc->bInterfaceProtocol == MOUSE_PROTOCOL) {
    HIDdata.min_length_in = 3;
    MouseInit();
  }
  else if (if_desc->bInterfaceProtocol == KEYBOARD_PROTOCOL) {
    HIDdata.min_length_in = 8;
    KeyboardInit();
  }
  else {
    return USBHLIB_ERROR_INVALID_PARAM;
  }

  for (i = 0; i < ep_count; ++i) {
    if ((ep_desc[i].bEndpointAddress & ENDP_DIRECTION_MASK) == ENDP_IN &&
        ep_desc[i].bmAttributes == INTERRUPT_TRANSFER_BM) {
      break;
    }
  }

  if (i >= ep_count)
    return USBHLIB_ERROR_NOT_FOUND;

  /* Independent on speed, the size of in_buffer is 8 bytes. */
  if (ep_desc[i].wMaxPacketSize > MAX_LS_INTERRUPT_PACKET_SIZE ||
      ep_desc[i].wMaxPacketSize < HIDdata.min_length_in)
    return USBHLIB_ERROR_INVALID_PARAM;

  if (ep_desc[i].bInterval == 0 ||
      (speed == HIGH_SPEED && ep_desc[i].bInterval > 16))
    return USBHLIB_ERROR_INVALID_PARAM;

  HIDdata.speed = speed;
  HIDdata.state = HID_INIT;
  HIDdata.errno = USBHLIB_SUCCESS;
  HIDdata.pid_in = PID_DATA0;
  HIDdata.ch_num_in = -1;
  HIDdata.timer_in = 1;
  HIDdata.length_in = ep_desc[i].wMaxPacketSize;
  HIDdata.dev_addr = dev_addr;
  HIDdata.iface = if_desc->bInterfaceNumber;
  HIDdata.protocol = if_desc->bInterfaceProtocol;
  HIDdata.ep_addr_in = ep_desc[i].bEndpointAddress;
  HIDdata.interval_in = ep_desc[i].bInterval;
  HIDdata.report_out = 0;

  return USBHsetClassMachine(HIDstateMachine, HIDatSoF,
                             HIDatDisconnect, &HIDdata);
}

int HIDisDeviceReady() {
  return HIDdata.state != HID_EXIT;
}
