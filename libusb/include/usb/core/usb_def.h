#ifndef _USB_DEF_H
#define _USB_DEF_H 1

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __packed
#define __packed  __attribute__ ((__packed__))
#endif

/** Definitions from the USB standards **/

/* Visible device states */

typedef uint32_t usb_visible_state_t;
enum usbh_state {
	DISCONNECTED  = 0x00,
	ATTACHED      = 0x01,
	POWERED       = 0x03,
	DEFAULT       = 0x07,
	ADDRESS       = 0x0f,
	CONFIGURED    = 0x1f,
	SUSPENDED     = 0x80
};

/* PID tokens */

typedef enum {
  PID_OUT      = 0x1,
  PID_IN       = 0x9,
  PID_SOF      = 0x5,
  PID_SETUP    = 0xD,
  PID_DATA0    = 0x3,
  PID_DATA1    = 0xB,
  PID_DATA2    = 0x7,
  PID_MDATA    = 0xF,
  PID_ACK      = 0x2,
  PID_NAK      = 0xA,
  PID_STALL    = 0xE,
  PID_NYET     = 0x6,
  PID_PRE      = 0xC,
  PID_ERR      = 0xC,
  PID_SPLIT    = 0x8,
  PID_PING     = 0x4,
  PID_RESERVED = 0x0
} usb_pid_t;

static inline usb_pid_t USBtoggleDataPid(usb_pid_t pid) {
  if (pid == PID_DATA0)
    return PID_DATA1;
  else
    return PID_DATA0;
}

/* The setup packet format */

typedef struct {
  uint8_t  bmRequestType;
  uint8_t  bRequest;
  uint16_t wValue;
  uint16_t wIndex;
  uint16_t wLength;
} __packed usb_setup_packet_t;

/* The notification packet format */

typedef struct {
  uint8_t  bmRequestType;
  uint8_t  bNotification;
  uint16_t wValue;
  uint16_t wIndex;
  uint16_t wLength;
} __packed usb_notification_packet_t;

/* The bmRequestType field in the setup packet */
enum usb_request_type {
	REQUEST_DIRECTION    =0x80, /* mask to get transfer direction */
	DEVICE_TO_HOST       =0x80,
	HOST_TO_DEVICE       =0x00,
	REQUEST_TYPE         =0x60, /* mask to get request type */
	STANDARD_REQUEST     =0x00,
	CLASS_REQUEST        =0x20,
	VENDOR_REQUEST       =0x40,
	REQUEST_RECIPIENT    =0x1f, /* mask to get recipient */
	DEVICE_RECIPIENT     =0x00,
	INTERFACE_RECIPIENT  =0x01,
	ENDPOINT_RECIPIENT   =0x02
};
/* The bRequest field in the standard request setup packet */

typedef enum {
  GET_STATUS        = 0,
  CLEAR_FEATURE     = 1,
  /* Reserved for future use = 2, */
  SET_FEATURE       = 3,
  /* Reserved for future use = 4, */
  SET_ADDRESS       = 5,
  GET_DESCRIPTOR    = 6,
  SET_DESCRIPTOR    = 7,
  GET_CONFIGURATION = 8,
  SET_CONFIGURATION = 9,
  GET_INTERFACE     = 10,
  SET_INTERFACE     = 11,
  SYNCH_FRAME       = 12
} usb_standard_request_t;

/* The bRequest field in the audio class specific request setup */

typedef enum {
  SET_CUR  = 0x01,
  GET_CUR  = 0x81,
  SET_MIN  = 0x02,
  GET_MIN  = 0x82,
  SET_MAX  = 0x03,
  GET_MAX  = 0x83,
  SET_RES  = 0x04,
  GET_RES  = 0x84,
  SET_MEM  = 0x05,
  GET_MEM  = 0x85,
  GET_STAT = 0xff
} usb_ac_request_t;

/* CS value in the audio class specific request setup */

typedef enum {
  MUTE_CONTROL              = 0x01,
  VOLUME_CONTROL            = 0x02,
  BASS_CONTROL              = 0x03,
  MID_CONTROL               = 0x04,
  TREBLE_CONTROL            = 0x05,
  GRAPHIC_EQUALIZER_CONTROL = 0x06,
  AUTOMATIC_GAIN_CONTROL    = 0x07,
  DELAY_CONTROL             = 0x08,
  BASS_BOOST_CONTROL        = 0x09,
  LOUDNESS_CONTROL          = 0x0a
} usb_ac_feature_unit_control_selector_t;

