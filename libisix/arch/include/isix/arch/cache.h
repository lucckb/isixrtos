/*
 * =====================================================================================
 *
 *       Filename:  cache.h
 *
 *    Description:  Isix cache maintenance functions
 *
 *        Version:  1.0
 *        Created:  23.04.2017 20:52:07
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Enable or disable icache
 * @parm[in] yes Enable or disable
 */
void isix_icache_enable( bool yes );

/** Enable or disable dcache
 * @parm[in] yes Enable or disable
 */
void isix_dcache_enable( bool yes );

/** Invalidate data cache  */
void isix_inval_dcache( void );

/** Invalidate instruction cache  */
void isix_inval_icache( void );

/** Clean instruction cache  */
void isix_clean_dcache( void );

/** Clean and invalidate data cache  */
void isix_clean_inval_dcache( void );

/** Invalidate data cache in selected range
 * @param[in] addr Input starting address
 * @param[in] dsize Area size
 */
void isix_inval_dcache_by_addr( void *addr, size_t dsize );

/** Clean data cache in selected range
 * @param[in] addr Input starting address
 * @param[in] dsize Area size
 */
void isix_clean_dcache_by_addr( void *addr, size_t dsize );

/** Clean and invalidate data cache in selected range
 * @param[in] addr Input starting address
 * @param[in] dsize Area size
 */
void isix_clean_inval_dcache_by_addr( void *addr, size_t dsize );




#ifdef __cplusplus
}
#endif



#ifdef __cplusplus
namespace isix {

static inline __attribute__((always_inline))
void icache_enable( bool yes ) {
	::isix_icache_enable( yes );
}

static inline __attribute__((always_inline))
void dcache_enable( bool yes ) {
	::isix_dcache_enable(yes);
}


static inline __attribute__((always_inline))
void inval_dcache( void )
{
	::isix_inval_dcache();
}

static inline __attribute__((always_inline))
void inval_icache( void )
{
	::isix_inval_icache();
}

static inline __attribute__((always_inline))
void clean_dcache( void )
{
	::isix_clean_dcache();
}

static inline __attribute__((always_inline))
void clean_inval_dcache( void )
{
	::isix_clean_inval_dcache();
}

static inline __attribute__((always_inline))
void inval_dcache_by_addr( void *addr, size_t dsize )
{
	::isix_inval_dcache_by_addr(addr,dsize);
}

static inline __attribute__((always_inline))
void clean_dcache_by_addr( void *addr, size_t dsize )
{
	::isix_clean_dcache_by_addr(addr,dsize);
}

static inline __attribute__((always_inline))
void clean_inval_dcache_by_addr( void *addr, size_t dsize )
{
	::isix_clean_inval_dcache_by_addr(addr,dsize);
}

}
#endif








