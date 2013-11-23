/* 
    Isix configuration file (C) 2007 L. Bryndza
*/

#ifndef _ISIX_CONFIG
#define _ISIX_CONFIG

#define ISIX_DBG_ON 1
#define ISIX_DBG_OFF 0
#define ISIX_ON 1
#define ISIX_OFF 0

#include <isix_config.h>

#if defined(ISIX_CONFIG_SHUTDOWN_API) && !defined(FUNCTION_MAIN_RETURN)
#error Shutdown API ISIX_CONFIG_SHUTDOWN_API require main crt exit code FUNCTION_MAIN_RETURN
#endif

#endif