/* The bRequest field in the CDC request setup packet */

typedef enum {
  SEND_ENCAPSULATED_COMMAND = 0x00,
  GET_ENCAPSULATED_COMMAND  = 0x01,
  SET_COMM_FEATURE          = 0x02,
  GET_COMM_FEATURE          = 0x03,
  CLEAR_COMM_FEATURE        = 0x04,
  SET_AUX_LINE_STATE        = 0x10,
  SET_HOOK_STATE            = 0x11,
  PULSE_SETUP               = 0x12,
  SEND_PULSE                = 0x13,
  SET_PULSE_TIME            = 0x14,
  RING_AUX_JACK             = 0x15,
  SET_LINE_CODING           = 0x20,
  GET_LINE_CODING           = 0x21,
  SET_CONTROL_LINE_STATE    = 0x22,
  SEND_BREAK                = 0x23,
  SET_RINGER_PARAMS         = 0x30,
  GET_RINGER_PARAMS         = 0x31,
  SET_OPERATION_PARAMS      = 0x32,
  GET_OPERATION_PARAMS      = 0x33,
  SET_LINE_PARAMS           = 0x34,
  GET_LINE_PARAMS           = 0x35,
  DIAL_DIGITS               = 0x36
  /* ... and many more ... */
} usb_cdc_request_t;

/* The bNotification field in the CDC notification packet */

typedef enum {
  NETWORK_CONNECTION  = 0x00,
  RESPONSE_AVAILABLE  = 0x01,
  AUX_JACK_HOOK_STATE = 0x08,
  RING_DETECT         = 0x09,
  SERIAL_STATE        = 0x20,
  CALL_STATE_CHANGE   = 0x28,
  LINE_STATE_CHANGE   = 0x29
} usb_cdc_pstn_subclass_notification_t;

/* The bRequest field in the HID class request setup packet */

typedef enum {
  GET_REPORT   = 1,
  GET_IDLE     = 2,
  GET_PROTOCOL = 3,
  SET_REPORT   = 9,
  SET_IDLE     = 10,
  SET_PROTOCOL = 11
} usb_hid_request_t;

typedef enum {
  HID_BOOT_PROTOCOL   = 0,
  HID_REPORT_PROTOCOL = 1
} usb_hid_protocol_t;

/* When bRequest is CLEAR_FEATURE or SET_FEATURE, the wValue field
   contains the feature selector. */

typedef enum {
  ENDPOINT_HALT        = 0,
  DEVICE_REMOTE_WAKEUP = 1,
  TEST_MODE            = 2
} usb_feature_selector_t;

/* Device, interface class, subclass, etc. */

#define AUDIO_CLASS              0x01
#define AUDIOCONTROL_SUBCLASS    0x01
#define AUDIOSTREAMING_SUBCLASS  0x02
#define MIDISTREAMING_SUBCLASS   0x03

#define COMMUNICATION_DEVICE_CLASS       0x02
#define COMMUNICATION_INTERFACE_CLASS    0x02
#define DATA_INTERFACE_CLASS             0x0A
#define ABSTRACT_CONTROL_MODEL_SUBCLASS  0x02

#define HUMAN_INTERFACE_DEVICE_CLASS  0x03
#define BOOT_INTERFACE_SUBCLASS       0x01
#define KEYBOARD_PROTOCOL             0x01
#define MOUSE_PROTOCOL                0x02

#define MASS_STORAGE_CLASS            0x08
#define SCSI_TRANSPARENT_SUBCLASS     0x06
#define BULK_ONLY_TRANSPORT_PROTOCOL  0x50

#define VENDOR_SPECIFIC  0xFF

/* When bRequest is GET_DESCRIPTOR or SET_DESCRIPTOR, the wValue field
   contains the descriptor type. The same values are used in the
   bDescriptorType field in all descriptors. */

