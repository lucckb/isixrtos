#if CONFIG_ISIX_WITHOUT_KERNEL
#include "foundation/sys/tiny_alloc.h"
#else /* CONFIG_ISIX_WITHOUT_KERNEL */
#include <isix.h>
#include <isix/prv/semaphore.h>
#endif
#include <cstring>

#if CONFIG_ISIX_WITHOUT_KERNEL

#if !CONFIG_FOUNDATION_NO_DYNAMIC_ALLOCATION
//It can be redefined
#define foundation_alloc fnd::tiny_alloc
#define foundation_free fnd::tiny_free
#define foundation_realloc(ptr, len) (ptr && len) ? nullptr : nullptr
#else /*CONFIG_FOUNDATION_NO_DYNAMIC_ALLOCATION */
#define foundation_alloc(x) ((x) ? NULL : NULL)
#define foundation_free(x) \
    do                     \
    {                      \
        (void)(x);         \
    } while (0)
#define foudation_realloc(ptr, len) (ptr && len) ? nullptr : nullptr
#endif /*CONFIG_FOUNDATION_NO_DYNAMIC_ALLOCATION */
#define terminate_process() while (1)

#else /* CONFIG_ISIX_WITHOUT_KERNEL */

#define foundation_alloc isix_alloc
#define foundation_free isix_free
#define foundation_realloc isix_realloc
#define terminate_process() isix_bug(__PRETTY_FUNCTION__)

#endif /* CONFIG_ISIX_WITHOUT_KERNEL */

#ifndef __EXCEPTIONS
void *operator new(size_t n) throw()
{
    if (n == 0)
        n++;
    void *ptr = foundation_alloc(n);
    if (!ptr)
        terminate_process();
    return ptr;
}

void operator delete(void *p) throw()
{
    if (p)
    {
        foundation_free(p);
    }
}

void operator delete(void *p, size_t) throw()
{
    if (p)
    {
        foundation_free(p);
    }
}

void *operator new[](size_t n) throw()
{
    if (n == 0)
        n++;
    void *ptr = foundation_alloc(n);
    if (!ptr)
        terminate_process();
    return ptr;
}

void operator delete[](void *p) throw()
{
    if (p)
    {
        foundation_free(p);
    }
}

void operator delete[](void *p, size_t) throw()
{
    if (p)
    {
        foundation_free(p);
    }
}
#endif

extern "C" {
    void* malloc(size_t size) __attribute__((used));
	void *_malloc_r(struct _reent */*r*/, size_t size);

	void free(void *ptr) __attribute__((used));
	void _free_r(struct _reent */*r*/, void *ptr);

	void *calloc(size_t nmemb, size_t size) __attribute__((used));
	void *_calloc_r(struct _reent */*r*/,size_t nmemb, size_t size) __attribute__((used));

	void *realloc(void */*ptr*/, size_t /*size*/);
	void *_realloc_r(struct _reent */*r*/,void */*ptr*/, size_t /*size*/);
}

void* malloc(size_t size)
{
	return foundation_alloc(size);
}
void *_malloc_r(struct _reent */*r*/, size_t size)
{
	return foundation_alloc(size);
}

void free(void *ptr)
{
	foundation_free(ptr);
}

void _free_r(struct _reent */*r*/, void *ptr)
{
	foundation_free(ptr);
}

void *calloc(size_t nmemb, size_t size)
{
	size_t ns = nmemb * size;
	void *ptr = foundation_alloc(ns);
	if(ptr)
	{
		std::memset(ptr,0,ns);
	}
	return ptr;
}

void *_calloc_r(struct _reent */*r*/,size_t nmemb, size_t size)
{
	size_t ns = nmemb * size;
	void *ptr = foundation_alloc(ns);
	if(ptr)
	{
		std::memset(ptr,0,ns);
	}
	return ptr;
}

void *realloc(void* ptr, size_t size)
{
	return foundation_realloc( ptr, size );
}

void *_realloc_r(struct _reent */*r*/,void* ptr, size_t size)
{
	return foundation_realloc( ptr, size );
}
