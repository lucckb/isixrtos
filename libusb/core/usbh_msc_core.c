#include <delay.h>
#include <string.h>
#include <usb_endianness.h>
#include <usbh_core.h>
#include <usbh_error.h>
#include <usbh_interrupt.h>
#include <usbh_io.h>
#include <usbh_msc_core.h>
#include <usbh_std_req.h>

/** MSC BOT protocol implementation **/

#define NAK_TIMEOUT_MS  10000

typedef enum {
  MSC_EXIT = 0, /* MSCisDeviceReady must return 0, if MSC machine is not configured. */
  MSC_INIT,
  MSC_IDLE,
  MSC_CBW_START,
  MSC_CBW_WAIT,
  MSC_CBW_NAKED,
  MSC_DATA_IN_START,
  MSC_DATA_IN_WAIT,
  MSC_DATA_IN_NAKED,
  MSC_DATA_IN_STALLED,
  MSC_DATA_OUT_START,
  MSC_DATA_OUT_WAIT,
  MSC_DATA_OUT_NAKED,
  MSC_DATA_OUT_STALLED,
  MSC_CSW_START,
  MSC_CSW_WAIT,
  MSC_CSW_NAKED,
  MSC_CSW_STALLED,
  MSC_DONE
} usbh_msc_state_t;

typedef struct {
  usbh_msc_state_t    state;
  int                 out_ch_num;
  int                 in_ch_num;
  uint8_t             *buffer;
  uint32_t            length;
  uint32_t            transfered;
  unsigned            nak_delay;
  unsigned            nak_timeout;
  unsigned            csw_attempts;
  uint8_t             status;
  msc_configuration_t cfg;
  msc_bot_cbw_t       cbw;
  msc_bot_csw_t       csw;
} usbh_msc_data_t;

static void FreeChannels(usbh_msc_data_t *md) {
  if (md->out_ch_num >= 0) {
    /* Channel is halted after error in the interrupt routine.
    USBHhaltChannel(md->out_ch_num); */
    USBHfreeChannel(md->out_ch_num);
    md->out_ch_num = -1;
  }
  if (md->in_ch_num >= 0) {
    /* Channel is halted after error in the interrupt routine.
    USBHhaltChannel(md->in_ch_num); */
    USBHfreeChannel(md->in_ch_num);
    md->in_ch_num = -1;
  }
}

static void InitState(usbh_msc_data_t *md) {
  md->out_ch_num = USBHallocChannel();
  md->in_ch_num = USBHallocChannel();
  if (md->out_ch_num >= 0 && md->in_ch_num >= 0) {
    USBHopenChannel(md->out_ch_num, md->cfg.dev_addr,
                    md->cfg.out_ep_addr, md->cfg.speed,
                    BULK_TRANSFER, md->cfg.out_ep_max_packet);
    USBHopenChannel(md->in_ch_num, md->cfg.dev_addr,
                    md->cfg.in_ep_addr, md->cfg.speed,
                    BULK_TRANSFER, md->cfg.in_ep_max_packet);
    md->state = MSC_IDLE;
  }
  else {
    md->state = MSC_EXIT;
  }
}

static void StartState(usbh_msc_data_t *md, int ch_num,
                       uint8_t *buffer, uint32_t length,
                       usbh_msc_state_t next_state) {
  if (USBHstartTransfer(ch_num, buffer, length) == 0) {
    md->nak_timeout = md->cfg.speed == HIGH_SPEED ?
                      NAK_TIMEOUT_MS << 3 : NAK_TIMEOUT_MS;
    md->state = next_state;
  }
  else {
    md->state = MSC_DONE;
  }
  md->status = MSC_BOT_UNDEF_ERROR;
}

static int WaitState(usbh_msc_data_t *md, int ch_num,
                     unsigned interval, usbh_msc_state_t next_state,
                     usbh_msc_state_t nak_state,
                     usbh_msc_state_t stall_state) {
  switch (USBHgetTransferResult(ch_num)) {
    case TR_DONE:
      md->state = next_state;
      return 1;
    case TR_NAK:
      if (md->nak_timeout > 0) {
        md->nak_delay = interval;
        md->state = nak_state;
      }
      else {
        md->status = MSC_BOT_UNDEF_ERROR;
        md->state = MSC_DONE;
      }
      return 0;
    case TR_STALL:
      /* STALL after CBW should result in Recovery Reset. */
      md->status = MSC_BOT_CSW_PHASE_ERROR;
      md->state = stall_state;
      return 0;
    case TR_ERROR:
      md->status = MSC_BOT_UNDEF_ERROR;
      md->state = MSC_DONE;
      return 0;
    default:
      return 0;
  }
}

static void NackedState(usbh_msc_data_t *md, int ch_num,
                        usbh_msc_state_t prev_state) {
  if (md->nak_delay == 0) {
    if (USBHrestartTransfer(ch_num) == 0) {
      md->state = prev_state;
    }
    else {
      md->status = MSC_BOT_UNDEF_ERROR;
      md->state = MSC_DONE;
    }
  }
}

