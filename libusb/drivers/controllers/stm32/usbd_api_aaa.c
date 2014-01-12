#include <usb_regs.h>
#include <usbd_api.h>

/** USB device API for STM32F102, STM32F103 and STM32L1xx **/

/** General API **/

/* Set the device address and enable the function. */
void USBDsetDeviceAddress(set_address_t token, uint8_t address) {
  if (token == SET_ADDRESS_RESET || token == SET_ADDRESS_STATUS)
    _SetDADDR(address | DADDR_EF);
}

/** Low level USB buffer handling **/

/* Possible values of receive buffers are:
   2, 4, 6, 8, ..., 60, 62, 64, 96, 128, ..., 512. */
static uint16_t AdjustRxBufferSize(uint16_t size) {
  if (size == 0)
    return 2;
  else if (size <= 62)
    return (size + 1) & ~1;
  else
    return (size + 31) & ~31;
}

/* Transmit buffer size must be even. */
static uint16_t AdjustTxBufferSize(uint16_t size) {
  return (size + 1) & ~1;
}

/* The first free address in the packet memory area (PMA) */
static uint16_t pmaAddress;

/* The first address in the PMA for non-control endpoint buffers */
static uint16_t pmaNonControlAddress;

/* The table maps the endpoint address to the internal endpoint index.
   The entries endPointRxIndex[0] and endPointTxIndex[0] are always
   zero. */
static uint16_t endPointRxIndex[USB_EP_MAX_COUNT] = {0};
static uint16_t endPointTxIndex[USB_EP_MAX_COUNT] = {0};
static uint16_t endPointTxSize[USB_EP_MAX_COUNT];

/* Configured endpoints bit map */
static uint16_t configuredRxEndPoints;
static uint16_t configuredTxEndPoints;
static uint16_t configuredDblBuffEndPoints;

/* The number of configured internal endpoints including the default
   control endpoint */
static uint16_t endPointCount = 0;

/** Library private API **/

static uint8_t const *pTxBuffer[USB_EP_MAX_COUNT];
static uint32_t      txBufferCount[USB_EP_MAX_COUNT];

void USBDcontinueInTransfer(uint8_t ep) {
  uint32_t count;

  count = txBufferCount[ep];
  if (count > 0) {
    if (count > endPointTxSize[ep])
      count = endPointTxSize[ep];
    USBDwrite(ep, pTxBuffer[ep], count);
    txBufferCount[ep] -= count;
    pTxBuffer[ep]     += count;
    if (txBufferCount[ep] == 0)
      pTxBuffer[ep] = 0; /* Transfer is finished. */
  }
}

/** Endpoint configuration API **/

/* Disable all non-control endpoints. */
void USBDdisableAllNonControlEndPoints() {
  unsigned i;

  for (i = 1; i < EP_MAX_COUNT; ++i)
    _SetEPRxTxStatus(i, EP_RX_DIS, EP_TX_DIS);
  for (i = 1; i < USB_EP_MAX_COUNT; ++i) {
    endPointRxIndex[i] = endPointTxIndex[i] = endPointTxSize[i] = 0;
    pTxBuffer[i] = 0;
    txBufferCount[i] = 0;
  }
  endPointCount = 1;
  configuredRxEndPoints &= 1;
  configuredTxEndPoints &= 1;
  configuredDblBuffEndPoints = 0;
  pmaAddress = pmaNonControlAddress;
}

/* Initialize a given endpoint. Control endpoint 0 must be configured
   first. Calculate addresses in the packet memory area (PMA).
    endPoint   - endpoint number
    type       - endpoint transfer type
    rxBuffSize - receive buffer size (possible values:
                 2, 4, 6, 8, ..., 60, 62, 64, 96, 128, ..., 512)
    txBuffSize - transmit buffer size
   Return REQUEST_SUCCESS if success and REQUEST_ERROR otherwise. */
