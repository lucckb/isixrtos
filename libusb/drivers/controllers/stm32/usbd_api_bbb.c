#include <string.h>
#include <usb_otg_fifo.h>
#include <usb_otg_regs.h>
#include <usbd_api.h>

/* NOTE: Isochronous endpoints limitations: only full speed is
   supported, for IN transfers bInterval must be 1. */

/** USB device API for STM32F105, STM32F107,
    STM32F2xx and STM32F4xx. **/

/** General API **/

/* Set the device address. */
void USBDsetDeviceAddress(set_address_t token, uint8_t address) {
  if (token == SET_ADDRESS_RESET || token == SET_ADDRESS_REQUEST) {
    USB_OTG_DCFG_TypeDef dcfg;

    dcfg.d32 = P_USB_OTG_DREGS->DCFG;
    dcfg.b.dad = address;
    P_USB_OTG_DREGS->DCFG = dcfg.d32;
  }
}

/** Low level USB buffer handling **/

/* Reserve buffer equal to the RX FIFO. This could be optimized. */
#define RX_FIFO_DWORDS_SIZE  256

/* fifoTop and rxBufferTop are in 32-bit dwords.
   rxBuffSize0, rxBufferCount and txBufferCount are in bytes. */
static uint32_t      globalRxBuffer[RX_FIFO_DWORDS_SIZE];
static uint32_t      *pRxBuffer[CONST_EP_MAX_COUNT];
static uint8_t const *pTxBuffer[CONST_EP_MAX_COUNT];
static uint32_t      fifoTop = 0;
static uint32_t      rxBufferTop = 0;
static uint32_t      rxBuffSize0;
static uint32_t      rxBufferCount[CONST_EP_MAX_COUNT];
static uint32_t      txBufferCount[CONST_EP_MAX_COUNT];

/** Library private API **/

/* Received data are copied first to globalRxBuffer. This buffer works
   in 4 bytes words, which are aligned to 4 bytes boundary. Data are
   copied from the receive buffer to an user buffer by the USBread
   function. */
void USBDdataReceived(uint8_t ep, uint32_t bytes) {
  rxBufferCount[ep] = bytes;
  ReadFifo32(ep, pRxBuffer[ep], bytes);
}

/* Transmitted data are copied directly from an user buffer. The size
   of this buffer is not guaranted to be multiply of 4 bytes. This
   buffer does not need be aligned to 4 bytes boundary. This function
   is called when there is free space in TX FIFO. */
void USBDcontinueInTransfer(uint8_t ep) {
  uint32_t count;

  /* Calculate the amount of free space in TX FIFO. */
  count = GetDeviceFreeTxFifoSpace(ep);

  /* Write data to FIFO. */
  if (count > txBufferCount[ep])
    count = txBufferCount[ep];
  WriteFifo8(ep, pTxBuffer[ep], count);

  /* Move data pointer and check if all data sent. */
  txBufferCount[ep] -= count;
  pTxBuffer[ep]     += count;
  if (txBufferCount[ep] == 0) {
    P_USB_OTG_DREGS->DIEPEMPMSK &= ~(1 << ep);
    pTxBuffer[ep] = 0;
  }
  else {
    P_USB_OTG_DREGS->DIEPEMPMSK |= 1 << ep;
  }
}

/** Endpoint configuration API **/

