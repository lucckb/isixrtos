/* 
    Isix configuration file (C) 2007 L. Bryndza
*/
#pragma once


#include <isix_config.h>

#if defined(ISIX_CONFIG_SHUTDOWN_API) && !defined(FUNCTION_MAIN_RETURN)
#error Shutdown API ISIX_CONFIG_SHUTDOWN_API require main crt exit code FUNCTION_MAIN_RETURN
#endif


