#ifndef _ISIX_USBH_ERROR_H
#define _ISIX_USBH_ERROR_H

enum usbh_errno {
	USBHLIB_SUCCESS              =   0,
	USBHLIB_ERROR_IO             =  -2048,
	USBHLIB_ERROR_INVALID_PARAM  =  -2049,
	USBHLIB_ERROR_NO_DEVICE      =  -2050,
	USBHLIB_ERROR_NOT_FOUND      =  -2052,
	USBHLIB_ERROR_BUSY           =  -2053,
	USBHLIB_ERROR_TIMEOUT        =  -2054,
	USBHLIB_ERROR_STALL          =  -2055,
	USBHLIB_ERROR_NO_MEM         = -2056,
	USBHLIB_ERROR_NOT_SUPPORTED  = -2057,
	USBHLIB_IN_PROGRESS          = -2058,
	USBHLIB_ERROR_OS			 = -2059,
	USBHLIB_ERROR_EXISTS		 = -2060,
	USBHLIB_ERROR_DESC_PARSE	 = -2061,
	USBHLIB_ERROR_NO_DRIVER		 = -2062
};

#endif