/* Disable all non-control endpoints. */
void USBDdisableAllNonControlEndPoints() {
  USB_OTG_DAINTMSK_TypeDef daintmsk;
  USB_OTG_DEPCTLx_TypeDef  depctl;
  unsigned i;

  /* Disable non-control endpoint interrupts. */
  daintmsk.d32 = P_USB_OTG_DREGS->DAINTMSK;
  daintmsk.b.oepm &= 1;
  daintmsk.b.iepm &= 1;
  P_USB_OTG_DREGS->DAINTMSK = daintmsk.d32;
  P_USB_OTG_DREGS->DIEPEMPMSK &= 1;

  /* Disable non-control endpoints and clear pending interrupts.*/
  depctl.d32 = 0;
  depctl.b.epdis = 1;
  for (i = 1; i < EP_MAX_COUNT; ++i) {
    P_USB_OTG_DOUTEPS[i].DOEPCTLx = depctl.d32;
    P_USB_OTG_DINEPS[i].DIEPCTLx = depctl.d32;
    P_USB_OTG_DOUTEPS[i].DOEPINTx = 0xff;
    P_USB_OTG_DINEPS[i].DIEPINTx = 0xff;
  }

  fifoTop = RX_FIFO_DWORDS_SIZE;
  rxBufferTop = rxBuffSize0 >> 2;
  for (i = 1; i < CONST_EP_MAX_COUNT; ++i) {
    pRxBuffer[i] = 0;
    pTxBuffer[i] = 0;
    rxBufferCount[i] = txBufferCount[i] = 0;
  }

  /* Default control end point 0 is also affected. This is acceptable,
     if the function is called to answer SET_CONFIGURATION request. */
  FlushTxFifo(ALL_TX_FIFOS);
  FlushRxFifo(ALL_RX_FIFOS);
}

usb_result_t USBDendPointConfigure(uint8_t endPoint,
                                   usb_transfer_t type,
                                   uint16_t rxBuffSize,
                                   uint16_t txBuffSize) {
  return USBDendPointConfigureEx(endPoint, type, rxBuffSize,
                                 txBuffSize, txBuffSize);
}

/* Initialize a given endpoint.
   Control endpoint 0 must be configured first.
    endPoint   - endpoint number
    type       - endpoint transfer type
    rxBuffSize - receive packet size
    txBuffSize - transmit packet size
    txFifoSize - transmit fifo size, must be >= txBuffSize
   Return REQUEST_SUCCESS if success and REQUEST_ERROR otherwise. */
