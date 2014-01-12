#include <auxiliary.h>
#include <delay.h>
#include <string.h>
#include <usb_endianness.h>
#include <usbh_api.h>
#include <usbh_core.h>
#include <usbh_error.h>
#include <usbh_interrupt.h>
#include <usbh_io.h>
#include <usbh_lib.h>
#include <usbh_std_req.h>

/* Hub is not supported - only one device can be attached at once. */
#define DEVICE_ADDRESS  1

/** USB host core structures **/

typedef enum {
  HOST_IDLE = 0,
  HOST_CONTROL_TRANSFER,
  HOST_ENUMERATION,
  HOST_CLASS
} usbh_machine_state_t;

typedef enum {
  ENUM_IDLE = 0,
  ENUM_GET_DEV_DESC,
  ENUM_GET_FULL_DEV_DESC,
  ENUM_SET_DEV_ADDR,
} usbh_enumeration_state_t;

typedef enum {
  CTRL_IDLE = 0,
  CTRL_SETUP,
  CTRL_SETUP_WAIT,
  CTRL_DATA_IN,
  CTRL_DATA_IN_WAIT,
  CTRL_DATA_OUT,
  CTRL_DATA_OUT_WAIT,
  CTRL_STATUS_IN,
  CTRL_STATUS_IN_WAIT,
  CTRL_STATUS_OUT,
  CTRL_STATUS_OUT_WAIT,
  CTRL_DONE
} usbh_control_state_t;

typedef struct {
  usb_speed_t             speed;
  usb_visible_state_t     visible_state;
  uint8_t                 address;
  usb_device_descriptor_t dev_desc;
} usbh_device_t;

typedef struct {
  usbh_control_state_t state;
  usb_setup_packet_t   setup;
  usb_pid_t            pid;
  int                  hc_num_in;
  int                  hc_num_out;
  int                  timeout;
  int                  error_count;
  int                  errno;
  uint8_t              *buffer;
  uint32_t             length;
  uint32_t             transfered;
  uint16_t             max_packet;
} usbh_control_block_t;

typedef struct {
  usbh_enumeration_state_t state;
  int                      errno;
} usbh_enumeration_block_t;

typedef struct {
  int  (*machine)(void *);
  void (*at_sof)(void *, uint16_t);
  void (*at_disconnect)(void *);
  void *parameter;
} usbh_class_block_t;

typedef struct {
  usbh_machine_state_t     g_state;
  usbh_machine_state_t     g_prev_state;
  usbh_control_block_t     control;
  usbh_enumeration_block_t enumeration;
  usbh_class_block_t       class;
} usbh_machine_t;

static usbh_machine_t Machine;
static usbh_device_t  Device;

/** USB host core initialization **/

/* Reinitialize host for new enumeration. */
static void USBHcoreDeInit(void) {
  USBHstopAllChannels();

  Machine.g_state = HOST_IDLE;
  Machine.g_prev_state = HOST_IDLE;

  Machine.control.state = CTRL_IDLE;
  Machine.control.setup.bmRequestType = 0;
  Machine.control.setup.bRequest = 0;
  Machine.control.setup.wValue = 0;
  Machine.control.setup.wIndex = 0;
  Machine.control.setup.wLength = 0;
  Machine.control.timeout = 0;
  Machine.control.error_count = 0;
  Machine.control.errno = USBHLIB_SUCCESS;
  Machine.control.buffer = 0;
  Machine.control.length = 0;
  Machine.control.transfered = 0;
  Machine.control.max_packet = MAX_LS_CONTROL_PACKET_SIZE;

  Machine.enumeration.state = ENUM_IDLE;

  Machine.class.machine = 0;
  Machine.class.at_disconnect = 0;
  Machine.class.at_sof = 0;
  Machine.class.parameter = 0;

  Device.speed = NO_SPEED;
  Device.visible_state = DISCONNECTED;
  Device.address = 0;
  memset(&Device.dev_desc, 0, sizeof(usb_device_descriptor_t));
}

/* This function is called only once at configuration phase.
   These initializations are never reversed. */
int USBHcoreConfigure() {
  int res;

  USBHcoreDeInit();

  res = USBHchannelsConfigure();
  if (res < 0)
    return res;

  Machine.control.hc_num_out = USBHallocChannel();
  Machine.control.hc_num_in  = USBHallocChannel();
  if (Machine.control.hc_num_out < 0 || Machine.control.hc_num_in < 0)
    return USBHLIB_ERROR_IO;

  return USBHLIB_SUCCESS;
}

