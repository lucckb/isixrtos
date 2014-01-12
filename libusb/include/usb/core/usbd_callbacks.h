#ifndef _USBD_CALLBACKS_H
#define _USBD_CALLBACKS_H 1

#include <usb/core/usb_def.h>

/** USB device application callbacks **/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  /* Application configuration */
  int          (*Configure)(void);
  /* Interrupt callbacks */
  uint8_t      (*Reset)(usb_speed_t speed);
  void         (*SoF)(uint16_t frameNumber);
  /* Standard request callbacks */
  usb_result_t (*GetDescriptor)(uint16_t wValue, uint16_t wIndex,
                                uint8_t const **data,
                                uint16_t *length);
  usb_result_t (*SetDescriptor)(uint16_t wValue, uint16_t wIndex,
                                uint16_t wLength, uint8_t **data);
  uint8_t      (*GetConfiguration)(void);
  usb_result_t (*SetConfiguration)(uint16_t configurationValue);
  uint16_t     (*GetStatus)(void);
  usb_result_t (*GetInterface)(uint16_t interface, uint8_t *setting);
  usb_result_t (*SetInterface)(uint16_t interface, uint16_t setting);
  usb_result_t (*ClearDeviceFeature)(uint16_t featureSelector);
  usb_result_t (*SetDeviceFeature)(uint16_t featureSelector);
  /* Class request callbacks */
  usb_result_t (*ClassNoDataSetup)(usb_setup_packet_t const *setup);
  usb_result_t (*ClassInDataSetup)(usb_setup_packet_t const *setup,
                                   uint8_t const **data,
                                   uint16_t *length);
  usb_result_t (*ClassOutDataSetup)(usb_setup_packet_t const *setup,
                                    uint8_t **data);
  void         (*ClassStatusIn)(usb_setup_packet_t const *setup);
  /* Endpoint callbacks, the callback index is the endpoint address
     decreased by 1. */
  void         (*EPin[15])(void);
  void         (*EPout[15])(void);
  /* Peripheral power management callbacks */
  void         (*Suspend)(void);
  void         (*Wakeup)(void);
} usbd_callback_list_t;

/** Application initialization callback **/

usbd_callback_list_t const * USBDgetApplicationCallbacks(void);

#ifdef __cplusplus
}
#endif


#endif
