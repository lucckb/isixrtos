#include <usb/drivers/controllers/stm32/usb_otg_fifo.h>
#include <usb/drivers/controllers/stm32/usb_otg_regs.h>
#include <usb/core/usbh_api.h>
#include <usb/core/usbh_error.h>

/** USB host API for STM32F105, STM32F107, STM32F2xx and STM32F4xx **/

/** Internal auxiliary function **/

static void HaltChannel(int ch_num) {
  USB_OTG_HNPTXSTS_TypeDef hnptxsts;
  USB_OTG_HPTXSTS_TypeDef  hptxsts;
  USB_OTG_HCCHARx_TypeDef  hcchar;

  hcchar.d32 = P_USB_OTG_HCHNNLS[ch_num].HCCHARx;
  hcchar.b.chdis = 1;
  if (hcchar.b.eptyp == CONTROL_TRANSFER ||
      hcchar.b.eptyp == BULK_TRANSFER) {
    hnptxsts.d32 = P_USB_OTG_GREGS->HNPTXSTS;
    /* Flush request queue, when it is full. */
    hcchar.b.chena = hnptxsts.b.nptxqsav != 0;
  }
  else {
    hptxsts.d32 = P_USB_OTG_HREGS->HPTXSTS;
    /* Flush request queue, when it is full. */
    hcchar.b.chena = hptxsts.b.ptxqsav != 0;
  }
  P_USB_OTG_HCHNNLS[ch_num].HCCHARx = hcchar.d32;
}

/** Host channels API **/

unsigned USBHgetChannelCount() {
  return _USB_CHNNL_MAX_COUNT;
}

void USBHstopAllChannels() {
  int i;

  P_USB_OTG_HREGS->HAINTMSK = 0;
  /* Fifos must be flushed before USBHhaltChannel. */
  FlushTxFifo(ALL_TX_FIFOS);
  FlushRxFifo(ALL_RX_FIFOS);
  for (i = 0; i < (int)_USB_CHNNL_MAX_COUNT; ++i) {
    HaltChannel(i);
    P_USB_OTG_HCHNNLS[i].HCINTMSKx = 0;
    P_USB_OTG_HCHNNLS[i].HCINTx = 0xffffffff;
  }
}

void USBHinitChannel(int ch_num, uint8_t dev_addr, uint8_t ep_addr,
                     usb_speed_t dev_speed, usb_transfer_t ep_type,
                     uint16_t max_packet) {
  USB_OTG_HCINTMSKx_TypeDef hcintmsk;
  USB_OTG_HCCHARx_TypeDef   hcchar;

  /* Clear old interrupt conditions for this host channel. */
  P_USB_OTG_HCHNNLS[ch_num].HCINTx = 0xffffffff;

  /* Enable channel interrupts required for a transfer.
     ACK for IN transactions is generated by the host. ACK for OUT
     transactions is used in the data PID toggle algorithm. */
  hcintmsk.d32 = 0;
  switch (ep_type) {
    case CONTROL_TRANSFER:
    case BULK_TRANSFER:
      hcintmsk.b.xfrcm = 1;
      hcintmsk.b.stallm = 1;
      hcintmsk.b.nakm = 1;
      hcintmsk.b.txerrm = 1;
      hcintmsk.b.dterrm = 1;
      if ((ep_addr & ENDP_DIRECTION_MASK) == ENDP_IN)
        hcintmsk.b.bberrm = 1;
      else
        hcintmsk.b.ackm = 1;
      break;
    case INTERRUPT_TRANSFER:
      hcintmsk.b.xfrcm = 1;
      hcintmsk.b.stallm = 1;
      hcintmsk.b.nakm = 1;
      hcintmsk.b.txerrm = 1;
      hcintmsk.b.frmorm = 1;
      hcintmsk.b.dterrm = 1;
      if ((ep_addr & ENDP_DIRECTION_MASK) == ENDP_IN)
        hcintmsk.b.bberrm = 1;
      else
        hcintmsk.b.ackm = 1;
      break;
    case ISOCHRONOUS_TRANSFER:
      hcintmsk.b.xfrcm = 1;
      hcintmsk.b.frmorm = 1;
      if ((ep_addr & ENDP_DIRECTION_MASK) == ENDP_IN) {
        hcintmsk.b.txerrm = 1;
        hcintmsk.b.bberrm = 1;
      }
      break;
  }
  P_USB_OTG_HCHNNLS[ch_num].HCINTMSKx = hcintmsk.d32;

  /* Enable the top level host channel interrupt. */
  P_USB_OTG_HREGS->HAINTMSK |= 1 << ch_num;

  /* Program channel characteristic. */
  hcchar.d32 = 0;
  hcchar.b.dad = dev_addr;
  hcchar.b.mcnt = 1;
  hcchar.b.eptyp = ep_type;
  hcchar.b.lsdev = dev_speed == LOW_SPEED;
  hcchar.b.epdir = (ep_addr & ENDP_DIRECTION_MASK) == ENDP_IN;
  hcchar.b.epnum = ep_addr & ENDP_NUMBER_MASK;
  hcchar.b.mpsiz = max_packet;
  if (ep_type == INTERRUPT_TRANSFER)
    hcchar.b.oddfrm = 1;
  P_USB_OTG_HCHNNLS[ch_num].HCCHARx = hcchar.d32;
}