/** USB host event handlers **/

void USBHdeviceDisconnected() {
  /* Class deinitialization must be called first. */
  if (Machine.class.at_disconnect)
    Machine.class.at_disconnect(Machine.class.parameter);
  USBHcoreDeInit();
}

/* This implementation does not distinguish between ATTACHED and
   POWERED device states. Pure ATTACHED state cannot be detected. */
void USBHdeviceAttached() {
  Device.visible_state = POWERED;
}

void USBHdeviceSpeed(usb_speed_t speed) {
  Device.speed = speed;
}

void USBHdeviceResetDone() {
  Device.visible_state = DEFAULT;
  USBHopenChannel(Machine.control.hc_num_out, Device.address,
                  ENDP_OUT | ENDP0, Device.speed,
                  CONTROL_TRANSFER, Machine.control.max_packet);
  USBHopenChannel(Machine.control.hc_num_in, Device.address,
                  ENDP_IN | ENDP0, Device.speed,
                  CONTROL_TRANSFER, Machine.control.max_packet);
  Machine.g_state = HOST_ENUMERATION;
  Machine.enumeration.state = ENUM_GET_DEV_DESC;
}

/** USB host core protocol state machines **/

/* Simplified timeout implementation. In some circumstances timeout
   could be longer, but it is never shorter. */
void USBHsof(uint16_t frnum) {
  if (Machine.control.timeout >= 0)
    --Machine.control.timeout;
  if (Machine.class.at_sof)
    Machine.class.at_sof(Machine.class.parameter, frnum);
}

/* Control transfer state machine.
   The return value is non-zero, if the machine finished its work.
   The return value is zero, if the machine still works. */