usb_result_t USBDendPointConfigureEx(uint8_t endPoint,
                                     usb_transfer_t type,
                                     uint16_t rxBuffSize,
                                     uint16_t txBuffSize,
                                     uint16_t txFifoSize) {
  USB_OTG_FIFOSIZE_TypeDef txfifosize;
  USB_OTG_DEPCTLx_TypeDef  depctl;
  USB_OTG_DAINTMSK_TypeDef daintmsk;
  USB_OTG_DEPTSIZx_TypeDef deptsiz;

  if (txFifoSize < txBuffSize)
    return REQUEST_ERROR;

  if (type == CONTROL_TRANSFER && endPoint == 0 &&
      rxBuffSize == txBuffSize && (rxBuffSize == 8 ||
      rxBuffSize == 16 || rxBuffSize == 32 || rxBuffSize == 64)) {
    /* Configure RX FIFO. */
    fifoTop = RX_FIFO_DWORDS_SIZE;
    P_USB_OTG_GREGS->GRXFSIZ = fifoTop;

    /* Configure RX buffer. */
    pRxBuffer[0] = globalRxBuffer;
    rxBufferCount[0] = 0;
    rxBufferTop = rxBuffSize >> 2;
    rxBuffSize0 = rxBuffSize;

    /* Reset TX buffer. */
    pTxBuffer[0] = 0;
    txBufferCount[0] = 0;

    /* Configure TX FIFO. */
    txfifosize.b.startaddr = fifoTop;
    txfifosize.b.depth = (txFifoSize + 3) >> 2;
    fifoTop += txfifosize.b.depth;
    P_USB_OTG_GREGS->DIEPTXF0 = txfifosize.d32;

    /* Configure endpoint registers. */
    deptsiz.d32 = 0;
    deptsiz.b.xfrsiz = rxBuffSize;
    deptsiz.b.pktcnt = 1;
    P_USB_OTG_DINEPS[0].DIEPTSIZx = deptsiz.d32;
    deptsiz.d32 = 0;
    deptsiz.b.xfrsiz = rxBuffSize;
    deptsiz.b.pktcnt = 1;
    deptsiz.b.stupcnt_mcnt_rxpid = 1; /* only one back-to-back setup */
    P_USB_OTG_DOUTEPS[0].DOEPTSIZx = deptsiz.d32;
    depctl.d32 = 0;
    switch (txBuffSize) {
      case 8:
        depctl.b.mpsiz = MPSIZ_8;
        break;
      case 16:
        depctl.b.mpsiz = MPSIZ_16;
        break;
      case 32:
        depctl.b.mpsiz = MPSIZ_32;
        break;
      case 64:
        depctl.b.mpsiz = MPSIZ_64;
        break;
    }
    depctl.b.txfnum = 0;
    /* Do not enable TX now, i.e., do not set the depctl.b.epena and
       depctl.b.cnak bits.
       All other values for the DIEPCTL0 register are hardcored. */
    P_USB_OTG_DINEPS[0].DIEPCTLx = depctl.d32;
    /* The most values for the DOEPCTL0 register are hardcored or
       taken from DIEPCTL0. Enable RX. */
    depctl.d32 = 0;
    depctl.b.cnak = 1;
    depctl.b.epena = 1;
    P_USB_OTG_DOUTEPS[0].DOEPCTLx = depctl.d32;

    /* Enable endpoint interrupt. */
    daintmsk.d32 = 0;
    daintmsk.b.iepm = 1;
    daintmsk.b.oepm = 1;
    /* Disable interrupt for others endpoints. */
    P_USB_OTG_DREGS->DAINTMSK = daintmsk.d32;
  }
  else if (type != CONTROL_TRANSFER &&
           endPoint > 0 && endPoint < EP_MAX_COUNT) {
    if (rxBuffSize > 0) {
      /* Configure RX buffer. */
      pRxBuffer[endPoint] = globalRxBuffer + rxBufferTop;
      rxBufferCount[endPoint] = 0;
      rxBufferTop += (rxBuffSize + 3) >> 2;

      /* Configure endpoint registers. */
      deptsiz.d32 = 0;
      deptsiz.b.xfrsiz = rxBuffSize;
      deptsiz.b.pktcnt = 1;
      P_USB_OTG_DOUTEPS[endPoint].DOEPTSIZx = deptsiz.d32;
      depctl.d32 = 0;
      depctl.b.mpsiz = rxBuffSize;
      depctl.b.usbaep = 1;
      depctl.b.eptyp = type;
      depctl.b.cnak = 1;
      depctl.b.sd0pid_sevnfrm = 1;
      depctl.b.epena = 1;
      P_USB_OTG_DOUTEPS[endPoint].DOEPCTLx = depctl.d32;

      /* Enable endpoint interrupt. */
      daintmsk.d32 = P_USB_OTG_DREGS->DAINTMSK;
      daintmsk.b.oepm |= 1 << endPoint;
      P_USB_OTG_DREGS->DAINTMSK = daintmsk.d32;
    }
    if (txBuffSize > 0) {
      /* Configure TX FIFO. */
      txfifosize.b.startaddr = fifoTop;
      txfifosize.b.depth = (txFifoSize + 3) >> 2;
      fifoTop += txfifosize.b.depth;
      P_USB_OTG_GREGS->DIEPTXFx[endPoint - 1] = txfifosize.d32;

      /* Configure endpoint registers. */
      deptsiz.d32 = 0;
      deptsiz.b.pktcnt = 1;
      if (type != ISOCHRONOUS_TRANSFER)
        deptsiz.b.xfrsiz = txBuffSize;
      else
        deptsiz.b.stupcnt_mcnt_rxpid = 1;
      P_USB_OTG_DINEPS[endPoint].DIEPTSIZx = deptsiz.d32;
      depctl.d32 = 0;
      depctl.b.mpsiz = txBuffSize;
      depctl.b.usbaep = 1;
      depctl.b.eptyp = type;
      depctl.b.txfnum = endPoint;
      if (type != ISOCHRONOUS_TRANSFER)
        depctl.b.snak = 1;
      depctl.b.sd0pid_sevnfrm = 1;
      depctl.b.epena = 1;
      P_USB_OTG_DINEPS[endPoint].DIEPCTLx = depctl.d32;

      /* Enable endpoint interrupt. */
      daintmsk.d32 = P_USB_OTG_DREGS->DAINTMSK;
      daintmsk.b.iepm |= 1 << endPoint;
      P_USB_OTG_DREGS->DAINTMSK = daintmsk.d32;
    }
  }
  else
    return REQUEST_ERROR;
  if (fifoTop <= FIFO_DWORDS_SIZE &&
      rxBufferTop <= RX_FIFO_DWORDS_SIZE)
    return REQUEST_SUCCESS;
  else
    return REQUEST_ERROR;
}

