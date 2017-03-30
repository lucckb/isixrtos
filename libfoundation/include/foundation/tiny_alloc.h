
/*
 * printf.h
 *
 *  Created on: 2009-08-08
 *      Author: lucck
 */

#pragma once

#if CONFIG_ISIX_WITHOUT_KERNEL

#include <stddef.h>

#ifdef __cplusplus
namespace fnd {
extern "C" {
#endif


	void *tiny_alloc(size_t size);
	void tiny_free(void *p);


#ifdef __cplusplus
 } }
#endif

#endif /* CONFIG_ISIX_WITHOUT_KERNEL */