typedef enum {
  DEVICE_DESCRIPTOR                    = 1,
  CONFIGURATION_DESCRIPTOR             = 2,
  STRING_DESCRIPTOR                    = 3,
  INTERFACE_DESCRIPTOR                 = 4,
  ENDPOINT_DESCRIPTOR                  = 5,
  DEVICE_QUALIFIER_DESCRIPTOR          = 6,
  OTHER_SPEED_CONFIGURATION_DESCRIPTOR = 7,
  INTERFACE_POWER_DESCRIPTOR           = 8,
  OTG_DESCRIPTOR                       = 9,
  DEBUG_DESCRIPTOR                     = 10,
  INTERFACE_ASSOCIATION_DESCRIPTOR     = 11,
  CS_UNDEFINED_DESCRIPTOR              = 0x20,
  CS_DEVICE_DESCRIPTOR                 = 0x21,
  CS_CONFIGURATION_DESCRIPTOR          = 0x22,
  CS_STRING_DESCRIPTOR                 = 0x23,
  CS_INTERFACE_DESCRIPTOR              = 0x24,
  CS_ENDPOINT_DESCRIPTOR               = 0x25
} usb_descriptor_type_t;

typedef enum {
  AC_HEADER_DESCRIPTOR          = 0x01,
  AC_INPUT_TERMINAL_DESCRIPTOR  = 0x02,
  AC_OUTPUT_TERMINAL_DESCRIPTOR = 0x03,
  AC_MIXER_UNIT_DESCRIPTOR      = 0x04,
  AC_SELECTOR_UNIT_DESCRIPTOR   = 0x05,
  AC_FEATURE_UNIT_DESCRIPTOR    = 0x06,
  AC_PROCESSING_UNIT_DESCRIPTOR = 0x07,
  AC_EXTENSION_UNIT_DESCRIPTOR  = 0x08
} usb_audio_control_interface_descriptor_subtype_t;

typedef enum {
  AS_GENERAL_DESCRIPTOR         = 0x01,
  AS_FORMAT_TYPE_DESCRIPTOR     = 0x02,
  AS_FORMAT_SPECIFIC_DESCRIPTOR = 0x03
} usb_audio_streaming_interface_descriptor_subtype_t;


typedef enum {
  EP_GENERAL_DESCRIPTOR = 0x01
} usb_audio_data_endpoint_descriptor_subtype_t;

typedef enum {
  CDC_HEADER_DESCRIPTOR          = 0x00,
  CDC_CALL_MANAGEMENT_DESCRIPTOR = 0x01,
  CDC_ACM_DESCRIPTOR             = 0x02,
  CDC_UNION_DESCRIPTOR           = 0x06
} usb_communication_data_class_functional_descriptor_subtype_t;

typedef enum {
  HID_MAIN_DESCRIPTOR     = 0x21,
  HID_REPORT_DESCRIPTOR   = 0x22,
  HID_PHYSICAL_DESCRIPTOR = 0x23,
} usb_hid_descriptor_type_t;

/* Descriptor formats */

typedef struct usb_device_descriptor {
  uint8_t  bLength;
  uint8_t  bDescriptorType;
  uint16_t bcdUSB;
  uint8_t  bDeviceClass;
  uint8_t  bDeviceSubClass;
  uint8_t  bDeviceProtocol;
  uint8_t  bMaxPacketSize0;
  uint16_t idVendor;
  uint16_t idProduct;
  uint16_t bcdDevice;
  uint8_t  iManufacturer;
  uint8_t  iProduct;
  uint8_t  iSerialNumber;
  uint8_t  bNumConfigurations;
} __packed usb_device_descriptor_t;

typedef struct {
  uint8_t  bLength;
  uint8_t  bDescriptorType;
  uint16_t bcdUSB;
  uint8_t  bDeviceClass;
  uint8_t  bDeviceSubClass;
  uint8_t  bDeviceProtocol;
  uint8_t  bMaxPacketSize0;
  uint8_t  bNumConfigurations;
  uint8_t  bReserved;
} __packed usb_device_qualifier_descriptor_t;

typedef struct {
  uint8_t  bLength;
  uint8_t  bDescriptorType;
  uint16_t wTotalLength;
  uint8_t  bNumInterfaces;
  uint8_t  bConfigurationValue;
  uint8_t  iConfiguration;
  uint8_t  bmAttributes;
  uint8_t  bMaxPower;
} __packed usb_configuration_descriptor_t;

typedef struct {
  uint8_t  bLength;
  uint8_t  bDescriptorType;
  uint8_t  bInterfaceNumber;
  uint8_t  bAlternateSetting;
  uint8_t  bNumEndpoints;
  uint8_t  bInterfaceClass;
  uint8_t  bInterfaceSubClass;
  uint8_t  bInterfaceProtocol;
  uint8_t  iInterface;
} __packed usb_interface_descriptor_t;

