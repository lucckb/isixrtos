#pragma once

#include <isix/types.h>
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/


typedef struct  isix_memory_stat {
	size_t free;
	size_t used;
	size_t fragments;
} isix_memory_stat_t;

/** Allocate the memory from the global heap
 * @param[in] size Allocated size
 * @return Pointer to the allocated area otherwise NULL
 */
void* isix_alloc(size_t size);

/** Free alocated earlier memory area
 * param[in] mem Pointer to the allocated memory
 */
void isix_free(void *mem);


/** Function display current memory usage
 * @param[out] meminfo Information about heap memory structure
 * @return free memory space
 */
void isix_heap_stats( isix_memory_stat_t* meminfo );


/** Function return heap current memory size
 *	@param[in] Pointer to the dynamic allocated memory
 *	@return 0 if mem is not dynamic block or size if it is
 */
size_t isix_heap_getsize( void* ptr );


/** Function changes the sise of the memory block pointed
 * by ptr to size bytes
 * @param[in] ptr Memory region for change alloc size
 * @param[in] size New requested size
 * @return Pointer to newly alocated memory or NULL on error
 */
void* isix_realloc(void *ptr, size_t size );


#ifdef __cplusplus
}	//end extern-C
#endif /* __cplusplus */


/** Extra C++ interface for memory allocator */
#ifdef __cplusplus
namespace isix {
namespace {
	using memory_stat = ::isix_memory_stat;
	inline void* alloc( size_t size ) {
		return ::isix_alloc( size );
	}
	inline void free(void *mem) {
		::isix_free( mem );
	}
	inline void heap_stats( memory_stat& meminfo ) {
		::isix_heap_stats( &meminfo );
	}
	inline size_t heap_getsize( void* ptr )
	{
		return ::isix_heap_getsize( ptr );
	}
	inline void* realloc(void *ptr, size_t size )
	{
		return ::isix_realloc( ptr, size );
	}
}}
#endif /*__cplusplus*/
