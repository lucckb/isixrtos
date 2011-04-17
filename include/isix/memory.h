#ifndef _ISIX_MEMORY_H
#define _ISIX_MEMORY_H
/*------------------------------------------------------*/
#include <isix/types.h>
/*------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
namespace isix {
#endif /*__cplusplus*/

/*------------------------------------------------------*/
/** Allocate the memory from the global heap
 * @param[in] size Allocated size
 * @return Pointer to the allocated area otherwise NULL
 */
void* isix_alloc(size_t size);

/*------------------------------------------------------*/
/** Free alocated earlier memory area
 * param[in] mem Pointer to the allocated memory
 */
void isix_free(void *mem);

/*------------------------------------------------------*/
//! Initialize global heap
void isix_alloc_init(void);

/*------------------------------------------------------*/
/** Function display current memory usage
 * @param[out] fragments - return number of fragments mem
 * @return free memory space
 */
size_t isix_heap_free(int *fragments);


/*------------------------------------------------------*/
#ifdef __cplusplus
}	//end namespace
}	//end extern-C
#endif /* __cplusplus */

/*------------------------------------------------------*/
#endif
