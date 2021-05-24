#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


static inline __attribute__((always_inline))
bool _isix_port_is_in_isr(void)
{
    uint32_t result;
    asm volatile ("MRS %0, ipsr" : "=r" (result) );
    return result;
}

#ifdef __cplusplus
}
#endif
