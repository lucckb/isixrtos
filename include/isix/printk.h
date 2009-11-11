/*-----------------------------------------------------------------------*/
#ifndef __ISIX_PRINTK_H
#define __ISIX_PRINTK_H
/*-----------------------------------------------------------------------*/
#include <isix/config.h>
/*-----------------------------------------------------------------------*/
#if (ISIX_DEBUG_ENABLE == ISIX_DBG_OFF)
#define printk(...)
#else
#include <foundation.h>
#define printk(...) do { isixp_enter_critical(); \
						 tiny_printf("%s:%d|%s|",__FILE__,__LINE__,__FUNCTION__); \
						 tiny_printf(__VA_ARGS__); \
						 tiny_printf("\r\n"); \
						 isixp_exit_critical(); \
					   } while(0)
#endif

/*-----------------------------------------------------------------------*/
#endif


