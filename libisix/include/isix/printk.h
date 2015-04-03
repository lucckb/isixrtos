/*-----------------------------------------------------------------------*/
#pragma once
/*-----------------------------------------------------------------------*/
#include <isix/config.h>
/*-----------------------------------------------------------------------*/
#if (ISIX_DEBUG_ENABLE == ISIX_DBG_OFF)
#define printk(...)
#else
#include <foundation/tiny_printf.h>

#define printk(...) do { _isixp_enter_critical(); \
						 tiny_printf("%s:%d|%s|",__FILE__,__LINE__,__FUNCTION__); \
						 tiny_printf(__VA_ARGS__); \
						 tiny_printf("\r\n"); \
						 _isixp_exit_critical(); \
					   } while(0)
#endif

/*-----------------------------------------------------------------------*/