static int USBHhandleControlTransfer(void) {
  usbh_transaction_result_t res;
  uint32_t                  len;

  switch (Machine.control.state) {
    case CTRL_SETUP:
      if (USBHstartTransaction(Machine.control.hc_num_out, PID_SETUP,
                               (uint8_t *)&Machine.control.setup,
                               sizeof(usb_setup_packet_t)) == 0) {
        Machine.control.state = CTRL_SETUP_WAIT;
      }
      else {
        Machine.control.errno = USBHLIB_ERROR_IO;
        Machine.control.state = CTRL_DONE;
      }
      break;
    case CTRL_SETUP_WAIT:
      res = USBHgetTransactionResult(Machine.control.hc_num_out);
      if (res == TR_DONE) {
        if (Machine.control.length != 0) {
          Machine.control.pid = PID_DATA1;
          Machine.control.timeout = DATA_STAGE_TIMEOUT_MS;
          if ((Machine.control.setup.bmRequestType &
                                 REQUEST_DIRECTION) == DEVICE_TO_HOST)
            Machine.control.state = CTRL_DATA_IN;
          else
            Machine.control.state = CTRL_DATA_OUT;
        }
        else { /* No DATA stage */
          Machine.control.timeout = NODATA_STAGE_TIMEOUT_MS;
          Machine.control.state = CTRL_STATUS_IN;
        }
        if (Device.speed == HIGH_SPEED) {
          Machine.control.timeout <<= 3; /* high speed microframes */
        }
        Machine.control.error_count = 0;
      }
      else if (res == TR_ERROR) {
        if (++Machine.control.error_count < TRANS_MAX_REP_COUNT) {
          Machine.control.state = CTRL_SETUP;
        }
        else {
          Machine.control.errno = USBHLIB_ERROR_IO;
          Machine.control.state = CTRL_DONE;
        }
      }
      break;
    case CTRL_DATA_IN:
      len = min(Machine.control.length, Machine.control.max_packet);
      if (USBHstartTransaction(Machine.control.hc_num_in,
                               Machine.control.pid,
                               Machine.control.buffer, len) == 0) {
        Machine.control.state = CTRL_DATA_IN_WAIT;
      }
      else {
        Machine.control.errno = USBHLIB_ERROR_IO;
        Machine.control.state = CTRL_DONE;
      }
      break;
    case CTRL_DATA_IN_WAIT:
      res = USBHgetTransactionResult(Machine.control.hc_num_in);
      if (res == TR_DONE) {
        len = USBHgetTransactionSize(Machine.control.hc_num_in);
        Machine.control.buffer     += len;
        Machine.control.transfered += len;
        Machine.control.length     -= len;
        if (Machine.control.length == 0 ||
            len < Machine.control.max_packet) {
          Machine.control.state = CTRL_STATUS_OUT;
        }
        else {
          Machine.control.pid = USBtoggleDataPid(Machine.control.pid);
          Machine.control.state = CTRL_DATA_IN;
        }
        Machine.control.error_count = 0;
      }
      else if (Machine.control.timeout < 0) {
        USBHhaltChannel(Machine.control.hc_num_in);
        Machine.control.errno = USBHLIB_ERROR_TIMEOUT;
        Machine.control.state = CTRL_DONE;
      }
      else if (res == TR_NAK) {
        Machine.control.error_count = 0;
        Machine.control.state = CTRL_DATA_IN;
      }
      else if (res == TR_STALL) {
        Machine.control.errno = USBHLIB_ERROR_STALL;
        Machine.control.state = CTRL_DONE;
      }
      else if (res == TR_ERROR) {
        if (++Machine.control.error_count < TRANS_MAX_REP_COUNT) {
          Machine.control.state = CTRL_DATA_IN;
        }
        else {
          Machine.control.errno = USBHLIB_ERROR_IO;
          Machine.control.state = CTRL_DONE;
        }
      }
      break;
    case CTRL_DATA_OUT:
      len = min(Machine.control.length, Machine.control.max_packet);
      if (USBHstartTransaction(Machine.control.hc_num_out,
                               Machine.control.pid,
                               Machine.control.buffer, len) == 0) {
        Machine.control.state = CTRL_DATA_OUT_WAIT;
      }
      else {
        Machine.control.errno = USBHLIB_ERROR_IO;
        Machine.control.state = CTRL_DONE;
      }
      break;
    case CTRL_DATA_OUT_WAIT:
      res = USBHgetTransactionResult(Machine.control.hc_num_out);
      if (res == TR_DONE) {
        len = USBHgetTransactionSize(Machine.control.hc_num_out);
        Machine.control.buffer     += len;
        Machine.control.transfered += len;
        Machine.control.length     -= len;
        if (Machine.control.length == 0) {
          Machine.control.state = CTRL_STATUS_IN;
        }
        else {
          Machine.control.pid = USBtoggleDataPid(Machine.control.pid);
          Machine.control.state = CTRL_DATA_OUT;
        }
        Machine.control.error_count = 0;
      }
      else if (Machine.control.timeout < 0) {
        USBHhaltChannel(Machine.control.hc_num_out);
        Machine.control.errno = USBHLIB_ERROR_TIMEOUT;
        Machine.control.state = CTRL_DONE;
      }
      else if (res == TR_NAK) {
        Machine.control.error_count = 0;
        Machine.control.state = CTRL_DATA_OUT;
      }
      else if (res == TR_STALL) {
        Machine.control.errno = USBHLIB_ERROR_STALL;
        Machine.control.state = CTRL_DONE;
      }
      else if (res == TR_ERROR) {
        if (++Machine.control.error_count < TRANS_MAX_REP_COUNT) {
          Machine.control.state = CTRL_DATA_OUT;
        }
        else {
          Machine.control.errno = USBHLIB_ERROR_IO;
          Machine.control.state = CTRL_DONE;
        }
      }
      break;
    case CTRL_STATUS_IN:
      if (USBHstartTransaction(Machine.control.hc_num_in, PID_DATA1,
                               0, 0) == 0) {
        Machine.control.state = CTRL_STATUS_IN_WAIT;
      }
      else {
        Machine.control.errno = USBHLIB_ERROR_IO;
        Machine.control.state = CTRL_DONE;
      }
      break;
    case CTRL_STATUS_IN_WAIT:
      res = USBHgetTransactionResult(Machine.control.hc_num_in);
      if (res == TR_DONE) {
        Machine.control.errno = USBHLIB_SUCCESS;
        Machine.control.state = CTRL_DONE;
        Machine.control.error_count = 0;
      }
      else if (Machine.control.timeout < 0) {
        USBHhaltChannel(Machine.control.hc_num_in);
        Machine.control.errno = USBHLIB_ERROR_TIMEOUT;
        Machine.control.state = CTRL_DONE;
      }
      else if (res == TR_NAK) {
        Machine.control.error_count = 0;
        Machine.control.state = CTRL_STATUS_IN;
      }
      else if (res == TR_STALL) {
        Machine.control.errno = USBHLIB_ERROR_STALL;
        Machine.control.state = CTRL_DONE;
      }
      else if (res == TR_ERROR) {
        if (++Machine.control.error_count < TRANS_MAX_REP_COUNT) {
          Machine.control.state = CTRL_STATUS_IN;
        }
        else {
          Machine.control.errno = USBHLIB_ERROR_IO;
          Machine.control.state = CTRL_DONE;
        }
      }
      break;
    case CTRL_STATUS_OUT:
      if (USBHstartTransaction(Machine.control.hc_num_out, PID_DATA1,
                               0, 0) == 0) {
        Machine.control.state = CTRL_STATUS_OUT_WAIT;
      }
      else {
        Machine.control.errno = USBHLIB_ERROR_IO;
        Machine.control.state = CTRL_DONE;
      }
      break;
    case CTRL_STATUS_OUT_WAIT:
      res = USBHgetTransactionResult(Machine.control.hc_num_out);
      if (res == TR_DONE) {
        Machine.control.errno = USBHLIB_SUCCESS;
        Machine.control.state = CTRL_DONE;
        Machine.control.error_count = 0;
      }
      else if (Machine.control.timeout < 0) {
        USBHhaltChannel(Machine.control.hc_num_out);
        Machine.control.errno = USBHLIB_ERROR_TIMEOUT;
        Machine.control.state = CTRL_DONE;
      }
      else if (res == TR_NAK) {
        Machine.control.error_count = 0;
        Machine.control.state = CTRL_STATUS_OUT;
      }
      else if (res == TR_STALL) {
        Machine.control.errno = USBHLIB_ERROR_STALL;
        Machine.control.state = CTRL_DONE;
      }
      else if (res == TR_ERROR) {
        if (++Machine.control.error_count < TRANS_MAX_REP_COUNT) {
          Machine.control.state = CTRL_STATUS_OUT;
        }
        else {
          Machine.control.errno = USBHLIB_ERROR_IO;
          Machine.control.state = CTRL_DONE;
        }
      }
      break;
    default: /* CTRL_IDLE, CTRL_DONE */
      break;
  }
  return Machine.control.state == CTRL_DONE;
}

