/* 
    Isix configuration file (C) 2007 L. Bryndza
*/
#pragma once

/** Memory protection model  */
#define ISIX_MPROT_NONE 0
#define ISIX_MPROT_LITE 1
#define ISIX_MPROT_FULL 2

#include <isix_config.h>

#if defined(CONFIG_ISIX_SHUTDOWN_API) && !defined(FUNCTION_MAIN_RETURN)
#error Shutdown API CONFIG_ISIX_SHUTDOWN_API require main crt exit code FUNCTION_MAIN_RETURN
#endif


