#include <auxiliary.h>
#include <limits.h>
#include <stdlib.h>
#include <usb_otg_fifo.h>
#include <usbh_api.h>
#include <usbh_error.h>
#include <usbh_io.h>

typedef struct {
  uint8_t                   used;
  uint8_t                   dev_addr;
  uint8_t                   ep_addr;
  usb_speed_t               dev_speed;
  usb_transfer_t            ep_type;
  uint16_t                  max_packet;
  usbh_transaction_result_t tr_result;
  usb_pid_t                 pid;
  uint8_t                   *buffer;
  uint32_t                  length;
  uint32_t                  transfered;
} usbh_channel_t;

typedef struct {
  int            ch_count;
  usbh_channel_t *ch;
} usbh_io_state_t;

static usbh_io_state_t Host = {0, NULL};

/** USB host channel API **/

/* This function is called only once at configuration phase.
   Hence allocated memory need not be released. */
int USBHchannelsConfigure() {
  unsigned i, ch_count;

  if (Host.ch == NULL) {
    ch_count = USBHgetChannelCount();
    /* At least two channels are required for default control pipe. */
    if (ch_count < 2 || ch_count > INT_MAX)
      return USBHLIB_ERROR_INVALID_PARAM;
    Host.ch = calloc(ch_count, sizeof(usbh_channel_t));
    if (Host.ch == NULL)
      return USBHLIB_ERROR_NO_MEM;
    /* Host.ch_count is intentionally initialized just here. */
    Host.ch_count = (int)ch_count;
  }

  for (i = 0; i < Host.ch_count; ++i)
    Host.ch[i].used = 0;

  return USBHLIB_SUCCESS;
}

static int USBHgetFreeChannel(void) {
  int ch_num;

  for (ch_num = 0; ch_num < Host.ch_count; ++ch_num)
    if (!Host.ch[ch_num].used)
      return ch_num;
  return -1;
}

int USBHallocChannel() {
  int ch_num;

  ch_num = USBHgetFreeChannel();
  if (ch_num >= 0)
    Host.ch[ch_num].used = 1;
  return ch_num;
}

void USBHfreeChannel(int ch_num) {
  if (ch_num >= 0 && ch_num < Host.ch_count)
    Host.ch[ch_num].used = 0;
}

void USBHopenChannel(int ch_num, uint8_t dev_addr, uint8_t ep_addr,
                     usb_speed_t dev_speed, usb_transfer_t ep_type,
                     uint16_t max_packet) {
  Host.ch[ch_num].dev_addr = dev_addr;
  Host.ch[ch_num].ep_addr = ep_addr;
  Host.ch[ch_num].dev_speed = dev_speed;
  Host.ch[ch_num].ep_type = ep_type;
  Host.ch[ch_num].max_packet = max_packet;
  Host.ch[ch_num].pid = PID_DATA0;
  Host.ch[ch_num].tr_result = TR_UNDEF;
  Host.ch[ch_num].buffer = 0;
  Host.ch[ch_num].length = 0;
  Host.ch[ch_num].transfered = 0;

  USBHinitChannel(ch_num,
                  Host.ch[ch_num].dev_addr,
                  Host.ch[ch_num].ep_addr,
                  Host.ch[ch_num].dev_speed,
                  Host.ch[ch_num].ep_type,
                  Host.ch[ch_num].max_packet);
}

void USBHmodifyChannel(int ch_num, uint8_t dev_addr,
                       uint16_t max_packet) {
  if (dev_addr != 0)
    Host.ch[ch_num].dev_addr = dev_addr;
  if (Host.ch[ch_num].max_packet != max_packet && max_packet != 0)
    Host.ch[ch_num].max_packet = max_packet;

  USBHinitChannel(ch_num,
                  Host.ch[ch_num].dev_addr,
                  Host.ch[ch_num].ep_addr,
                  Host.ch[ch_num].dev_speed,
                  Host.ch[ch_num].ep_type,
                  Host.ch[ch_num].max_packet);
}