typedef struct {
  uint8_t  bLength;
  uint8_t  bDescriptorType;
  uint8_t  bEndpointAddress;
  uint8_t  bmAttributes;
  uint16_t wMaxPacketSize;
  uint8_t  bInterval;
} __packed usb_endpoint_descriptor_t;

typedef struct {
  uint8_t  bLength;
  uint8_t  bDescriptorType;
  uint16_t bcdHID;
  uint8_t  bCountryCode;
  uint8_t  bNumDescriptors;
  uint8_t  bDescriptorType1;
  uint16_t wDescriptorLength1;
} __packed usb_hid_main_descriptor_t;

typedef struct {
  uint8_t  bFunctionLength;
  uint8_t  bDescriptorType;
  uint8_t  bDescriptorSubtype;
  uint16_t bcdCDC;
} __packed usb_cdc_header_descriptor_t;

typedef struct {
  uint8_t bFunctionLength;
  uint8_t bDescriptorType;
  uint8_t bDescriptorSubtype;
  uint8_t bmCapabilities;
  uint8_t bDataInterface;
} __packed usb_cdc_call_management_descriptor_t;

typedef struct {
  uint8_t bFunctionLength;
  uint8_t bDescriptorType;
  uint8_t bDescriptorSubtype;
  uint8_t bmCapabilities;
} __packed usb_cdc_acm_descriptor_t;

typedef struct {
  uint8_t bFunctionLength;
  uint8_t bDescriptorType;
  uint8_t bDescriptorSubtype;
  uint8_t bControlInterface;
  uint8_t bSubordinateInterface0;
} __packed usb_cdc_union_descriptor_t;

typedef struct {
  uint8_t  bLength;
  uint8_t  bDescriptorType;
  uint8_t  bDescriptorSubtype;
  uint16_t bcdADC;
  uint16_t wTotalLength;
  uint8_t  bInCollection;
  uint8_t  baInterfaceNr;
} __packed usb_ac_header_descriptor_t;

typedef struct {
  uint8_t  bLength;
  uint8_t  bDescriptorType;
  uint8_t  bDescriptorSubtype;
  uint8_t  bTerminalID;
  uint16_t wTerminalType;
  uint8_t  bAssocTerminal;
  uint8_t  bNrChannels;
  uint16_t wChannelConfig;
  uint8_t  iChannelNames;
  uint8_t  iTerminal;
} __packed usb_ac_input_terminal_descriptor_t;

typedef struct {
  uint8_t  bLength;
  uint8_t  bDescriptorType;
  uint8_t  bDescriptorSubtype;
  uint8_t  bUnitID;
  uint8_t  bSourceID;
  uint8_t  bControlSize;
  uint16_t bmaControls0;
  uint8_t  iFeature;
} __packed usb_ac_feature_unit_descriptor_t;

typedef struct {
  uint8_t  bLength;
  uint8_t  bDescriptorType;
  uint8_t  bDescriptorSubtype;
  uint8_t  bTerminalID;
  uint16_t wTerminalType;
  uint8_t  bAssocTerminal;
  uint8_t  bSourceID;
  uint8_t  iTerminal;
} __packed usb_ac_output_terminal_descriptor_t;

typedef struct {
  uint8_t  bLength;
  uint8_t  bDescriptorType;
  uint8_t  bDescriptorSubtype;
  uint8_t  bTerminalLink;
  uint8_t  bDelay;
  uint16_t wFormatTag;
} __packed usb_as_general_descriptor_t;

typedef struct {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bDescriptorSubtype;
  uint8_t bFormatType;
  uint8_t bNrChannels;
  uint8_t bSubFrameSize;
  uint8_t bBitResolution;
  uint8_t bSamFreqType;
  uint8_t tSamFreq[3];
} __packed usb_as_format_type_descriptor_t;

typedef struct {
  uint8_t  bLength;
  uint8_t  bDescriptorType;
  uint8_t  bEndpointAddress;
  uint8_t  bmAttributes;
  uint16_t wMaxPacketSize;
  uint8_t  bInterval;
  uint8_t  bRefresh;
  uint8_t  bSynchAddress;
} __packed usb_std_audio_data_endpoint_descriptor_t;

typedef struct {
  uint8_t  bLength;
  uint8_t  bDescriptorType;
  uint8_t  bDescriptorSubtype;
  uint8_t  bmAttributes;
  uint8_t  bLockDelayUnits;
  uint16_t wLockDelay;
} __packed usb_cs_audio_data_endpoint_descriptor_t;

