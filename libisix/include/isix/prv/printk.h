#pragma once
#ifndef _ISIX_KERNEL_CORE_
#	error This is private header isix kernel headers cannot be used by app
#endif

#define ISIXLOG_OFF 0
#define ISIXLOG_CRIT 1
#define ISIXLOG_ERR 2
#define ISIXLOG_WARN 3
#define ISIXLOG_INFO 4
#define ISIXLOG_DEBUG 5

/** Logging level */
#include <isix/config.h>


#if (CONFIG_ISIX_LOGLEVEL == ISIXLOG_OFF )
#define printk(...)
#else
int tiny_printf(const char *format, ...) __attribute__ ((format (printf, 1, 2)));

#define printk(level,...) do { if(CONFIG_ISIX_LOGLEVEL>=level) { \
						 isix_enter_critical(); \
						 tiny_printf("%s:%d|%s|",__FILE__,__LINE__,__FUNCTION__); \
						 tiny_printf(__VA_ARGS__); \
						 tiny_printf("\r\n"); \
						 isix_exit_critical(); } \
					   } while(0)

#endif

#define pr_crit(...) printk(ISIXLOG_CRIT,__VA_ARGS__)
#define pr_err(...) printk(ISIXLOG_ERR,__VA_ARGS__)
#define pr_warn(...) printk(ISIXLOG_WARN,__VA_ARGS__)
#define pr_info(...) printk(ISIXLOG_INFO,__VA_ARGS__)
#define pr_debug(...) printk(ISIXLOG_DEBUG,__VA_ARGS__)


