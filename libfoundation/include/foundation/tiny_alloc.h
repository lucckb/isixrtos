/* ------------------------------------------------------------ */
/*
 * printf.h
 *
 *  Created on: 2009-08-08
 *      Author: lucck
 */
/* ------------------------------------------------------------ */
#ifndef TINY_ALLOC_H_
#define TINY_ALLOC_H_

#ifndef COMPILED_UNDER_ISIX
/* ------------------------------------------------------------ */
#include <stddef.h>

/* ------------------------------------------------------------ */
#ifdef __cplusplus
namespace fnd {
extern "C" {
#endif

/* ------------------------------------------------------------ */
//! Initialize global heap
void tiny_alloc_init(void);

/* ------------------------------------------------------------ */
void *tiny_alloc(size_t size);

/* ------------------------------------------------------------ */

void tiny_free(void *p);

/* ------------------------------------------------------------ */
#ifdef __cplusplus
 }
 }
#endif

#endif /* COMPILED_UNDER_ISIX */

#endif /* TINY_ALLOC_H_ */