usb_result_t USBDendPointConfigure(uint8_t endPoint,
                                   usb_transfer_t type,
                                   uint16_t rxBuffSize,
                                   uint16_t txBuffSize) {
  /* Microcontroller supports only 8 endpoints.
     The standard allows up to 16 endpoints. */
  if (endPointCount >= EP_MAX_COUNT ||
      endPoint < 0 || endPoint >= USB_EP_MAX_COUNT)
    return REQUEST_ERROR;
  switch (type) {
    case CONTROL_TRANSFER:
      /* The address of the default control endpoint must be zero.
         Allowed buffer sizes are only 8, 16, 32 or 64 bytes. */
      if (endPoint != ENDP0 || rxBuffSize != txBuffSize ||
          (rxBuffSize != 8  && rxBuffSize != 16 &&
           rxBuffSize != 32 && rxBuffSize != 64))
        return REQUEST_ERROR;

      /* Reset TX buffer. */
      pTxBuffer[0] = 0;
      txBufferCount[0] = 0;

      /* Assume the default control endpoint is configired first. */
      _SetBTABLE(0);
      /* Reserve 64 bytes for buffer description table. */
      pmaAddress = 64;

      /* The default control endpoint should have the highest
         priority, hence it is configured under index 0. */
      _SetEPType(0, EP_CONTROL);
      _SetEPNumber(0, ENDP0);

      _SetEPTxAddr(0, pmaAddress);
      _SetEPTxCount(0, 0);
      pmaAddress += txBuffSize;
      endPointTxSize[0] = txBuffSize;

      _SetEPRxAddr(0, pmaAddress);
      _SetEPRxCount(0, rxBuffSize);
      pmaAddress += rxBuffSize;

      /* endPointIndex[ENDP0] = 0; Always set. */
      endPointCount = 1;
      configuredRxEndPoints = configuredTxEndPoints = 1;
      configuredDblBuffEndPoints = 0;
      pmaNonControlAddress = pmaAddress;

      /* Clear the STATUS_OUT (EP_KIND) bit. This allows that OUT
         status transaction (normally without data) can have any
         number of data bytes. */
      _Clear_Status_Out(0);
      _SetEPRxTxStatus(0, EP_RX_VALID, EP_TX_STALL);
      break;
    case ISOCHRONOUS_TRANSFER:
      if (endPoint == ENDP0 || (txBuffSize == 0 && rxBuffSize == 0))
        return REQUEST_ERROR;

      if (txBuffSize > 0) {
        _SetEPType(endPointCount, EP_ISOCHRONOUS);
        _SetEPNumber(endPointCount, endPoint);
        _ClearDTOG_TX(endPointCount);
        txBuffSize = AdjustTxBufferSize(txBuffSize);
        endPointTxSize[endPoint] = txBuffSize;
        _SetEPDblBuf0Addr(endPointCount, pmaAddress);
        pmaAddress += txBuffSize;
        _SetEPDblBuf1Addr(endPointCount, pmaAddress);
        pmaAddress += txBuffSize;
        endPointTxIndex[endPoint] = endPointCount;
        configuredTxEndPoints |= 1 << endPoint;
        configuredDblBuffEndPoints |= 1 << endPoint;
        _SetEPDblBuffCount(endPointCount, EP_DBUF_IN, 0);
        /* Isochronous endpoint cannot be in NAK or STALL state. */
        _SetEPRxTxStatus(endPointCount, EP_RX_DIS, EP_TX_VALID);
        ++endPointCount;
      }

      if (rxBuffSize > 0) {
        if (endPointCount >= EP_MAX_COUNT)
          return REQUEST_ERROR;
        _SetEPType(endPointCount, EP_ISOCHRONOUS);
        _SetEPNumber(endPointCount, endPoint);
        _ClearDTOG_RX(endPointCount);
        rxBuffSize = AdjustRxBufferSize(rxBuffSize);
        _SetEPDblBuf0Addr(endPointCount, pmaAddress);
        pmaAddress += rxBuffSize;
        _SetEPDblBuf1Addr(endPointCount, pmaAddress);
        pmaAddress += rxBuffSize;
        endPointRxIndex[endPoint] = endPointCount;
        configuredRxEndPoints |= 1 << endPoint;
        configuredDblBuffEndPoints |= 1 << endPoint;
        _SetEPDblBuffCount(endPointCount, EP_DBUF_OUT, rxBuffSize);
        _SetEPRxTxStatus(endPointCount, EP_RX_VALID, EP_TX_DIS);
        ++endPointCount;
      }

      break;
    case BULK_TRANSFER:
      /* Use single buffered bulk endpoint. Double buffering prevents
         from NAKing transactions, but decrease buffer size and then
         increase transmission overhead. */
    case INTERRUPT_TRANSFER:
      if (endPoint == ENDP0 || (txBuffSize == 0 && rxBuffSize == 0))
        return REQUEST_ERROR;

      _SetEPType(endPointCount,
                 type == BULK_TRANSFER ? EP_BULK : EP_INTERRUPT);
      _SetEPNumber(endPointCount, endPoint);
      /* Interrupt endpoints do not use the EP_KIND bit. For bulk
         endpoints the EP_KIND bit control double buffering. */
      _ClearEP_KIND(endPointCount);

      if (txBuffSize > 0) {
        _SetEPTxAddr(endPointCount, pmaAddress);
        txBuffSize = AdjustTxBufferSize(txBuffSize);
        endPointTxSize[endPoint] = txBuffSize;
        _SetEPTxCount(endPointCount, 0);
        pmaAddress += txBuffSize;
        endPointTxIndex[endPoint] = endPointCount;
        configuredTxEndPoints |= 1 << endPoint;
        _ClearDTOG_TX(endPointCount);
        _SetEPTxStatus(endPointCount, EP_TX_NAK);
      }

      if (rxBuffSize > 0) {
        _SetEPRxAddr(endPointCount, pmaAddress);
        rxBuffSize = AdjustRxBufferSize(rxBuffSize);
        _SetEPRxCount(endPointCount, rxBuffSize);
        pmaAddress += rxBuffSize;
        endPointRxIndex[endPoint] = endPointCount;
        configuredRxEndPoints |= 1 << endPoint;
        _ClearDTOG_RX(endPointCount);
        _SetEPRxStatus(endPointCount, EP_RX_VALID);
      }

      configuredDblBuffEndPoints &= ~(1 << endPoint);
      ++endPointCount;
      break;
  }
  if (pmaAddress <= 512)
    return REQUEST_SUCCESS;
  else
    return REQUEST_ERROR;
}

