#include <isix.h>
#include <sys/lock.h>

struct __lock {
    osmtx_t mutex;
};


#if defined(_RETARGETABLE_LOCKING)
struct __lock __lock___sinit_recursive_mutex;
struct __lock __lock___sfp_recursive_mutex;
struct __lock __lock___atexit_recursive_mutex;
struct __lock __lock___at_quick_exit_mutex;
//struct __lock __lock___malloc_recursive_mutex;
struct __lock __lock___env_recursive_mutex;
struct __lock __lock___tz_mutex;
struct __lock __lock___dd_hash_mutex;
struct __lock __lock___arc4random_mutex;


__attribute__((constructor))
static void init_retarget_locks(void)
{
    __lock___sinit_recursive_mutex.mutex  = isix_mutex_create(NULL);
    __lock___sfp_recursive_mutex.mutex    = isix_mutex_create(NULL);
    __lock___atexit_recursive_mutex.mutex = isix_mutex_create(NULL);
    __lock___at_quick_exit_mutex.mutex      = isix_mutex_create(NULL);
    //__lock___malloc_recursive_mutex.mutex = isix_mutex_create(NULL);  // We are not using malloc
    __lock___env_recursive_mutex.mutex    = isix_mutex_create(NULL);
    __lock___tz_mutex.mutex               = isix_mutex_create(NULL); 
    __lock___dd_hash_mutex.mutex          = isix_mutex_create(NULL);
    __lock___arc4random_mutex.mutex       = isix_mutex_create(NULL);
}


__attribute__(( section(".preinit_array"), used ))
    static void (* const preinit_array[])(void) = { &init_retarget_locks };

void __retarget_lock_init(_LOCK_T *lock)
{
   *lock = isix_alloc(sizeof(struct __lock));
    (*lock)->mutex = isix_mutex_create(NULL);
}

void __retarget_lock_init_recursive(_LOCK_T *lock)
{
   *lock = isix_alloc(sizeof(struct __lock));
    (*lock)->mutex = isix_mutex_create(NULL);
}

void __retarget_lock_close(_LOCK_T lock)
{
    isix_mutex_destroy(lock->mutex);
    isix_free(lock);
}

void __retarget_lock_close_recursive(_LOCK_T lock)
{
    isix_mutex_destroy(lock->mutex);
    isix_free(lock);
}

void __retarget_lock_acquire(_LOCK_T lock)
{
    isix_mutex_lock(lock->mutex);
}

void __retarget_lock_acquire_recursive(_LOCK_T lock)
{
    isix_mutex_lock(lock->mutex);
}

int __retarget_lock_try_acquire(_LOCK_T lock)
{
    return isix_mutex_trylock(lock->mutex);
}

int __retarget_lock_try_acquire_recursive(_LOCK_T lock)
{
    return isix_mutex_trylock(lock->mutex);
}

void __retarget_lock_release(_LOCK_T lock)
{
    isix_mutex_unlock(lock->mutex);
}
void __retarget_lock_release_recursive(_LOCK_T lock)
{
    isix_mutex_unlock(lock->mutex);
}

#else /* _RETARGETABLE_LOCKING */

/*    
// Implement when use newlib malloc 
void __malloc_lock(struct _reent *r)
void __malloc_unlock(struct _reent *r) 
*/

#endif /* _RETARGETABLE_LOCKING */
