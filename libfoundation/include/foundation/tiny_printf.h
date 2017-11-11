
/*
 * printf.h
 *
 *  Created on: 2009-08-08
 *      Author: lucck
 */

#ifndef TINY_PRINTF_H_
#define TINY_PRINTF_H_

#include <stddef.h>

#ifdef __cplusplus
namespace fnd {
extern "C" {
#endif


/** Printf replacement without floating point */
int tiny_printf(const char *format, ...) __attribute__ ((format (printf, 1, 2)));



/** Snprintf replacement without floating point aritmethic */
int tiny_snprintf(char *out, unsigned long len,  const char *format, ...) __attribute__ ((format (printf, 3, 4)));


void register_printf_putc_handler_syslock(int (*fputc)(int, void*),void *arg,
		void (*lock)(void),void (*unlock)(void));

static inline void register_printf_putc_handler(int (*fputc)(int, void*),void *arg)
{
	register_printf_putc_handler_syslock(fputc, arg, NULL, NULL );
}

int tiny_putchar( int ch );

int tiny_puts( const char* str );

#ifdef __cplusplus
 }
 }
#endif


#endif /* PRINTF_H_ */

