#pragma once

#ifdef __cplusplus
extern "C" {
#endif

struct isix_mutex;
//! Created mutex handler
typedef struct isix_mutex* osmtx_t;


/** Function create the recursive mutex with priority inheritance
 * @param[in] mutex Optional static allocated mutex
 * @return Mutex object or NULL when cannot be created
 */
osmtx_t isix_mutex_create( osmtx_t mutex );

/** Function lock the recursive mutex
 * @param[in] mutex Mutex object
 * @return Error code
 */
int isix_mutex_lock( osmtx_t mutex );

/** Function unlock the recursive mutex
 * @param[in] mutex Mutex object
 * @return Error code
 */
int isix_mutex_unlock( osmtx_t mutex );

/** Function trying to lock the recurcive mutex
 * @param[in] mutex Mutex object
 * @return Error code
 */
int isix_mutex_trylock( osmtx_t mutex );

/** Function unlock all objects waiting in the current task
 * @return Error code
 */
void isix_mutex_unlock_all(void);


/** Destroy the recursive mutex
 * @param[in] mutex Recursive mutex object
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */
int isix_mutex_destroy( osmtx_t mutex );

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
namespace isix {
namespace {
	using mtx_t = osmtx_t;

	inline osmtx_t mutex_create( osmtx_t mutex = nullptr ) {
		return ::isix_mutex_create( mutex );
	}
	inline int mutex_lock( osmtx_t mutex ) {
		return ::isix_mutex_lock( mutex );
	}
	inline int mutex_unlock( osmtx_t mutex ) {
		return ::isix_mutex_unlock( mutex );
	}
	inline int mutex_trylock( osmtx_t mutex ) {
		return ::isix_mutex_trylock( mutex );
	}
	inline void mutex_unlock_all() {
		::isix_mutex_unlock_all();
	}
	inline int mutex_destroy( osmtx_t mutex ) {
		return ::isix_mutex_destroy( mutex );
	}

}}
#endif
