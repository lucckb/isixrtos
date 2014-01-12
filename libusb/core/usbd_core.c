#include <string.h>
#include <usb_endianness.h>
#include <usbd_api.h>
#include <usbd_callbacks.h>
#include <usbd_core.h>

/* Some useful abbreviations */
#define IN_REQ   (DEVICE_TO_HOST)
#define STD_REQ  (STANDARD_REQUEST)
#define IN_STD_REQ_DEV \
        (DEVICE_TO_HOST | STANDARD_REQUEST | DEVICE_RECIPIENT)
#define IN_STD_REQ_IF \
        (DEVICE_TO_HOST | STANDARD_REQUEST | INTERFACE_RECIPIENT)
#define IN_STD_REQ_EP \
        (DEVICE_TO_HOST | STANDARD_REQUEST | ENDPOINT_RECIPIENT)
#define OUT_STD_REQ_DEV \
        (HOST_TO_DEVICE | STANDARD_REQUEST | DEVICE_RECIPIENT)

/** USB device core structure **/

/* States of the control pipe */
typedef enum {
  IDLE, DATA_IN, DATA_OUT, LAST_DATA_IN,
  WAIT_STATUS_IN, WAIT_STATUS_OUT
} usb_control_state_t;

typedef struct {
  usb_visible_state_t        visibleState;
  usb_control_state_t        controlState;
  usb_setup_packet_t         setup;
  usbd_callback_list_t const *callback;
  uint8_t              const *txdata;
  uint8_t                    *rxdata;
  uint16_t                   length;
  uint8_t                    maxPacketSize0;
} usb_device_state_t;

static usb_device_state_t DeviceState;

/** Device initialization **/

int USBDcoreConfigure() {
  memset(&DeviceState, 0, sizeof(usb_device_state_t));
  DeviceState.callback = USBDgetApplicationCallbacks();
  if (DeviceState.callback == 0)
    return -1;
  if (DeviceState.callback->Configure)
    if (DeviceState.callback->Configure() < 0)
      return -1;
  DeviceState.visibleState = POWERED;
  DeviceState.controlState = IDLE;
  return 0;
}

/** Interrupts handling **/

void USBDreset(usb_speed_t speed) {
  memset(&DeviceState.setup, 0, sizeof(usb_setup_packet_t));
  DeviceState.visibleState = DEFAULT;
  DeviceState.controlState = IDLE;
  if (DeviceState.callback->Reset)
    DeviceState.maxPacketSize0 = DeviceState.callback->Reset(speed);
  else
    DeviceState.maxPacketSize0 = MAX_LS_CONTROL_PACKET_SIZE;
  /* Default control endpoint is now configured. Set the default
     address and enable USB function. */
  USBDsetDeviceAddress(SET_ADDRESS_RESET, 0);
}

void USBDsuspend() {
  if (DeviceState.callback->Suspend)
    DeviceState.callback->Suspend();
  DeviceState.visibleState |= SUSPENDED;
}

void USBDwakeup() {
  if (DeviceState.callback->Wakeup)
    DeviceState.callback->Wakeup();
  DeviceState.visibleState &= ~SUSPENDED;
}

void USBDsof(uint16_t frameNumber) {
  if (DeviceState.callback->SoF)
    DeviceState.callback->SoF(frameNumber);
}

/** Transfers handling **/

/* Control pipe transfer routines */
static void Setup0(usb_device_state_t *);
static void NoDataSetup0(usb_device_state_t *);
static void InDataSetup0(usb_device_state_t *);
static void In0(usb_device_state_t *);
static void DataStageIn0(usb_device_state_t *);
static void OutDataSetup0(usb_device_state_t *);
static void Out0(usb_device_state_t *);
static void DataStageOut0(usb_device_state_t *);

/* Multiple endpoint transactions are served one by one. For every
   unserviced transaction the interrupt is signaled again and the
   USBtransfer function is called. */
