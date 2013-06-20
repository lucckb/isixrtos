/*
 * usbhost.h
 *
 *  Created on: 20-06-2013
 *      Author: lucck
 */

#ifndef LIBISIX_DRV_USBHOST_H_
#define LIBISIX_DRV_USBHOST_H_
/* ------------------------------------------------------------------ */
#include <stddef.h>

/* ------------------------------------------------------------------ */
#ifdef __cplusplus
extern "C" {
namespace stm32 {
namespace dev {
#endif

/* ------------------------------------------------------------------ */
int stm32_usbhost_init(void);


/* ------------------------------------------------------------------ */

#ifdef __cplusplus
}}}
#endif
/* ------------------------------------------------------------------ */
#endif /* USBHOST_H_ */
