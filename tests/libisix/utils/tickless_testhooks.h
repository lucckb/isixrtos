#pragma once

#include <isix/types.h>

#ifdef __cplusplus
extern "C" {
#endif

void _isixp_test_set_jiffies(ostick_t v);
void _isixp_test_advance_jiffies(ostick_t dt);

#ifdef __cplusplus
}
#endif