void USBDtransfer(uint8_t ep, usb_pid_t token) {
  if (ep == 0) { /* control transfer */
    if (DeviceState.visibleState == DEFAULT ||
        DeviceState.visibleState == ADDRESS ||
        DeviceState.visibleState == CONFIGURED) {
      switch (token) {
        case PID_SETUP:
          Setup0(&DeviceState);
          break;
        case PID_OUT:
          Out0(&DeviceState);
          break;
        case PID_IN:
          In0(&DeviceState);
          break;
        default:
          break;
      }
    }
  }
  else { /* non-control transfer */
    if (DeviceState.visibleState == CONFIGURED) {
      switch (token) {
        case PID_OUT:
          if (DeviceState.callback->EPout[ep - 1])
            DeviceState.callback->EPout[ep - 1]();
          break;
        case PID_IN:
          if (DeviceState.callback->EPin[ep - 1])
            DeviceState.callback->EPin[ep - 1]();
          break;
        default:
          break;
      }
    }
  }
}

/* Process the SETUP token on endpoint 0. */
void Setup0(usb_device_state_t *ds) {
  uint16_t len;

  len = USBDread0((uint8_t*)&(ds->setup), sizeof(usb_setup_packet_t));

  if (ds->controlState != IDLE || len != sizeof(usb_setup_packet_t)) {
    ds->controlState = IDLE;
    USBDendPoint0RxSTALL();
    USBDendPoint0TxSTALL();
    return;
  }

  ds->setup.wValue  = USBTOHS(ds->setup.wValue);
  ds->setup.wIndex  = USBTOHS(ds->setup.wIndex);
  ds->setup.wLength = USBTOHS(ds->setup.wLength);

  if (ds->setup.wLength == 0) {
    /* For SETUP without data stage the direction bit is ignored. */
    ds->setup.bmRequestType &= ~REQUEST_DIRECTION;
    NoDataSetup0(ds);
  }
  else if ((ds->setup.bmRequestType & REQUEST_DIRECTION) == IN_REQ) {
    InDataSetup0(ds);  /* SETUP with read data stage */
  }
  else {
    OutDataSetup0(ds); /* SETUP with write data stage */
  }
}

