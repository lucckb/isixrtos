/*
 * =====================================================================================
 *
 *       Filename:  tiny_vaprintf.h
 *
 *    Description:  Va printf internal implementation
 *
 *        Version:  1.0
 *        Created:  05.05.2014 16:58:42
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */



#include <stdarg.h>
#include <stddef.h>

#ifdef __cplusplus
namespace fnd {
extern "C" {
#endif



int tiny_vaprintf(char **out, size_t len, const char *format, va_list args );


#ifdef __cplusplus
}
}

#endif