/** Low level USB buffer handling **/

/* Copy data from a buffer in the user memory
   to the packet memory area (PMA).
    pma_offset - destination offset in the PMA
    buffer     - source pointer
    count      - the number of bytes to be copied */
static void LowLevelWrite(uint32_t pma_offset, uint8_t const *buffer,
                          uint32_t count) {
  uint32_t       i, n;
  uint16_t       *pma;
  uint16_t const *buffer16;

  /* We want to copy two byte data pieces. */
  pma = (uint16_t *)(PMAAddr + 2 * pma_offset);
  buffer16 = (uint16_t const *)buffer;

  n = count & ~1;
  for (i = 0; i < n; i += 2)
    pma[i] = buffer16[i >> 1];
  /* If the buffer contains an odd number of bytes, we copy the last
     one. The size of the PMA buffer is always even, hence we do not
     need to care for the high byte of pma[n]. */
  if (n < count)
    pma[n] = buffer[n];
}

/* Copy data from the packet memory area (PMA)
   to a buffer in the user memory.
    pma_offset - source offset in the PMA
    buffer     - destination pointer
    count      - the number of bytes to be copied */
static void LowLevelRead(uint32_t pma_offset, uint8_t *buffer,
                         uint32_t count) {
  uint32_t       i, n;
  uint16_t const *pma;
  uint16_t       *buffer16;

  /* We want to copy two byte data pieces. */
  pma = (uint16_t const *)(PMAAddr + 2 * pma_offset);
  buffer16 = (uint16_t *)buffer;

  n = count & ~1;
  for (i = 0; i < n; i += 2)
    buffer16[i >> 1] = pma[i];
  /* If the buffer contains an odd number of bytes,
     we copy the last one. */
  if (n < count)
    buffer[n] = pma[n];
}

/** Microcontroller independent endpoint API **/

/* Write data to the selected endpoint. Only one packet can be sent.
   User buffer can be reused after function return - copy semantics.
    ep    - endpoint number
    buff  - the pointer to the buffer containing data to be written
            to the endpoint
    count - the number of data to be written in bytes
   Return the number of bytes copied. */
