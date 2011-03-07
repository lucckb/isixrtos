/* ------------------------------------------------------------ */
/*
 * printf.h
 *
 *  Created on: 2009-08-08
 *      Author: lucck
 */
/* ------------------------------------------------------------ */
#ifndef TINY_PRINTF_H_
#define TINY_PRINTF_H_
/* ------------------------------------------------------------ */

#ifdef __cplusplus
namespace fnd {
extern "C" {
#endif

/* ------------------------------------------------------------ */

int tiny_printf(const char *format, ...);

/* ------------------------------------------------------------ */

int tiny_snprintf(char *out, unsigned long len,  const char *format, ...);


/* ------------------------------------------------------------ */

void register_printf_putc_handler_syslock(int (*fputc)(int, void*),void *arg,
		void (*lock)(void),void (*unlock)(void));

static inline void register_printf_putc_handler(int (*fputc)(int, void*),void *arg)
{
	register_printf_putc_handler_syslock(fputc, arg, NULL, NULL );
}

/* ------------------------------------------------------------ */
#ifdef __cplusplus
 }
 }
#endif


#endif /* PRINTF_H_ */