#define usb_string_descriptor_t(n) \
  struct {                         \
    uint8_t  bLength;              \
    uint8_t  bDescriptorType;      \
    uint16_t bString[(n)];         \
  } __packed

/* Maximal length USB string descriptor */

typedef usb_string_descriptor_t(126) usb_max_string_descriptor_t;

/* The bmAttributes field in the configuration descriptor */

#define D7_RESERVED    0x80
#define SELF_POWERED   0x40
#define REMOTE_WAKEUP  0x20

/* Device status returned in GET_STATUS request */

#define STATUS_SELF_POWERED   0x0001
#define STATUS_REMOTE_WAKEUP  0x0002

/* The endpoint address (number and direction) */

#define ENDP0   0
#define ENDP1   1
#define ENDP2   2
#define ENDP3   3
#define ENDP4   4
#define ENDP5   5
#define ENDP6   6
#define ENDP7   7
#define ENDP8   8
#define ENDP9   9
#define ENDP10  10
#define ENDP11  11
#define ENDP12  12
#define ENDP13  13
#define ENDP14  14
#define ENDP15  15
#define ENDP_DIRECTION_MASK  0x80
#define ENDP_NUMBER_MASK     0x7f
#define ENDP_IN              0x80
#define ENDP_OUT             0x00

#define USB_EP_MAX_COUNT  16

/* The bmAttributes field in the endpoint descriptor */

typedef enum {
  CONTROL_TRANSFER     = 0,
  ISOCHRONOUS_TRANSFER = 1,
  BULK_TRANSFER        = 2,
  INTERRUPT_TRANSFER   = 3
} usb_transfer_t;

#define CONTROL_TRANSFER_BM              0x00
#define ISOCHRONOUS_TRANSFER_BM          0x01
#define BULK_TRANSFER_BM                 0x02
#define INTERRUPT_TRANSFER_BM            0x03
#define ASYNCHRONOUS_ENDPOINT            0x04
#define ADAPTIVE_ENDPOINT                0x08
#define SYNCHRONOUS_ENDPOINT             0x0c
#define FEEDBACK_ENDPOINT                0x10
#define IMPLICIT_FEEDBACK_DATA_ENDPOINT  0x20

/* Endpoint wMaxPacketSize */

#define MAX_LS_CONTROL_PACKET_SIZE         8
#define MAX_FS_CONTROL_PACKET_SIZE        64
#define MAX_HS_CONTROL_PACKET_SIZE        64
#define MAX_FS_ISOCHRONOUS_PACKET_SIZE  1023
#define MAX_HS_ISOCHRONOUS_PACKET_SIZE  1024
#define MAX_FS_BULK_PACKET_SIZE           64
#define MAX_HS_BULK_PACKET_SIZE          512
#define MAX_LS_INTERRUPT_PACKET_SIZE      8
#define MAX_FS_INTERRUPT_PACKET_SIZE      64
#define MAX_HS_INTERRUPT_PACKET_SIZE    1024

/* The language selector in the string descriptor */

#define LANG_US_ENGLISH  0x0409
#define LANG_POLISH      0x0415

/* Mouse boot protocol report */

typedef struct {
  uint8_t buttons;
  int8_t  x;
  int8_t  y;
} __attribute__ ((__packed__)) hid_mouse_boot_report_t;

#define MOUSE_LEFT_BUTTON     0x01
#define MOUSE_RIGHT_BUTTON    0x02
#define MOUSE_MIDDLE_BUTTON   0x04

/* Keyboard boot protocol report */

#define KEYBOARD_LEFT_CTRL    0x01
#define KEYBOARD_LEFT_SHIFT   0x02
#define KEYBOARD_LEFT_ALT     0x04
#define KEYBOARD_LEFT_GUI     0x08
#define KEYBOARD_RIGHT_CTRL   0x10
#define KEYBOARD_RIGHT_SHIFT  0x20
#define KEYBOARD_RIGHT_ALT    0x40
#define KEYBOARD_RIGHT_GUI    0x80

#define KEYBOARD_NUM_LOCK_LED     0x01
#define KEYBOARD_CAPS_LOCK_LED    0x02
#define KEYBOARD_SCROLL_LOCK_LED  0x04
#define KEYBOARD_COMPOSE_LED      0x08
#define KEYBOARD_KANA_LED         0x10