/* Process a request without data stage. */
void NoDataSetup0(usb_device_state_t *ds) {
  usb_result_t result;
  uint8_t      recipient;

  result = REQUEST_ERROR;
  if ((ds->setup.bmRequestType & REQUEST_TYPE) == STANDARD_REQUEST) {
    recipient = ds->setup.bmRequestType & REQUEST_RECIPIENT;
    if (recipient == DEVICE_RECIPIENT && ds->setup.wIndex == 0) {
      if (ds->setup.bRequest == SET_ADDRESS &&
          ds->setup.wValue <= 127 &&
          (ds->visibleState == DEFAULT ||
           ds->visibleState == ADDRESS)) {
        USBDsetDeviceAddress(SET_ADDRESS_REQUEST, ds->setup.wValue);
        result = REQUEST_SUCCESS;
      }
      else if (ds->setup.bRequest == SET_CONFIGURATION &&
               (ds->visibleState == ADDRESS ||
                ds->visibleState == CONFIGURED)) {
        if (ds->callback->SetConfiguration)
          result = ds->callback->SetConfiguration(ds->setup.wValue);
        if (result == REQUEST_SUCCESS &&
            ds->visibleState == ADDRESS &&
            ds->setup.wValue != 0) {
          ds->visibleState = CONFIGURED;
        }
        else if (ds->visibleState == CONFIGURED &&
                 ds->setup.wValue == 0) {
          ds->visibleState = ADDRESS;
        }
      }
      else if (ds->setup.bRequest == SET_FEATURE &&
               (ds->visibleState == ADDRESS ||
                ds->visibleState == CONFIGURED) &&
               ds->setup.wValue == DEVICE_REMOTE_WAKEUP) {
        if (ds->callback->SetDeviceFeature)
          result = ds->callback->SetDeviceFeature(ds->setup.wValue);
      }
      else if (ds->setup.bRequest == SET_FEATURE &&
               ds->setup.wValue == TEST_MODE) {
        /* TODO: Implementation of the device TEST_MODE feature */
      }
      else if (ds->setup.bRequest == CLEAR_FEATURE &&
               (ds->visibleState == ADDRESS ||
                ds->visibleState == CONFIGURED)) {
        if (ds->callback->ClearDeviceFeature)
          result = ds->callback->ClearDeviceFeature(ds->setup.wValue);
      }
    }
    else if (recipient == INTERFACE_RECIPIENT) {
      if (ds->setup.bRequest == SET_INTERFACE &&
          ds->visibleState == CONFIGURED) {
        if (ds->callback->SetInterface)
          result = ds->callback->SetInterface(ds->setup.wIndex,
                                              ds->setup.wValue);
      }
      else if (ds->setup.bRequest == SET_FEATURE ||
               ds->setup.bRequest == CLEAR_FEATURE) {
        /* There are no feature selectors for interface recipient. */
        result = REQUEST_ERROR;
      }
    }
    else if (recipient == ENDPOINT_RECIPIENT &&
             ((ds->visibleState == ADDRESS &&
               ds->setup.wIndex == 0) ||
              ds->visibleState == CONFIGURED)) {
      if (ds->setup.bRequest == SET_FEATURE &&
          ds->setup.wValue == ENDPOINT_HALT) {
        result = USBDsetEndPointHalt(ds->setup.wIndex);
      }
      else if (ds->setup.bRequest == CLEAR_FEATURE &&
               ds->setup.wValue == ENDPOINT_HALT) {
        result = USBDclearEndPointHalt(ds->setup.wIndex);
      }
    }
  }
  else if ((ds->setup.bmRequestType & REQUEST_TYPE) != STD_REQ) {
    if (ds->callback->ClassNoDataSetup)
      result = ds->callback->ClassNoDataSetup(&ds->setup);
  }

  if (result == REQUEST_SUCCESS) {
    /* No data - go to the IN status stage.
       Send zero length data packet. */
    ds->controlState = WAIT_STATUS_IN;
    USBDwrite0(0, 0);
    USBDendPoint0RxSTALL();
    USBDendPoint0TxVALID();
  }
  else { /* REQUEST_ERROR */
    ds->controlState = IDLE;
    USBDendPoint0RxSTALL();
    USBDendPoint0TxSTALL();
  }
}

