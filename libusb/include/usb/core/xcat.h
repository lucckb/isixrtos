#ifndef _XCAT_H
#define _USBLIB_XCAT_H
#define _USBLIB_XCAT_H

#define _usblib_cat(a, b) a##b
#define _usblib_cat3(a, b, c) a##b##c
#define _usblib_cat4(a, b, c, d) a##b##c##d
#define _usblib_cat5(a, b, c, d, e) a##b##c##d##e
#define usblib_xcat(a, b) _usblib_cat(a, b)
#define usblib_xcat3(a, b, c) _usblib_cat3(a, b, c)
#define usblib_xcat4(a, b, c, d) _usblib_cat4(a, b, c, d)
#define usblib_xcat5(a, b, c, d, e) _usblib_cat5(a, b, c, d, e)

#define ARRAY_SIZE( x ) (sizeof(x)/sizeof(x[0]))

#endif
