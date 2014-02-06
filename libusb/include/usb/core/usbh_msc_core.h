#ifndef _USBH_MSC_CORE_H
#define _USBH_MSC_CORE_H 1

#include <usb/core/usb_def.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  usb_speed_t speed;
  uint16_t    out_ep_max_packet;
  uint16_t    in_ep_max_packet;
  uint16_t    sector_size;
  uint8_t     dev_addr;
  uint8_t     configuration_value;
  uint8_t     interface_number;
  uint8_t     out_ep_addr;
  uint8_t     in_ep_addr;
  uint8_t     interval;
  uint8_t     max_lun;
} msc_configuration_t;

int MSCsetMachine(msc_configuration_t const *);
int MSCisDeviceReady(void);
int MSCBOT(int, uint8_t, uint8_t const *, uint32_t,
               uint8_t const *, uint8_t *, uint32_t *);

#ifdef __cplusplus
}
#endif

#endif