/* Device enumeration state machine.
   The return value is non-zero, if the machine finished its work.
   The return value is zero, if the machine still works. */
static int USBHhandleEnumeration(void) {
  int res;

  switch (Machine.enumeration.state) {
    case ENUM_GET_DEV_DESC: /* Get the first 8 bytes of the device descriptor. */
      res = USBHgetDeviceDescriptor(0, (uint8_t *)&Device.dev_desc, 8);
      if (res == USBHLIB_SUCCESS) {
        /* Set bMaxPacketSize0 for control channels. */
        Machine.control.max_packet = Device.dev_desc.bMaxPacketSize0;
        USBHmodifyChannel(Machine.control.hc_num_out,
                          0, Machine.control.max_packet);
        USBHmodifyChannel(Machine.control.hc_num_in,
                          0, Machine.control.max_packet);
        Machine.enumeration.state = ENUM_GET_FULL_DEV_DESC;
      }
      break;
    case ENUM_GET_FULL_DEV_DESC: /* Get the whole device descriptor. */
      res = USBHgetDeviceDescriptor(0, (uint8_t *)&Device.dev_desc,
                                    sizeof(usb_device_descriptor_t));
      if (res == USBHLIB_SUCCESS) {
        Machine.enumeration.state = ENUM_SET_DEV_ADDR;
      }
      break;
    case ENUM_SET_DEV_ADDR: /* Set device address. */
      res = USBHsetDeviceAddress(0, DEVICE_ADDRESS);
      if (res == USBHLIB_SUCCESS) {
        Device.address = DEVICE_ADDRESS;
        Device.visible_state = ADDRESS;
        /* Update device address for control channels. */
        USBHmodifyChannel(Machine.control.hc_num_in,
                          Device.address, 0);
        USBHmodifyChannel(Machine.control.hc_num_out,
                          Device.address, 0);
        Machine.enumeration.state = ENUM_IDLE;
      }
      break;
    default: /* ENUM_IDLE */
      res = USBHLIB_SUCCESS;
      break;
  }
  Machine.enumeration.errno = res;
  return Machine.enumeration.state == ENUM_IDLE ||
         (res != USBHLIB_SUCCESS && res != USBHLIB_IN_PROGRESS);
}