/** Endpoint API **/

/* Write data to the selected endpoint. Only one packet can be sent.
   User buffer can be reused after function return - copy semanatics.
    ep    - endpoint number
    buff  - a pointer to the buffer containing data to
            be written to the endpoint
    count - the number of data to be written in bytes
   Return the number of bytes which will be copied. */
uint16_t USBDwrite(uint8_t ep, uint8_t const *buff, uint16_t count) {
  USB_OTG_DEPTSIZx_TypeDef dieptsiz;
  USB_OTG_DEPCTLx_TypeDef  diepctl;
  USB_OTG_DSTS_TypeDef     dsts;

  if (ep >= USB_EP_MAX_COUNT || pTxBuffer[ep])
    return 0; /* Previous write was not finished or wrong endpoint. */

  if (!buff)
    count = 0;

  diepctl.d32 = P_USB_OTG_DINEPS[ep].DIEPCTLx;
  if (count > diepctl.b.mpsiz)
    count = diepctl.b.mpsiz;

  pTxBuffer[ep] = buff;
  txBufferCount[ep] = count;

  dieptsiz.d32 = P_USB_OTG_DINEPS[ep].DIEPTSIZx;
  dieptsiz.b.xfrsiz = count;
  dieptsiz.b.pktcnt = 1;
  dieptsiz.b.stupcnt_mcnt_rxpid = 1; /* for isochronous only */
  P_USB_OTG_DINEPS[ep].DIEPTSIZx = dieptsiz.d32;

  diepctl.b.cnak = 1;
  diepctl.b.epena = 1;
  if (diepctl.b.eptyp == ISOCHRONOUS_TRANSFER) {
    dsts.d32 = P_USB_OTG_DREGS->DSTS;
    /* The packet will be send in the next frame. */
    if (dsts.b.fnsof & 1)
      diepctl.b.sd0pid_sevnfrm = 1;
    else
      diepctl.b.sd1pid_soddfrm = 1;
  }
  P_USB_OTG_DINEPS[ep].DIEPCTLx = diepctl.d32;

  if (count > 0)
    /* Write directly to FIFO ... */
    USBDcontinueInTransfer(ep);
    /* ... or enable interrupt when TX FIFO will become empty.
    P_USB_OTG_DREGS->DIEPEMPMSK |= (1 << ep);
    pTxBuffer[0] is zeroed in USBDcontinueInTransfer. */
  else {
    /* pTxBuffer[0] must be zeroed here. */
    pTxBuffer[ep] = 0;
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
  USB_OTG_DEPTSIZx_TypeDef dieptsiz;
  USB_OTG_DEPCTLx_TypeDef  diepctl;
  uint32_t                 pktsize;

  if (ep >= USB_EP_MAX_COUNT || pTxBuffer[ep])
    return 0; /* Previous write was not finished or wrong endpoint. */

  if (!buff)
    count = 0;

  pTxBuffer[ep] = buff;
  txBufferCount[ep] = count;

  dieptsiz.d32 = P_USB_OTG_DINEPS[ep].DIEPTSIZx;
  diepctl.d32 = P_USB_OTG_DINEPS[ep].DIEPCTLx;
  pktsize = diepctl.b.mpsiz;
  dieptsiz.b.xfrsiz = count;
  dieptsiz.b.pktcnt = (count + pktsize - 1) / pktsize;
  diepctl.b.cnak = 1;
  diepctl.b.epena = 1;
  P_USB_OTG_DINEPS[ep].DIEPTSIZx = dieptsiz.d32;
  P_USB_OTG_DINEPS[ep].DIEPCTLx = diepctl.d32;

  if (count > 0) {
    /* Enable interrupt when TX FIFO will become empty.
       pTxBuffer[0] is zeroed in USBDcontinueInTransfer. */
    P_USB_OTG_DREGS->DIEPEMPMSK |= (1 << ep);
  }
  else {
    /* pTxBuffer[0] must be zeroed here. */
    pTxBuffer[ep] = 0;
  }

  return count;
}

/* Read data from the selected endpoint.
    ep    - endpoint number
    buff  - a pointer to data buffer where received data should
            be stored
    count - the size of the destination buffer in bytes
   Return the number of bytes copied. */
uint16_t USBDread(uint8_t ep, uint8_t *buff, uint16_t count) {
  USB_OTG_DEPTSIZx_TypeDef doeptsiz;
  USB_OTG_DEPCTLx_TypeDef  doepctl;

  if (count > rxBufferCount[ep])
    count = rxBufferCount[ep];
  memcpy(buff, pRxBuffer[ep], count);
  rxBufferCount[ep] = 0;

  /* Reenable endpoint. */
  doeptsiz.d32 = P_USB_OTG_DOUTEPS[ep].DOEPTSIZx;
  doepctl.d32 = P_USB_OTG_DOUTEPS[ep].DOEPCTLx;
  doeptsiz.b.xfrsiz = doepctl.b.mpsiz;
  doeptsiz.b.pktcnt = 1;
  doepctl.b.cnak = 1;
  P_USB_OTG_DOUTEPS[ep].DOEPTSIZx = doeptsiz.d32;
  P_USB_OTG_DOUTEPS[ep].DOEPCTLx = doepctl.d32;

  return count;
}

/* Isochronous endpoint is never in the STALL stauts, hence this
   function works well for isochronous endpoints. */
usb_result_t USBDgetEndPointStatus(uint8_t epaddr, uint16_t *status) {
  USB_OTG_DEPCTLx_TypeDef depctl;
  uint8_t ep;

  ep = epaddr & ENDP_NUMBER_MASK;
  if (ep < 1 || ep >= EP_MAX_COUNT) {
    status = 0;
    return REQUEST_ERROR;
  }

  if ((epaddr & ENDP_DIRECTION_MASK) == ENDP_IN)
    depctl.d32 = P_USB_OTG_DINEPS[ep].DIEPCTLx;
  else
    depctl.d32 = P_USB_OTG_DOUTEPS[ep].DOEPCTLx;

  if (depctl.b.epena) {
    *status = depctl.b.stall;
    return REQUEST_SUCCESS;
  }
  else {
    *status = 0;
    return REQUEST_ERROR;
  }
}

usb_result_t USBDsetEndPointHalt(uint8_t epaddr) {
  USB_OTG_DEPCTLx_TypeDef depctl;
  uint8_t ep;

  ep = epaddr & ENDP_NUMBER_MASK;
  if (ep < 1 || ep >= EP_MAX_COUNT)
    return REQUEST_ERROR;

  if ((epaddr & ENDP_DIRECTION_MASK) == ENDP_IN)
    depctl.d32 = P_USB_OTG_DINEPS[ep].DIEPCTLx;
  else
    depctl.d32 = P_USB_OTG_DOUTEPS[ep].DOEPCTLx;

  /* It seems that clear endpoint halt must not depend on the epena bit. */
  /* if (depctl.b.epena == 0 || depctl.b.eptyp == ISOCHRONOUS_TRANSFER) */
  if (depctl.b.eptyp == ISOCHRONOUS_TRANSFER)
    return REQUEST_ERROR;

  depctl.b.stall = 1;
  if ((epaddr & ENDP_DIRECTION_MASK) == ENDP_IN)
    P_USB_OTG_DINEPS[ep].DIEPCTLx = depctl.d32;
  else
    P_USB_OTG_DOUTEPS[ep].DOEPCTLx = depctl.d32;

  return REQUEST_SUCCESS;
}

usb_result_t USBDclearEndPointHalt(uint8_t epaddr) {
  USB_OTG_DEPCTLx_TypeDef depctl;
  uint8_t ep;

  ep = epaddr & ENDP_NUMBER_MASK;
  if (ep < 1 || ep >= EP_MAX_COUNT)
    return REQUEST_ERROR;

  if ((epaddr & ENDP_DIRECTION_MASK) == ENDP_IN)
    depctl.d32 = P_USB_OTG_DINEPS[ep].DIEPCTLx;
  else
    depctl.d32 = P_USB_OTG_DOUTEPS[ep].DOEPCTLx;

  /* It seems that clear endpoint halt must not depend on the epena bit. */
  /* if (depctl.b.epena == 0 || depctl.b.eptyp == ISOCHRONOUS_TRANSFER) */
  if (depctl.b.eptyp == ISOCHRONOUS_TRANSFER)
    return REQUEST_ERROR;

  depctl.b.stall = 0;
  depctl.b.sd0pid_sevnfrm = 1;
  if ((epaddr & ENDP_DIRECTION_MASK) == ENDP_IN)
    P_USB_OTG_DINEPS[ep].DIEPCTLx = depctl.d32;
  else
    P_USB_OTG_DOUTEPS[ep].DOEPCTLx = depctl.d32;

  return REQUEST_SUCCESS;
}

/** Default control endpoint shortcuts **/

uint16_t USBDwrite0(uint8_t const *buffer, uint16_t count) {
  USB_OTG_FIFOSIZE_TypeDef txFifoSize;
  USB_OTG_DEPTSIZx_TypeDef dieptsiz;

  if (pTxBuffer[0])
    return 0;  /* Previous write was not finished. */

  if (!buffer)
    count = 0;

  txFifoSize.d32 = P_USB_OTG_GREGS->DIEPTXF0;
  if (count > (txFifoSize.b.depth << 2))
    count = txFifoSize.b.depth << 2;
  pTxBuffer[0] = buffer;
  txBufferCount[0] = count;

  dieptsiz.d32 = P_USB_OTG_DINEPS[0].DIEPTSIZx;
  dieptsiz.b.xfrsiz = count;
  dieptsiz.b.pktcnt = 1;
  P_USB_OTG_DINEPS[0].DIEPTSIZx = dieptsiz.d32;

  if (count > 0) {
    /* Enable interrupt when TX FIFO will become empty. Writting
       directly to the FIFO does not work, because the DIEPCTL0
       register is not set yet. It is set later in usb_core.c, when
       the function USBendPoint0TxVALID is called. pTxBuffer[0] is
       zeroed in USBDcontinueInTransfer. */
    P_USB_OTG_DREGS->DIEPEMPMSK |= 1;
  }
  else {
    /* pTxBuffer[0] must be zeroed here. */
    pTxBuffer[0] = 0;
  }

  return count;
}

uint16_t USBDread0(uint8_t *buffer, uint16_t count) {
  USB_OTG_DEPTSIZx_TypeDef doeptsiz;

  if (count > rxBufferCount[0])
    count = rxBufferCount[0];
  memcpy(buffer, pRxBuffer[0], count);
  rxBufferCount[0] = 0;

  /* Reenable endpoint. */
  doeptsiz.d32 = P_USB_OTG_DOUTEPS[0].DOEPTSIZx;
  doeptsiz.b.xfrsiz = rxBuffSize0;
  doeptsiz.b.pktcnt = 1;
  P_USB_OTG_DOUTEPS[0].DOEPTSIZx = doeptsiz.d32;

  return count;
}

void USBDendPoint0TxVALID(void) {
  USB_OTG_DEPCTLx_TypeDef depctl;

  depctl.d32 = P_USB_OTG_DINEPS[0].DIEPCTLx;
  depctl.b.cnak = 1;
  depctl.b.epena = 1;
  P_USB_OTG_DINEPS[0].DIEPCTLx = depctl.d32;
}

void USBDendPoint0RxVALID(void) {
  USB_OTG_DEPCTLx_TypeDef depctl;

  depctl.d32 = P_USB_OTG_DOUTEPS[0].DOEPCTLx;
  depctl.b.cnak = 1;
  depctl.b.epena = 1;
  P_USB_OTG_DOUTEPS[0].DOEPCTLx = depctl.d32;
}

void USBDendPoint0TxSTALL(void) {
  USB_OTG_DEPCTLx_TypeDef depctl;

  depctl.d32 = P_USB_OTG_DINEPS[0].DIEPCTLx;
  depctl.b.stall = 1;
  P_USB_OTG_DINEPS[0].DIEPCTLx = depctl.d32;
}

void USBDendPoint0RxSTALL(void) {
  USB_OTG_DEPCTLx_TypeDef depctl;

  depctl.d32 = P_USB_OTG_DOUTEPS[0].DOEPCTLx;
  depctl.b.stall = 1;
  P_USB_OTG_DOUTEPS[0].DOEPCTLx = depctl.d32;
}
