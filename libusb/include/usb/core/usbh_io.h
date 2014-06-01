#ifndef _USBH_IO_H
#define _USBH_IO_H 1

#include <usb/core/usb_def.h>

#ifdef __cplusplus
extern "C" {
#endif

/** USB host basic input-output API **/

int USBHchannelsConfigure(void);
int USBHallocChannel(void);
void USBHfreeChannel(int);
void USBHopenChannel(int, uint8_t, uint8_t,
                     usb_speed_t, usb_transfer_t, uint16_t);
void USBHmodifyChannel(int, uint8_t, uint16_t);

void USBHpacketReceived(int, uint32_t);
void USBHpacketSent(int);
void USBHtransferFinished(int, usbh_transaction_result_t);

int USBHstartTransaction(int, usb_pid_t, uint8_t *, uint32_t);
usbh_transaction_result_t USBHgetTransactionResult(int);
uint32_t USBHgetTransactionSize(int);

int USBHstartTransfer(int, uint8_t *, uint32_t);
int USBHrestartTransfer(int);
usbh_transaction_result_t USBHgetTransferResult(int);
uint32_t USBHgetTransferSize(int);
void USBHsetPID(int, usb_pid_t);

#ifdef __cplusplus
}
#endif

#endif