/** USB host low level interrupt input-output API **/

static void ForwardBufferAndTogglePid(int ch_num, uint32_t length) {
  Host.ch[ch_num].length     -= length;
  Host.ch[ch_num].buffer     += length;
  Host.ch[ch_num].transfered += length;
  if (Host.ch[ch_num].ep_type != ISOCHRONOUS_TRANSFER)
    Host.ch[ch_num].pid = USBtoggleDataPid(Host.ch[ch_num].pid);
}

void USBHpacketReceived(int ch_num, uint32_t length) {
  ReadFifo8(ch_num, Host.ch[ch_num].buffer, length);
  ForwardBufferAndTogglePid(ch_num, length);
}

void USBHpacketSent(int ch_num) {
  uint32_t length;

  length = min(Host.ch[ch_num].length, Host.ch[ch_num].max_packet);
  ForwardBufferAndTogglePid(ch_num, length);
}

void USBHtransferFinished(int ch_num, usbh_transaction_result_t tr) {
  Host.ch[ch_num].tr_result = tr;

  if ((Host.ch[ch_num].ep_addr & ENDP_DIRECTION_MASK) == ENDP_OUT &&
      Host.ch[ch_num].length > 0) {
    /* Not all data are sent. */
    if (USBHrestartTransfer(ch_num) < 0) {
      /* TODO: Activate TX FIFO (half) empty interrupt and send data,
         when there is enough free space in the FIFO. */
    }
  }
}

/** USB host basic application input-output API **/

int USBHstartTransaction(int ch_num, usb_pid_t pid,
                         uint8_t *buffer, uint32_t length) {
  if (length > Host.ch[ch_num].max_packet)
    return USBHLIB_ERROR_INVALID_PARAM;

  Host.ch[ch_num].pid = pid;

  return USBHstartTransfer(ch_num, buffer, length);
}

int USBHstartTransfer(int ch_num, uint8_t *buffer, uint32_t length) {
  Host.ch[ch_num].buffer = buffer;
  Host.ch[ch_num].length = length;
  Host.ch[ch_num].transfered = 0;

  return USBHrestartTransfer(ch_num);
}

int USBHrestartTransfer(int ch_num) {
  uint32_t fifo_space, length;

  if ((Host.ch[ch_num].ep_addr & ENDP_DIRECTION_MASK) == ENDP_OUT) {
    fifo_space = GetHostFreeTxFifoSpace(ch_num);
    if (fifo_space >= Host.ch[ch_num].length)
      length = Host.ch[ch_num].length;
    else if (fifo_space >= Host.ch[ch_num].max_packet)
      /* Host.ch[ch_num].length > Host.ch[ch_num].max_packet */
      length = fifo_space - fifo_space % Host.ch[ch_num].max_packet;
    else
      return USBHLIB_ERROR_IO;
  }
  else {
    length = Host.ch[ch_num].length;
  }

  if (USBHprepareChannel(ch_num, length, Host.ch[ch_num].pid) < 0)
    return USBHLIB_ERROR_IO;

  if ((Host.ch[ch_num].ep_addr & ENDP_DIRECTION_MASK) == ENDP_OUT)
    WriteFifo8(ch_num, Host.ch[ch_num].buffer, length);

  Host.ch[ch_num].tr_result = TR_UNDEF;

  return USBHLIB_SUCCESS;
}

usbh_transaction_result_t USBHgetTransactionResult(int ch_num) {
  return Host.ch[ch_num].tr_result;
}

usbh_transaction_result_t USBHgetTransferResult(int ch_num) {
  return Host.ch[ch_num].tr_result;
}

uint32_t USBHgetTransactionSize(int ch_num) {
  return Host.ch[ch_num].transfered;
}

uint32_t USBHgetTransferSize(int ch_num) {
  return Host.ch[ch_num].transfered;
}

void USBHsetPID(int ch_num, usb_pid_t pid) {
  Host.ch[ch_num].pid = pid;
}