uint16_t USBDwrite(uint8_t ep, uint8_t const *buff, uint16_t count) {
  uint32_t pma_offset;
  uint16_t configured, endp_index, buffer_toggle_bit;

  /* Here and in the next functions, the value of the variable
     configured is 0, if ep >= USB_EP_MAX_COUNT. */
  configured = configuredTxEndPoints & (1 << ep);
  endp_index = endPointTxIndex[ep];
  buffer_toggle_bit = 0;
  if (configured & configuredDblBuffEndPoints) {
    buffer_toggle_bit = _GetENDPOINT(endp_index) & EP_DTOG_TX;
    if (buffer_toggle_bit) {
      pma_offset = _GetEPDblBuf0Addr(endp_index);
    }
    else {
      pma_offset = _GetEPDblBuf1Addr(endp_index);
    }
  }
  else if (configured)
    pma_offset = _GetEPTxAddr(endp_index);
  else
    return 0;

  if (count > endPointTxSize[ep])
    count = endPointTxSize[ep];
  if (buff)
    LowLevelWrite(pma_offset, buff, count);
  else
    count = 0;

  /* Update the data length in the control register. */
  if (configured & configuredDblBuffEndPoints) {
    if (buffer_toggle_bit) {
      _SetEPDblBuf0Count(endp_index, EP_DBUF_IN, count);
    }
    else {
      _SetEPDblBuf1Count(endp_index, EP_DBUF_IN, count);
    }
  }
  else if (configured) {
    _SetEPTxCount(endp_index, count);
    _SetEPTxStatus(endp_index, EP_TX_VALID);
  }

  return count;
}

/* Write data to the selected endpoint. Arbitrary number of packets
   can be sent - meant for bulk transfers. The buffer cannot be reused
   until the end of the data transfer - non-copy semantics.
    ep    - endpoint number
    buff  - a pointer to the buffer containing data to
            be written to the endpoint
    count - the number of data to be written in bytes
   Return the number of bytes which will be copied. */
uint32_t USBDwriteEx(uint8_t ep, uint8_t const *buff, uint32_t count) {
  uint16_t configured;

  configured = configuredTxEndPoints & (1 << ep);
  if (configured == 0 || pTxBuffer[ep])
    return 0; /* Previous write was not finished or wrong endpoint. */

  pTxBuffer[ep] = buff;
  txBufferCount[ep] = count;
  USBDcontinueInTransfer(ep);
  return count;
}

/* Read data from the selected endpoint.
    ep          - endpoint number
    buff        - a pointer to data buffer where a received
                  data are to stored to
    buffer_size - the size of the destination buffer in bytes
   Return the number of bytes copied. */
uint16_t USBDread(uint8_t ep, uint8_t *buff, uint16_t buffer_size) {
  uint32_t pma_offset;
  uint16_t copied_data, configured, endp_index;

  /* The "if" anf "else if" statements are false,
     when ep >= USB_EP_MAX_COUNT. */
  configured = configuredRxEndPoints & (1 << ep);
  endp_index = endPointRxIndex[ep];
  if (configured & configuredDblBuffEndPoints) {
    if (_GetENDPOINT(endp_index) & EP_DTOG_RX) {
      pma_offset  = _GetEPDblBuf0Addr(endp_index);
      copied_data = _GetEPDblBuf0Count(endp_index);
    }
    else {
      pma_offset  = _GetEPDblBuf1Addr(endp_index);
      copied_data = _GetEPDblBuf1Count(endp_index);
    }
  }
  else if (configured) {
    pma_offset  = _GetEPRxAddr(endp_index);
    copied_data = _GetEPRxCount(endp_index);
  }
  else
    return 0;

  /* Prevent from a buffer overflow. */
  if (copied_data > buffer_size)
    copied_data = buffer_size;
  if (buff)
    LowLevelRead(pma_offset, buff, copied_data);

  /* Receiver of the isochronous double buffered endpoint is always in
     the valid state. */
  if ((configured & configuredDblBuffEndPoints) == 0)
    _SetEPRxStatus(endp_index, EP_RX_VALID);

  return buff ? copied_data : 0;
}

/* Isochronous endpoint is never in the STALL stauts, hence this
   function works well for isochronous endpoints. */
