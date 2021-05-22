#pragma once

#include <isix/config.h>
#include <isix/types.h>
#include <isix/arch/irq_cpu.h>

//ISIX BUG macro
#define isix_bug( msg ) isix_kernel_panic(__FILE__,__LINE__, (msg) )


#define ISIX_ASSERT_ISR_ONE_OR_TWO(...) ISIX_ASSERT_ISR_ONE_OR_TWO_(__VA_ARGS__, 2, 1,)
#define ISIX_ASSERT_ISR_ONE_OR_TWO_(_1, _2, X, ...) X
#define ISIX_ASSERT_ISR_CAT(A, B) ISIX_ASSERT_ISR_CAT_(A, B)
#define ISIX_ASSERT_ISR_CAT_(A, B) A ## B

#define isix_assert_isr(X) ISIX_ASSERT_ISR_CAT(ISIX_ASSERT_ISR_, ISIX_ASSERT_ISR_ONE_OR_TWO(ISIX_ASSERT_ISR_FUNC_IMPL_ ## X))(X)
#define ISIX_ASSERT_ISR_FUNC_IMPL_ ,

#ifdef PDEBUG
//! Ensure that not called from the interrupt context
#define ISIX_ASSERT_ISR_2() \
    do { \
        if(_isix_port_is_in_isr()) \
            isix_kernel_panic(__FILE__,__LINE__,"IsixAssert: Called from ISR context"); \
    } while(0)
//! Assert when in isr with args
#define ISIX_ASSERT_ISR_1(IN_ISR) \
    do { \
        if(!(IN_ISR) && _isix_port_is_in_isr()) \
            isix_kernel_panic(__FILE__,__LINE__,"IsixAssert: Called from ISR context"); \
    } while(0)
//! Assert when in isr with args
#else
#define ISIX_ASSERT_ISR_2() do {} while(0)
#define ISIX_ASSERT_ISR_1(IN_ISR) (void)(IN_ISR)
#endif



#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

//! Halt system when critical error is found
void isix_kernel_panic( const char *file, int line, const char *msg );


#ifdef __cplusplus
}
#endif /*__cplusplus*/
