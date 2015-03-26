/* 
    Isix configuration file (C) 2007 L. Bryndza
*/
#pragma once
#define ISIX_DBG_ON 1
#define ISIX_DBG_OFF 0

#include <isix_config.h>

#if defined(ISIX_CONFIG_SHUTDOWN_API) && !defined(FUNCTION_MAIN_RETURN)
#error Shutdown API ISIX_CONFIG_SHUTDOWN_API require main crt exit code FUNCTION_MAIN_RETURN
#endif

#if defined(ISIX_CONFIG_USE_MULTIOBJECTS)
#error ISIX_CONFIG_USE_MULTIOBJECTS multiobjects not implemented yet
#endif