usb_result_t USBDgetEndPointStatus(uint8_t epaddr, uint16_t *status) {
  uint16_t ep;

  ep = epaddr & ENDP_NUMBER_MASK;
  /* This checking is not necessary.
  if (ep < 1 || ep >= USB_EP_MAX_COUNT)
    return REQUEST_ERROR;
  else
  */
  if ((epaddr & ENDP_DIRECTION_MASK) == ENDP_IN &&
      (configuredTxEndPoints & (1 << ep)) != 0) {
    *status = (_GetEPTxStatus(endPointTxIndex[ep]) == EP_TX_STALL);
    return REQUEST_SUCCESS;
  }
  else if ((epaddr & ENDP_DIRECTION_MASK) == ENDP_OUT &&
           (configuredRxEndPoints & (1 << ep)) != 0) {
    *status = (_GetEPRxStatus(endPointRxIndex[ep]) == EP_RX_STALL);
    return REQUEST_SUCCESS;
  }
  else {
    *status = 0;
    return REQUEST_ERROR;
  }
}

usb_result_t USBDsetEndPointHalt(uint8_t epaddr) {
  uint16_t ep;

  ep = epaddr & ENDP_NUMBER_MASK;
  /* if (ep < 1 || ep >= USB_EP_MAX_COUNT) */
  if (ep == 0)
    return REQUEST_ERROR;
  else if ((epaddr & ENDP_DIRECTION_MASK) == ENDP_IN &&
           (configuredTxEndPoints & (1 << ep)) != 0 &&
           (configuredDblBuffEndPoints & (1 << ep)) == 0)
    _SetEPTxStatus(endPointTxIndex[ep], EP_TX_STALL);
  else if ((epaddr & ENDP_DIRECTION_MASK) == ENDP_OUT &&
           (configuredRxEndPoints & (1 << ep)) != 0 &&
           (configuredDblBuffEndPoints & (1 << ep)) == 0)
    _SetEPTxStatus(endPointRxIndex[ep], EP_RX_STALL);
  else
    return REQUEST_ERROR;
  return REQUEST_SUCCESS;
}

usb_result_t USBDclearEndPointHalt(uint8_t epaddr) {
  uint16_t ep;

  ep = epaddr & ENDP_NUMBER_MASK;
  /* if (ep < 1 || ep >= USB_EP_MAX_COUNT) */
  if (ep == 0)
    return REQUEST_ERROR;
  else if ((epaddr & ENDP_DIRECTION_MASK) == ENDP_IN &&
           (configuredTxEndPoints & (1 << ep)) != 0 &&
           (configuredDblBuffEndPoints & (1 << ep)) == 0) {
    _ClearDTOG_TX(endPointTxIndex[ep]);
    _SetEPTxStatus(endPointTxIndex[ep], EP_TX_NAK);
  }
  else if ((epaddr & ENDP_DIRECTION_MASK) == ENDP_OUT &&
           (configuredRxEndPoints & (1 << ep)) != 0 &&
           (configuredDblBuffEndPoints & (1 << ep)) == 0) {
    _ClearDTOG_RX(endPointRxIndex[ep]);
    _SetEPTxStatus(endPointRxIndex[ep], EP_RX_VALID);
  }
  else
    return REQUEST_ERROR;
  return REQUEST_SUCCESS;
}

/** Default control endpoint shortcuts **/

/* Endpoint 0 is always control endpoint and has index 0. */

uint16_t USBDwrite0(uint8_t const *buffer, uint16_t count) {
  if (buffer)
    LowLevelWrite(_GetEPTxAddr(0), buffer, count);
  else
    count = 0;
  _SetEPTxCount(0, count);
  return count;
}

uint16_t USBDread0(uint8_t *buffer, uint16_t count) {
  uint16_t r_count;

  if (buffer == 0)
    return 0;
  r_count = _GetEPRxCount(0);
  if (r_count > count)
    r_count = count;
  LowLevelRead(_GetEPRxAddr(0), buffer, r_count);
  return r_count;
}

void USBDendPoint0TxVALID(void) {
  _SetEPTxStatus(0, EP_TX_VALID);
}

void USBDendPoint0RxVALID(void) {
  _SetEPRxStatus(0, EP_RX_VALID);
}

void USBDendPoint0TxSTALL(void) {
  _SetEPTxStatus(0, EP_TX_STALL);
}

void USBDendPoint0RxSTALL(void) {
  _SetEPRxStatus(0, EP_RX_STALL);
}