/* Process a request with read data stage. */
void InDataSetup0(usb_device_state_t *ds) {
  static uint16_t buffer16;
  static uint8_t  buffer8;
  usb_result_t    result;

  result = REQUEST_ERROR;

  if ((ds->setup.bmRequestType == IN_STD_REQ_DEV ||
       ds->setup.bmRequestType == IN_STD_REQ_IF) &&
      ds->setup.bRequest == GET_DESCRIPTOR) {
    if (ds->callback->GetDescriptor)
      result = ds->callback->GetDescriptor(ds->setup.wValue,
                                           ds->setup.wIndex,
                                           &ds->txdata,
                                           &ds->length);
  }
  else if (ds->setup.bmRequestType == IN_STD_REQ_DEV &&
           ds->setup.bRequest == GET_STATUS &&
           ds->setup.wValue == 0 &&
           ds->setup.wIndex == 0 &&
           ds->setup.wLength == 2) {
    if (ds->callback->GetStatus) {
      buffer16 = HTOUSBS(ds->callback->GetStatus());
      ds->txdata = (uint8_t const *)&buffer16;
      ds->length = 2;
      result = REQUEST_SUCCESS;
    }
  }
  else if (ds->setup.bmRequestType == IN_STD_REQ_IF &&
           ds->setup.bRequest == GET_STATUS &&
           ds->setup.wValue == 0 &&
           ds->setup.wLength == 2) {
    buffer16 = HTOUSBS(0);
    ds->txdata = (uint8_t const *)&buffer16;
    ds->length = 2;
    result = REQUEST_SUCCESS;
  }
  else if (ds->setup.bmRequestType == IN_STD_REQ_EP &&
           ds->setup.bRequest == GET_STATUS &&
           ds->setup.wValue == 0 &&
           ds->setup.wLength == 2) {
    result = USBDgetEndPointStatus(ds->setup.wIndex, &buffer16);
    buffer16 = HTOUSBS(buffer16);
    ds->txdata = (uint8_t const *)&buffer16;
    ds->length = 2;
  }
  else if (ds->setup.bmRequestType == IN_STD_REQ_DEV &&
           ds->setup.bRequest == GET_CONFIGURATION &&
           ds->setup.wValue == 0 &&
           ds->setup.wIndex == 0 &&
           ds->setup.wLength == 1) {
    if (ds->visibleState == ADDRESS) {
      buffer8 = 0;
      ds->txdata = &buffer8;
      ds->length = 1;
      result = REQUEST_SUCCESS;
    }
    else if (ds->visibleState == CONFIGURED) {
      if (ds->callback->GetConfiguration) {
        buffer8 = ds->callback->GetConfiguration();
        ds->txdata = &buffer8;
        ds->length = 1;
        result = REQUEST_SUCCESS;
      }
    }
  }
  else if (ds->setup.bmRequestType == IN_STD_REQ_IF &&
           ds->setup.bRequest == GET_INTERFACE &&
           ds->setup.wValue == 0 &&
           ds->setup.wLength == 1 &&
           ds->visibleState == CONFIGURED) {
    if (ds->callback->GetInterface) {
      result = ds->callback->GetInterface(ds->setup.wIndex, &buffer8);
      ds->txdata = &buffer8;
      ds->length = 1;
    }
  }
  else if (ds->setup.bmRequestType == IN_STD_REQ_EP &&
           ds->setup.bRequest == SYNCH_FRAME &&
           ds->setup.wValue == 0 &&
           ds->setup.wLength == 2) {
    /* TODO: Implementation of the endpoint SYNCH_FRAME request */
  }
  else if ((ds->setup.bmRequestType & REQUEST_TYPE) != STD_REQ) {
    if (ds->callback->ClassInDataSetup)
      result = ds->callback->ClassInDataSetup(&ds->setup,
                                              &ds->txdata,
                                              &ds->length);
  }

  if (result == REQUEST_SUCCESS) {
    ds->controlState = DATA_IN;
    /* If data to send are longer than the wLength field,
       only the initial bytes are returned. */
    if (ds->length > ds->setup.wLength)
      ds->length = ds->setup.wLength;
    DataStageIn0(ds);
  }
  else { /* REQUEST_ERROR */
    ds->controlState = IDLE;
    USBDendPoint0TxSTALL();
    USBDendPoint0RxSTALL();
  }
}

/* Process the IN token on endpoint 0. */
void In0(usb_device_state_t *ds) {
  switch (ds->controlState) {
    case DATA_IN:
    case LAST_DATA_IN:
      DataStageIn0(ds);
      return;
    case DATA_OUT: /* The host aborts the write data stage. */
      ds->controlState = IDLE;
      USBDendPoint0TxSTALL();
      USBDendPoint0RxVALID();
      return;
    case WAIT_STATUS_IN:
      /* The no-data or write control request is finished. */
      if (ds->setup.bmRequestType == OUT_STD_REQ_DEV &&
          ds->setup.bRequest == SET_ADDRESS &&
          ds->setup.wValue <= 127 &&
          ds->setup.wIndex == 0 &&
          ds->setup.wLength == 0 &&
          (ds->visibleState == DEFAULT ||
           ds->visibleState == ADDRESS)) {
        USBDsetDeviceAddress(SET_ADDRESS_STATUS, ds->setup.wValue);
        if (ds->visibleState == DEFAULT && ds->setup.wValue != 0)
          ds->visibleState = ADDRESS;
        else if (ds->visibleState == ADDRESS && ds->setup.wValue == 0)
          ds->visibleState = DEFAULT;
      }
      else if ((ds->setup.bmRequestType & REQUEST_TYPE) != STD_REQ) {
        if (ds->callback->ClassStatusIn)
          ds->callback->ClassStatusIn(&ds->setup);
      }
      ds->controlState = IDLE;
      USBDendPoint0TxSTALL();
      USBDendPoint0RxVALID();
      return;
    default:
      ds->controlState = IDLE;
      USBDendPoint0TxSTALL();
      USBDendPoint0RxSTALL();
  }
}