static void StalledState(usbh_msc_data_t *md, uint8_t ep_addr) {
  int res;

  res = USBHclearEndpointHalt(0, ep_addr);
  if (res == USBHLIB_SUCCESS) {
    md->state = MSC_CSW_START;
  }
  else if (res != USBHLIB_IN_PROGRESS) {
    md->status = MSC_BOT_UNDEF_ERROR;
    md->state = MSC_DONE;
  }
}

static int IsCSWvalidAndMeaningful(usbh_msc_data_t const *md) {
  return USBHgetTransferSize(md->in_ch_num) == sizeof(msc_bot_csw_t) &&
         md->csw.dCSWSignature == MSC_BOT_CSW_SIGNATURE &&
         md->csw.dCSWTag == md->cbw.dCBWTag &&
         (md->csw.bCSWStatus == MSC_BOT_CSW_PHASE_ERROR ||
          (md->transfered == md->length - md->csw.dCSWDataResidue &&
           (md->csw.bCSWStatus == MSC_BOT_CSW_COMMAND_PASSED ||
            md->csw.bCSWStatus == MSC_BOT_CSW_COMMAND_FAILED)));
}

/* The return value is non-zero, if the machine finished its work.
   The return value is zero, if the machine still works. */
static int MSCstateMachine(void *p) {
  usbh_msc_data_t *md = p;

  switch (md->state) {
    case MSC_INIT:
      InitState(md);
      break;
    case MSC_CBW_START:
      StartState(md, md->out_ch_num, (uint8_t *)&md->cbw,
                 sizeof(msc_bot_cbw_t), MSC_CBW_WAIT);
      break;
    case MSC_CBW_WAIT:
      if (WaitState(md, md->out_ch_num, md->cfg.interval,
                    MSC_CSW_START, MSC_CBW_NAKED, MSC_DONE)) {
        if (md->cbw.bmCBWFlags == MSC_BOT_CBW_DATA_IN) {
          md->state = MSC_DATA_IN_START;
        }
        else if (md->length > 0) {
          md->state = MSC_DATA_OUT_START;
        }
      }
      break;
    case MSC_CBW_NAKED:
      NackedState(md, md->out_ch_num, MSC_CBW_WAIT);
      break;
    case MSC_DATA_IN_START:
      StartState(md, md->in_ch_num, md->buffer, md->length,
                 MSC_DATA_IN_WAIT);
      break;
    case MSC_DATA_IN_WAIT:
      WaitState(md, md->in_ch_num, 0, MSC_CSW_START,
                MSC_DATA_IN_NAKED, MSC_DATA_IN_STALLED);
      md->transfered = USBHgetTransferSize(md->in_ch_num);
      break;
    case MSC_DATA_IN_NAKED:
      NackedState(md, md->in_ch_num, MSC_DATA_IN_WAIT);
      break;
    case MSC_DATA_IN_STALLED:
      StalledState(md, md->cfg.in_ep_addr);
      break;
    case MSC_DATA_OUT_START:
      StartState(md, md->out_ch_num, md->buffer, md->length,
                 MSC_DATA_OUT_WAIT);
      break;
    case MSC_DATA_OUT_WAIT:
      WaitState(md, md->out_ch_num, md->cfg.interval, MSC_CSW_START,
                MSC_DATA_OUT_NAKED, MSC_DATA_OUT_STALLED);
      md->transfered = USBHgetTransferSize(md->out_ch_num);
      break;
    case MSC_DATA_OUT_NAKED:
      NackedState(md, md->out_ch_num, MSC_DATA_OUT_WAIT);
      break;
    case MSC_DATA_OUT_STALLED:
      StalledState(md, md->cfg.out_ep_addr);
      break;
    case MSC_CSW_START:
      StartState(md, md->in_ch_num, (uint8_t *)&md->csw,
                 sizeof(msc_bot_csw_t), MSC_CSW_WAIT);
      break;
    case MSC_CSW_WAIT:
      if (WaitState(md, md->in_ch_num, 0,
                    MSC_DONE, MSC_CSW_NAKED, MSC_CSW_STALLED)) {
        md->csw.dCSWSignature = USBTOHL(md->csw.dCSWSignature);
        md->csw.dCSWDataResidue = USBTOHL(md->csw.dCSWDataResidue);
        if (IsCSWvalidAndMeaningful(md))
          md->status = md->csw.bCSWStatus;
        else
          md->status = MSC_BOT_CSW_PHASE_ERROR;
      }
      break;
    case MSC_CSW_NAKED:
      NackedState(md, md->in_ch_num, MSC_CSW_WAIT);
      break;
    case MSC_CSW_STALLED:
      if (++md->csw_attempts < 2) {
        StalledState(md, md->cfg.in_ep_addr);
      }
      else {
        md->status = MSC_BOT_UNDEF_ERROR;
        md->state = MSC_DONE;
      }
      break;
    default: /* MSC_EXIT, MSC_IDLE, MSC_DONE */
      break;
  }
  if (md->state == MSC_EXIT)
    FreeChannels(md);
  return md->state == MSC_EXIT;
}

