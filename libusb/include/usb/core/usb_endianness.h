#ifndef _USB_ENDIANNESS_H
#define _USB_ENDIANNESS_H 1

/* STM32 is little-endian, hence no byte reversal is needed. */
#define HTOUSBS(x) (x)
#define USBTOHS(x) (x)
#define HTOUSBL(x) (x)
#define USBTOHL(x) (x)

#endif