/* Process the read data stage of the request. */
void DataStageIn0(usb_device_state_t *ds) {
  if (ds->controlState == DATA_IN) {
    uint16_t length;

    /* If data to send are shorter than the wLength field, the
       device indicates the end of the transfer by sending
       a short packet when further data is requested. */
    if (ds->length < ds->maxPacketSize0 ||
        (ds->length == ds->maxPacketSize0 &&
         ds->setup.wLength % ds->maxPacketSize0 == 0)) {
      ds->controlState = LAST_DATA_IN;
      length = ds->length;
    }
    else
      length = ds->maxPacketSize0;

    USBDwrite0(ds->txdata, length);
    USBDendPoint0TxVALID();
    /* The host can abort the read data stage, enable the receiver. */
    USBDendPoint0RxVALID();

    ds->length -= length;
    ds->txdata += length;
  }
  else if (ds->controlState == LAST_DATA_IN) {
    /* No more data to send - stall the transmitter. */
    ds->controlState = WAIT_STATUS_OUT;
    USBDendPoint0TxSTALL();
    USBDendPoint0RxVALID();
  }
}

/* Process a request with write data stage. */
void OutDataSetup0(usb_device_state_t *ds) {
  usb_result_t result;

  result = REQUEST_ERROR;
  if (ds->setup.bmRequestType == OUT_STD_REQ_DEV &&
      ds->setup.bRequest == SET_DESCRIPTOR &&
      (ds->visibleState == ADDRESS ||
       ds->visibleState == CONFIGURED)) {
    if (ds->callback->SetDescriptor)
      result = ds->callback->SetDescriptor(ds->setup.wValue,
                                           ds->setup.wIndex,
                                           ds->setup.wLength,
                                           &ds->rxdata);
  }
  else if ((ds->setup.bmRequestType & REQUEST_TYPE) != STD_REQ) {
    if (ds->callback->ClassOutDataSetup)
      result = ds->callback->ClassOutDataSetup(&ds->setup,
                                               &ds->rxdata);
  }

  if (result == REQUEST_SUCCESS) {
    ds->controlState = DATA_OUT;
    ds->length = ds->setup.wLength;
    /* Prepare for the IN status stage or
       aborting the read data stage. */
    USBDwrite0(0, 0);
    USBDendPoint0TxVALID();
    /* Prepare for the write data stage. */
    USBDendPoint0RxVALID();
  }
  else { /* REQUEST_ERROR */
    ds->controlState = IDLE;
    USBDendPoint0TxSTALL();
    USBDendPoint0RxSTALL();
  }
}

/* Process the OUT token on endpoint 0. */
void Out0(usb_device_state_t *ds) {
  switch (ds->controlState) {
    case DATA_OUT:
      DataStageOut0(ds);
      return;
    case WAIT_STATUS_OUT: /* The read data stage is finished. */
    case DATA_IN:         /* The host aborts the read data stage. */
    case LAST_DATA_IN:    /* The host aborts the read data stage. */
      ds->controlState = IDLE;
      USBDendPoint0TxSTALL();
      USBDendPoint0RxVALID();
      return;
    default:
      ds->controlState = IDLE;
      USBDendPoint0TxSTALL();
      USBDendPoint0RxSTALL();
  }
}

/* Process the write data stage of the request. */
void DataStageOut0(usb_device_state_t *ds) {
  uint16_t length;

  length = USBDread0(ds->rxdata, ds->length);
  ds->length -= length;
  ds->rxdata += length;
  if (ds->length == 0) {
    /* Receive buffer is full - stall the receiver.
       IN status stage is expected - enable transmitter. */
    ds->controlState = WAIT_STATUS_IN;
    /* USBwrite0(0, 0); Already written in OutDataSetup0. */
    USBDendPoint0RxSTALL();
    USBDendPoint0TxVALID();
  }
  else {
    /* Stay in the DATA_OUT state. Wait for the next data - enable
       receiver. The host can abort the write data stage - enable also
       the transmitter. */
    USBDendPoint0RxVALID();
    USBDendPoint0TxVALID();
  }
}