static void MSCatSof(void *p, uint16_t frnum) {
  usbh_msc_data_t *md = p;

  if (md->nak_delay > 0)
    --md->nak_delay;
  if (md->nak_timeout > 0)
    --md->nak_timeout;
}

static void MSCatDisconnect(void *p) {
  usbh_msc_data_t *md = p;

  FreeChannels(md);
  md->status = MSC_BOT_UNDEF_ERROR;
  md->state = MSC_EXIT;
}

static usbh_msc_data_t MSCdata = {MSC_EXIT, -1, -1};

int MSCsetMachine(msc_configuration_t const *cfg) {
  int res;

  if ((cfg->out_ep_addr & ENDP_DIRECTION_MASK) != ENDP_OUT ||
      (cfg->in_ep_addr & ENDP_DIRECTION_MASK) != ENDP_IN)
    return USBHLIB_ERROR_INVALID_PARAM;

  MSCdata.state = MSC_INIT;
  MSCdata.status = MSC_BOT_UNDEF_ERROR;
  memcpy(&MSCdata.cfg, cfg, sizeof(msc_configuration_t));

  res = USBHsetClassMachine(MSCstateMachine, MSCatSof,
                            MSCatDisconnect, &MSCdata);
  if (res == USBHLIB_SUCCESS) {
    while (MSCdata.state == MSC_INIT)
      Delay(200); /* How long should we wait? */
    if (MSCdata.state != MSC_IDLE)
      res = USBHLIB_ERROR_BUSY; /* Channels are not allocated. */
  }
  return res;
}

int MSCisDeviceReady() {
  return MSCdata.state != MSC_EXIT;
}

static void MSCsubmit(uint8_t lun, uint8_t const *scsi_cmd,
                      uint32_t scsi_len, uint8_t const *out_buff,
                      uint8_t *in_buff, uint32_t len) {
  static uint32_t tag;

  /* Not completely received CSW must be recognized as not valid. */
  memset(&MSCdata.csw, 0xFF, sizeof(msc_bot_csw_t));
  memset(&MSCdata.cbw, 0, sizeof(msc_bot_cbw_t));
  MSCdata.cbw.dCBWSignature = HTOUSBL(MSC_BOT_CBW_SIGNATURE);
  MSCdata.cbw.dCBWTag = HTOUSBL(++tag);
  MSCdata.cbw.dCBWDataTransferLength = HTOUSBL(len);
  MSCdata.cbw.bCBWLUN = lun;
  MSCdata.cbw.bCBWCBLength = scsi_len;
  memcpy(MSCdata.cbw.CBWCB, scsi_cmd, scsi_len);
  if (in_buff) {
    MSCdata.cbw.bmCBWFlags = MSC_BOT_CBW_DATA_IN;
    MSCdata.buffer = in_buff;
  }
  else {
    MSCdata.cbw.bmCBWFlags = 0;
    /* The out_buff buffer is never modified. */
    MSCdata.buffer = (uint8_t *)out_buff;
  }
  /* Byte order of dCBWDataTransferLength is changed. */
  MSCdata.length = len;
  MSCdata.transfered = 0;
  MSCdata.csw_attempts = 0;
  MSCdata.status = MSC_BOT_UNDEF_ERROR;
  MSCdata.state = MSC_CBW_START;
}

uint8_t MSCBOT(int synch, uint8_t lun,
               uint8_t const *scsi_cmd, uint32_t scsi_len,
               uint8_t const *out_buff, uint8_t *in_buff,
               uint32_t *len) {
  if (lun > 15 || scsi_cmd == 0 || scsi_len > MSC_CBWCB_LENGTH ||
      len == 0 || (out_buff == 0 && in_buff == 0 && *len > 0))
    return MSC_BOT_UNDEF_ERROR;

  if (synch == 0) {
    if (MSCdata.state == MSC_IDLE) {
      MSCsubmit(lun, scsi_cmd, scsi_len, out_buff, in_buff, *len);
    }
    else if (MSCdata.state == MSC_DONE) {
      MSCdata.state = MSC_IDLE;
      *len = MSCdata.transfered;
    }
  }
  else {
    uint32_t x;
    x = USBHprotectInterrupt();
    if (MSCdata.state != MSC_IDLE) {
      USBHunprotectInterrupt(x);
      return USBHLIB_ERROR_BUSY;
    }
    else {
      MSCsubmit(lun, scsi_cmd, scsi_len, out_buff, in_buff, *len);
      while (MSCdata.state != MSC_DONE) {
        USBHunprotectInterrupt(x);
        Delay(200); /* How long should we wait? */
        x = USBHprotectInterrupt();
      }
      MSCdata.state = MSC_IDLE;
      *len = MSCdata.transfered;
    }
    USBHunprotectInterrupt(x);
  }
  return MSCdata.status;
}