int USBHprepareChannel(int ch_num, uint32_t xfer_len,
                       usb_pid_t data_pid) {
  USB_OTG_HCCHARx_TypeDef hcchar;
  USB_OTG_HCTSIZx_TypeDef hctsiz;
  USB_OTG_HFNUM_TypeDef   hfnum;
  uint32_t max_packet, num_packets;

  hcchar.d32 = P_USB_OTG_HCHNNLS[ch_num].HCCHARx;

  /* Compute the expected number of packets associated
     to the transfer. */
  if (xfer_len > 0) {
    max_packet = hcchar.b.mpsiz;
    num_packets = (xfer_len + max_packet - 1) / max_packet;
  }
  else {
    num_packets = 1;
  }

  if (num_packets > USB_MAX_HOST_PACKET_COUNT)
    return USBHLIB_ERROR_IO;

  hctsiz.d32 = 0;
  hctsiz.b.xfrsiz = xfer_len;
  hctsiz.b.pktcnt = num_packets;
  hctsiz.b.pid = data_pid >> 2;
  P_USB_OTG_HCHNNLS[ch_num].HCTSIZx = hctsiz.d32;

  hfnum.d32 = P_USB_OTG_HREGS->HFNUM;
  hcchar.b.oddfrm = !(hfnum.b.frnum & 1);
  hcchar.b.chena = 1;
  hcchar.b.chdis = 0;
  P_USB_OTG_HCHNNLS[ch_num].HCCHARx = hcchar.d32;

  return USBHLIB_SUCCESS;
}

void USBHhaltChannel(int ch_num) {
  USB_OTG_HCINTMSKx_TypeDef hcintmsk;

  /* Channel is not disabled immediately. Enable an interrupt when
     the channel becomes effectively disabled. */
  hcintmsk.d32 = P_USB_OTG_HCHNNLS[ch_num].HCINTMSKx;
  hcintmsk.b.chhm = 1;
  P_USB_OTG_HCHNNLS[ch_num].HCINTMSKx = hcintmsk.d32;

  HaltChannel(ch_num);
}

/** Miscellaneous API **/

uint16_t USBHgetCurrentFrame() {
  USB_OTG_HFNUM_TypeDef hfnum;

  hfnum.d32 = P_USB_OTG_HREGS->HFNUM;
  return hfnum.b.frnum;
}

/* One LS frame takes 6000 clocks.
   One FS frame takes 48000 clocks.
   One HS microframe takes 7500 clocks. */
uint16_t USBHgetFrameClocksRemaining() {
  USB_OTG_HFNUM_TypeDef hfnum;

  hfnum.d32 = P_USB_OTG_HREGS->HFNUM;
  return hfnum.b.ftrem;
}
