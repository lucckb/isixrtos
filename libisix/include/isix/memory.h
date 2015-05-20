#pragma once

#include <isix/types.h>
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

/** Allocate the memory from the global heap
 * @param[in] size Allocated size
 * @return Pointer to the allocated area otherwise NULL
 */
void* isix_alloc(size_t size);

/** Free alocated earlier memory area
 * param[in] mem Pointer to the allocated memory
 */
void isix_free(void *mem);

//! Initialize global heap
void _isixp_alloc_init(void);

/** Function display current memory usage
 * @param[out] fragments - return number of fragments mem
 * @return free memory space
 */
size_t isix_heap_free(int *fragments);

#ifdef __cplusplus
}	//end extern-C
#endif /* __cplusplus */



#ifdef __cplusplus
namespace isix {
namespace {
	inline void* alloc( size_t size ) {
		return ::isix_alloc( size );
	}
	inline void free(void *mem) {
		::isix_free( mem );	
	}
	inline size_t heap_free(int *fragments=nullptr) {
		return ::isix_heap_free( fragments );
	}
}}
#endif /*__cplusplus*/
