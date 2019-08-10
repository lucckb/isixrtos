/*
 * dbglog.h
 *
 *  Created on: 2010-03-21
 *      Author: lucck
 */

#pragma once


#include <config/conf.h>

/** Predefined configuration log levels */
#define FOUNDATION_DBGLOG_OFF 	0
#define FOUNDATION_DBGLOG_CRIT 	1
#define FOUNDATION_DBGLOG_ERR 	2
#define FOUNDATION_DBGLOG_WARN 	3
#define FOUNDATION_DBGLOG_INFO 	4
#define FOUNDATION_DBGLOG_DEBUG 5

/* Default log level */
#ifndef CONFIG_FOUNDATION_LOGLEVEL
#	ifdef PDEBUG
#		define CONFIG_FOUNDATION_LOGLEVEL FOUNDATION_DBGLOG_ERR
#	else
#		define CONFIG_FOUNDATION_LOGLEVEL FOUNDATION_DBGLOG_OFF
#	endif
#endif


#ifdef PDEBUG /* Debug enabled */
#include <foundation/sys/tiny_printf.h>

static inline const char* _isix_dbglog_extract_basename(const char *file) {
	size_t pos;
	for(pos=0; file&&file[pos]; ++pos) {}
	while(pos>0) {
		if((file[pos] == '/' || file[pos] == '\\')) return file+pos+1;
		else --pos;
	}
	return file;
}

#pragma GCC system_header
#ifdef __cplusplus
#define dblog_init(function,arg, usart_init,...)	\
  do { fnd::register_printf_putc_handler(function,arg); \
  	   usart_init(__VA_ARGS__); } while(0)
#define dblog_init_putc(function,arg) fnd::register_printf_putc_handler(function,arg)
#define dblog_init_putc_locked(function,arg,lock,unlock) fnd::register_printf_putc_handler_syslock(function,arg,lock,unlock)
#define dbprintf(fmt, ...) fnd::tiny_printf("%s:%d|" fmt "\r\n",_isix_dbglog_extract_basename(__FILE__),__LINE__,## __VA_ARGS__)
#define dblog_init_simple(function, arg ) fnd::register_printf_putc_handler(function,arg)
#else /*__cplusplus */

#define dblog_init(function,arg, usart_init,...)	\
  do { register_printf_putc_handler(function,arg); \
  	   usart_init(__VA_ARGS__); } while(0)
#define dblog_init_putc(function,arg) register_printf_putc_handler(function,arg)
#define dblog_init_putc_locked(function,arg,lock,unlock) register_printf_putc_handler_syslock(function,arg,lock,unlock)
#define dblog_init_simple(function, arg ) register_printf_putc_handler(function,arg)

//! Generic logger appi only if pdebug is defined
#define dbprintf(fmt, ...) tiny_printf("%s:%d|" fmt "\r\n",_isix_dbglog_extract_basename(__FILE__),__LINE__,## __VA_ARGS__)



#endif /*__cplusplus */

#define dblog_init_locked(function,arg,lock,unlock,usart_init,...) \
	do { dblog_init_putc_locked(function,arg,lock,unlock) ; \
		 usart_init(__VA_ARGS__); } while(0)

#else /* PDEBUG */

#define dblog_init(function,arg, usart_init,...) do {} while(0)
#define dblog_init_putc(function,arg) do {} while(0)
#define dbprintf(...) do {} while(0)
#define dblog_init_simple(function, arg ) do {} while(0)
#define dblog_init_putc_locked(function,arg,lock,unlock) do {} while(0)
#define dblog_init_locked(function,arg,lock,unlock,usart_init,...) do {} while(0)

#endif /* PDEBUG */


#if (CONFIG_FOUNDATION_LOGLEVEL==ISIXLOG_OFF) || !defined(PDEBUG)

#define dbg_crit(...) do {} while(0)
#define dbg_err(...) do {} while(0)
#define dbg_warn(...) do {} while(0)
#define dbg_info(...) do {} while(0)
#define dbg_debug(...) do {} while(0)

#else

#define dbg_crit(...) do { \
	if( CONFIG_FOUNDATION_LOGLEVEL>=FOUNDATION_DBGLOG_CRIT ) \
		dbprintf(__VA_ARGS__); } while(0)
#define dbg_err(...) do { \
	if( CONFIG_FOUNDATION_LOGLEVEL>=FOUNDATION_DBGLOG_ERR ) \
		dbprintf(__VA_ARGS__); } while(0)
#define dbg_warn(...) do { \
	if( CONFIG_FOUNDATION_LOGLEVEL>=FOUNDATION_DBGLOG_WARN ) \
		dbprintf(__VA_ARGS__); } while(0)
#define dbg_info(...) do { \
	if( CONFIG_FOUNDATION_LOGLEVEL>=FOUNDATION_DBGLOG_INFO ) \
		dbprintf(__VA_ARGS__); } while(0)
#define dbg_debug(...) do { \
	if( CONFIG_FOUNDATION_LOGLEVEL>=FOUNDATION_DBGLOG_DEBUG ) \
		dbprintf(__VA_ARGS__); } while(0)

#endif

