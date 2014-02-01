#ifndef _USBH_ERROR_H
#define _USBH_ERROR_H 1

enum usbh_errno {
	USBHLIB_SUCCESS              =   0,
	USBHLIB_ERROR_IO             =  -1,
	USBHLIB_ERROR_INVALID_PARAM  =  -2,
	USBHLIB_ERROR_NO_DEVICE      =  -4,
	USBHLIB_ERROR_NOT_FOUND      =  -5,
	USBHLIB_ERROR_BUSY           =  -6,
	USBHLIB_ERROR_TIMEOUT        =  -7,
	USBHLIB_ERROR_STALL          =  -9,
	USBHLIB_ERROR_NO_MEM         = -11,
	USBHLIB_ERROR_NOT_SUPPORTED  = -12,
	USBHLIB_IN_PROGRESS          = -50

};

#endif