void USBHcoreProcess() {
  switch (Machine.g_state) {
    case HOST_CONTROL_TRANSFER:
      if (USBHhandleControlTransfer())
        Machine.g_state = Machine.g_prev_state;
      return;
    case HOST_ENUMERATION:
      if (USBHhandleEnumeration()) {
        Machine.g_state = HOST_IDLE;
        if (Machine.enumeration.errno != USBHLIB_SUCCESS)
          USBHdeviceHardReset(DEVICE_RESET_TIME_MS);
          /* The disconnection state is detected and USBHcoreDeInit
             is called. */
      }
      return;
    case HOST_CLASS:
      if (Machine.class.machine) {
        if (Machine.class.machine(Machine.class.parameter)) {
          Machine.g_state = HOST_IDLE;
          Machine.class.machine = 0;
          Machine.class.at_sof = 0;
          Machine.class.at_disconnect = 0;
          Machine.class.parameter = 0;
        }
      }
      return;
    default: /* HOST_IDLE */
      return;
  }
}

/** Library functions called ouside the interrupt context **/

static void USBHsubmitControlRequest(usb_setup_packet_t const *setup,
                                     uint8_t *buffer) {
  /* Change the global state. */
  Machine.g_prev_state = Machine.g_state;
  Machine.g_state = HOST_CONTROL_TRANSFER;
  /* Prepare transfer. */
  Machine.control.state = CTRL_SETUP;
  Machine.control.errno = USBHLIB_IN_PROGRESS;
  Machine.control.setup.bmRequestType = setup->bmRequestType;
  Machine.control.setup.bRequest = setup->bRequest;
  Machine.control.setup.wValue = HTOUSBS(setup->wValue);
  Machine.control.setup.wIndex = HTOUSBS(setup->wIndex);
  Machine.control.setup.wLength = HTOUSBS(setup->wLength);
  Machine.control.timeout = -1;
  Machine.control.buffer = buffer;
  /* wLength cannot be used, because of changed byte order. */
  Machine.control.length = setup->wLength;
  Machine.control.transfered = 0;
  Machine.control.error_count = 0;
}

int USBHcontrolRequest(int synch, usb_setup_packet_t const *setup,
                       uint8_t *buffer, uint32_t *length) {
  if (synch == 0) {
    if (Machine.control.state == CTRL_IDLE) {
      USBHsubmitControlRequest(setup, buffer);
    }
    else if (Machine.control.state == CTRL_DONE) {
      Machine.control.state = CTRL_IDLE;
      if (length)
        *length = Machine.control.transfered;
    }
  }
  else {
    uint32_t x;
    x = USBHprotectInterrupt();
    if (Machine.control.state != CTRL_IDLE) {
      USBHunprotectInterrupt(x);
      return USBHLIB_ERROR_BUSY;
    }
    else {
      USBHsubmitControlRequest(setup, buffer);
      while (Machine.control.state != CTRL_DONE) {
        USBHunprotectInterrupt(x);
        Delay(200); /* How long should we wait? */
        x = USBHprotectInterrupt();
      }
      Machine.control.state = CTRL_IDLE;
      if (length)
        *length = Machine.control.transfered;
    }
    USBHunprotectInterrupt(x);
  }
  return Machine.control.errno;
}

int USBHgetDevice(usb_speed_t *speed, uint8_t *dev_addr,
                  usb_device_descriptor_t *dev_desc) {
  int res;
  uint32_t x;

  x = USBHprotectInterrupt();
  if (Device.visible_state == ADDRESS) {
    if (speed)
      *speed = Device.speed;
    if (dev_addr)
      *dev_addr = Device.address;
    if (dev_desc)
      memcpy(dev_desc, &Device.dev_desc,
             sizeof(usb_device_descriptor_t));
    res = USBHLIB_SUCCESS;
  }
  else {
    res = USBHLIB_ERROR_NO_DEVICE;
  }
  USBHunprotectInterrupt(x);

  return res;
}

usb_visible_state_t USBHgetVisibleDeviceState() {
  /* Reading an integer value from memory is atomic on ARM. Therefore
     we do not need to protect them by
     USBHprotectInterrupt/USBHunprotectInterrupt. */
  return Device.visible_state;
}

int USBHsetClassMachine(int (*machine)(void *),
                        void (*at_sof)(void *, uint16_t),
                        void (*at_disconnect)(void *),
                        void *parameter) {
  int res;
  uint32_t x;

  x = USBHprotectInterrupt();
  if (Machine.g_state == HOST_IDLE) {
    Machine.class.machine = machine;
    Machine.class.at_sof = at_sof;
    Machine.class.at_disconnect = at_disconnect;
    Machine.class.parameter = parameter;
    Machine.g_state = HOST_CLASS;
    res = USBHLIB_SUCCESS;
  }
  else {
    res = USBHLIB_ERROR_BUSY;
  }
  USBHunprotectInterrupt(x);

  return res;
}
