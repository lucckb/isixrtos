/* ------------------------------------------------------------------ */
/*
 * dbglog.h
 *
 *  Created on: 2010-03-21
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#ifndef DBGLOG_H_
#define DBGLOG_H_
/* ------------------------------------------------------------------ */
#ifdef PDEBUG /* Debug enabled */
#include <foundation/tiny_printf.h>

#pragma GCC system_header

#ifdef __cplusplus

#define dblog_init(function,arg, usart_init,...)	\
  do { fnd::register_printf_putc_handler(function,arg); \
  	   usart_init(__VA_ARGS__); } while(0)
#define dblog_init_putc(function,arg) fnd::register_printf_putc_handler(function,arg)
#define dblog_init_putc_locked(function,arg,lock,unlock) fnd::register_printf_putc_handler_syslock(function,arg,lock,unlock)
#define dbprintf(fmt, ...) fnd::tiny_printf("%s:%d|" fmt "\r\n",__FILE__,__LINE__,## __VA_ARGS__)
#define dblog_init_simple(function, arg ) fnd::register_printf_putc_handler(function,arg)

#else /*__cplusplus */

#define dblog_init(function,arg, usart_init,...)	\
  do { register_printf_putc_handler(function,arg); \
  	   usart_init(__VA_ARGS__); } while(0)
#define dblog_init_putc(function,arg) register_printf_putc_handler(function,arg)
#define dblog_init_putc_locked(function,arg,lock,unlock) register_printf_putc_handler_syslock(function,arg,lock,unlock)
#define dbprintf(fmt, ...) tiny_printf("%s:%d|" fmt "\r\n",__FILE__,__LINE__,## __VA_ARGS__)
#define dblog_init_simple(function, arg ) register_printf_putc_handler(function,arg)

#endif /*__cplusplus */


#else

#define dblog_init(function,arg, usart_init,...) do {} while(0)
#define dblog_init_putc(function,arg) do {} while(0)
#define dbprintf(...) do {} while(0)
#define dblog_init_simple(function, arg ) do {} while(0)
#define dblog_init_putc_locked(function,arg,lock,unlock) do {} while(0)
#endif /* PDEBUG */


/* ------------------------------------------------------------------ */
#endif /* DBGLOG_H_ */
/* ------------------------------------------------------------------ */