#define KEYBOARD_MAX_PRESSED_KEYS  6
#define CAPS_LOCK_SCAN_CODE        57
#define NUM_LOCK_SCAN_CODE         83

/* RS-232 emulation data structures */

typedef struct {
  uint32_t dwDTERate;   /* bits per second */
  uint8_t  bCharFormat; /* stop bit(s), see below */
  uint8_t  bParityType; /* parity, see below */
  uint8_t  bDataBits;
} __packed usb_cdc_line_coding_t;

#define ONE_STOP_BIT            0
#define ONE_AND_HALF_STOP_BITS  1
#define TWO_STOP_BITS           2

#define NO_PARITY     0
#define ODD_PARITY    1
#define EVEN_PARITY   2
#define MARK_PARITY   3
#define SPACE_PARITY  4

typedef struct {
  usb_notification_packet_t notification;
  uint16_t                  wData;
} __packed usb_cdc_serial_state_t;

/* Mass Storage Class, Bulk-Only Transport structures and constants */

/* Mass Storage Class requests */
#define MSC_GET_MAX_LUN      0xfe
#define MSC_BULK_ONLY_RESET  0xff

#define MSC_CBWCB_LENGTH  16

/* Mass Storage Class, Bulk-Only Transport, Command Block Wrapper */
typedef struct {
  uint32_t dCBWSignature;
  uint32_t dCBWTag;
  uint32_t dCBWDataTransferLength;
  uint8_t  bmCBWFlags;   /* Only 7-th bit, bits 0-6 are reserved. */
  uint8_t  bCBWLUN;      /* Only 4 LS bits, 4 MS bits are reserved. */
  uint8_t  bCBWCBLength; /* Only 5 LS bits, 3 MS bits are reserved. */
  uint8_t  CBWCB[MSC_CBWCB_LENGTH];
} __packed msc_bot_cbw_t;

#define MSC_BOT_CBW_SIGNATURE                0x43425355
#define MSC_BOT_CBW_FLAGS_RESERVED_BITS      0x7f
#define MSC_BOT_CBW_LUN_RESERVED_BITS        0xf0
#define MSC_BOT_CBW_CB_LENGTH_RESERVED_BITS  0xe0
#define MSC_BOT_CBW_DATA_IN                  0x80

/* Mass Storage Class, Bulk-Only Transport, Command Status Wrapper */
typedef struct {
  uint32_t dCSWSignature;
  uint32_t dCSWTag;
  uint32_t dCSWDataResidue;
  uint8_t  bCSWStatus;
} __packed msc_bot_csw_t;

#define MSC_BOT_CSW_SIGNATURE       0x53425355
#define MSC_BOT_CSW_COMMAND_PASSED  0x00
#define MSC_BOT_CSW_COMMAND_FAILED  0x01
#define MSC_BOT_CSW_PHASE_ERROR     0x02
#define MSC_BOT_UNDEF_ERROR         0xFF

/** Host limits **/

/* Standard timers (milliseconds) */

#define STARTUP_TIME_MS          100
#define RESET_TIME_MS            15
#define RECOVERY_TIME_MS         10
#define DATA_STAGE_TIMEOUT_MS    5000
#define NODATA_STAGE_TIMEOUT_MS  50

/* Application specific timers (microseconds) */

#define HOST_LS_SCHEDULE_US  275
#define HOST_FS_SCHEDULE_US  30
#define HOST_HS_SCHEDULE_US  15

/* Application specific timers (milliseconds) */

#define DEVICE_RESET_TIME_MS  300

/* Others */

#define TRANS_MAX_REP_COUNT  3

/** USB library common definitions **/

/* Device speed or maximal host speed */

typedef enum {
  HIGH_SPEED     = 0,
  FULL_SPEED     = 1,
  LOW_SPEED      = 2,
  FULL_LOW_SPEED = 3,
  NO_SPEED       = 15 /* indicates an error */
} usb_speed_t;

/* Used transceiver */

typedef enum {
  USB_PHY_A,
  USB_PHY_B,
  USB_PHY_ULPI,
  USB_PHY_I2C
} usb_phy_t;

/* Return value of device request routines */

typedef enum {
  REQUEST_SUCCESS,
  REQUEST_ERROR
} usb_result_t;

/* Values reported after transaction completion */

typedef enum {
  TR_UNDEF = 0,
  TR_DONE,
  TR_NAK,
  TR_STALL,
  TR_ERROR
} usbh_transaction_result_t;

#ifdef __cplusplus
}
#endif

#endif
