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
#include <tiny_printf.h>

#pragma GCC system_header

#ifdef __cplusplus

#define dblog_init(function,arg, usart_init,...)	\
  do { fnd::register_printf_putc_handler(function,arg); \
  	   usart_init(__VA_ARGS__); } while(0)
#define dbprintf(fmt, ...) fnd::tiny_printf("%s:%d|"fmt"\r\n",__FILE__,__LINE__,## __VA_ARGS__)

#else

#define dblog_init(function,arg, usart_init,...)	\
  do { register_printf_putc_handler(function,arg); \
  	   usart_init(__VA_ARGS__); } while(0)
#define dbprintf(fmt, ...) tiny_printf("%s:%d|"fmt"\r\n",__FILE__,__LINE__,## __VA_ARGS__)


#endif /*__cplusplus */


#else

#define dblog_init(function,arg, usart_init,...) do {} while(0)
#define dbprintf(fmt,...) do {} while(0)


#endif /* PDEBUG */


/* ------------------------------------------------------------------ */
#endif /* DBGLOG_H_ */
/* ------------------------------------------------------------------ */
