#pragma once

#ifdef __GNUC__
#   if __GNUC_PREREQ(11,1)
#       define ISIX_ISR_ATTRIBUTE  __attribute__((optimize("align-functions=8"))) 
#  else
#       define ISIX_ISR_ATTRIBUTE  __attribute__((__interrupt__))
#  endif
#else
#   error Isix supports only GCC compiler
#endif


#ifdef __cplusplus
#define _ISIX_ISR_EXTERN_C_  extern "C"
#else
#define _ISIX_ISR_EXTERN_C_
#endif

#define _ISIX_NAKED_ATTRIBUTE_  __attribute__((naked)) 

#define ISIX_ISR_VECTOR(FUNC) _ISIX_ISR_EXTERN_C_ ISIX_ISR_ATTRIBUTE void FUNC(void) 
#define ISIX_ISR_NACKED_VECTOR(FUNC) _ISIX_ISR_EXTERN_C_ ISIX_ISR_ATTRIBUTE _ISIX_NAKED_ATTRIBUTE_ void FUNC(void) 
