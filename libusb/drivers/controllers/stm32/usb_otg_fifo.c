#include <usb_def.h>
#include <usb_otg_fifo.h>
#include <usb_otg_regs.h>

/** FIFO API for STM32F105, STM32F107, STM32F207, STM32F217,
    STM32F405, STM32F407, STM32F415 and STM32F417 **/

int FlushTxFifo(int num) {
  USB_OTG_GRSTCTL_TypeDef grstctl;
  int tout;

  grstctl.d32 = 0;
  grstctl.b.txfflsh = 1;
  grstctl.b.txfnum = num;
  P_USB_OTG_GREGS->GRSTCTL = grstctl.d32;
  for (tout = 200000; tout >= 0 && grstctl.b.txfflsh == 1; --tout)
    grstctl.d32 = P_USB_OTG_GREGS->GRSTCTL;
  return tout < 0 ? -1 : 0;
}

int FlushRxFifo() {
  USB_OTG_GRSTCTL_TypeDef grstctl;
  int tout;

  grstctl.d32 = 0;
  grstctl.b.rxfflsh = 1;
  P_USB_OTG_GREGS->GRSTCTL = grstctl.d32;
  for (tout = 200000; tout >= 0 && grstctl.b.rxfflsh == 1; --tout)
    grstctl.d32 = P_USB_OTG_GREGS->GRSTCTL;
  return tout < 0 ? -1 : 0;
}

void WriteFifo8(int num, uint8_t const *src, uint32_t len) {
  uint32_t i, j;
  union {
    uint32_t dw;
    uint8_t  by[4];
  } buff;

  for (i = 0; i < len; i += 4) {
    for (j = 0; j < 4; ++j) {
      if (i + j < len)
        buff.by[j] = src[i + j];
      else
        buff.by[j] = 0;
    }
    P_USB_OTG_FIFO[num][0] = buff.dw;
  }
}

/* Copy 4 * ((len + 3) >> 2) bytes from src to the transmit fifo.
   The src buffer must be 4 byte aligned. */
void WriteFifo32(int num, uint32_t const *src, uint32_t len) {
  uint32_t i;

  for (i = 0; i < len; i += 4, ++src)
    P_USB_OTG_FIFO[num][0] = *src;
}

void ReadFifo8(int num, uint8_t *dst, uint32_t len) {
  uint32_t i, j;
  union {
    uint32_t dw;
    uint8_t  by[4];
  } buff;

  for (i = 0; i < len; i += 4) {
    buff.dw = P_USB_OTG_FIFO[num][0];
    for (j = 0; j < 4; ++j) {
      if (i + j < len)
        dst[i + j] = buff.by[j];
    }
  }
}

/* Copy 4 * ((len + 3) >> 2) bytes from the receive fifo to the dst
   buffer, which must be 4 byte aligned. */
void ReadFifo32(int num, uint32_t *dst, uint32_t len) {
  uint32_t i;

  for (i = 0; i < len; i += 4, ++dst)
    *dst = P_USB_OTG_FIFO[num][0];
}

/* Calculate the amount of free space in TX FIFO. */
uint32_t GetDeviceFreeTxFifoSpace(int ep) {
  USB_OTG_DTXFSTSx_TypeDef dtxfsts;

  dtxfsts.d32 = P_USB_OTG_DINEPS[ep].DTXFSTSx;
  return dtxfsts.b.ineptfsav << 2;
}

/* Calculate the amount of free space in TX FIFO. */
uint32_t GetHostFreeTxFifoSpace(int ch_num) {
  USB_OTG_HCCHARx_TypeDef  hcchar;
  USB_OTG_HNPTXSTS_TypeDef hnptxsts;
  USB_OTG_HPTXSTS_TypeDef  hptxsts;

  hcchar.d32 = P_USB_OTG_HCHNNLS[ch_num].HCCHARx;
  if (hcchar.b.eptyp == CONTROL_TRANSFER ||
      hcchar.b.eptyp == BULK_TRANSFER) { /* non-periodic */
    hnptxsts.d32 = P_USB_OTG_GREGS->HNPTXSTS;
    return hnptxsts.b.nptxqsav ? hnptxsts.b.nptxfsav << 2 : 0;
  }
  else { /* periodic */
    hptxsts.d32 = P_USB_OTG_HREGS->HPTXSTS;
    return hptxsts.b.ptxqsav ? hptxsts.b.ptxfsav << 2 : 0;
  }
}
