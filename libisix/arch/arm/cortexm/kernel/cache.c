/*
 * =====================================================================================
 *
 *       Filename:  cache.c
 *
 *    Description:  Cache maintenance functions implementation
 *
 *        Version:  1.0
 *        Created:  23.04.2017 21:13:16
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#include <isix/arch/cache.h>
#include <isix/cortexm/scb_regs.h>


static inline __attribute__((always_inline)) void dsb(void)
{
  asm volatile ("dsb 0xF":::"memory");
}

static inline __attribute__((always_inline)) void isb(void)
{
  asm volatile ("isb 0xF":::"memory");
}


#define CCSIDR_WAYS(x)         (((x) & SCB_CCSIDR_ASSOCIATIVITY) >> SCB_CCSIDR_ASSOCIATIVITY_BIT)
#define CCSIDR_SETS(x)         (((x) & SCB_CCSIDR_NUMSETS      ) >> SCB_CCSIDR_NUMSETS_BIT      )




/** Enable or disable icache
 * @parm[in] yes Enable or disable
 */
void isix_icache_enable( bool yes )
{
#if _ISIX_CACHE_PRESENT_ == 1
	dsb();
	isb();
    SCB_ICIALLU = 0UL;                     /* invalidate I-Cache */
    if(yes) SCB_CCR |=  SCB_CCR_ICACHE;  /* enable I-Cache */
	else	SCB_CCR &=  ~SCB_CCR_ICACHE;  /* enable I-Cache */
	dsb();
	isb();
#else
	(void)yes;
#endif
}


/** Invalidate instruction cache  */
void isix_inval_icache( void )
{
#if _ISIX_CACHE_PRESENT_ == 1
	dsb();
	isb();
    SCB_ICIALLU = 0UL;
	dsb();
	isb();
#endif
}




#if _ISIX_CACHE_PRESENT_ == 1

static inline void dcache_enable(void)
{
	uint32_t ccsidr;
	uint32_t sets;
	uint32_t ways;

	SCB_CSSELR = (0U << 1U) | 0U;          /* Level 1 data cache */
	dsb();

	ccsidr = SCB_CCSIDR;

	/* invalidate D-Cache */
	sets = (uint32_t)(CCSIDR_SETS(ccsidr));
	do {
		ways = (uint32_t)(CCSIDR_WAYS(ccsidr));
		do {
			SCB_DCISW = (((sets << SCB_DCISW_SET_BIT) & SCB_DCISW_SET) |
					((ways << SCB_DCISW_WAY_BIT) & SCB_DCISW_WAY)  );
		} while (ways--);
	} while(sets--);
	dsb();

	SCB_CCR |=  (uint32_t)SCB_CCR_DCACHE;  /* enable D-Cache */

	dsb();
	isb();
}

static inline void dcache_disable(void)
{

    uint32_t ccsidr;
    uint32_t sets;
    uint32_t ways;

    SCB_CSSELR = (0U << 1U) | 0U;          /* Level 1 data cache */
    dsb();

    ccsidr = SCB_CCSIDR;

    SCB_CCR &= ~(uint32_t)SCB_CCR_DCACHE;  /* disable D-Cache */

                                            /* clean & invalidate D-Cache */
    sets = (uint32_t)(CCSIDR_SETS(ccsidr));
    do {
      ways = (uint32_t)(CCSIDR_WAYS(ccsidr));
      do {
        SCB_DCCISW = (((sets << SCB_DCCISW_SET_BIT) & SCB_DCCISW_SET) |
                       ((ways << SCB_DCCISW_WAY_BIT) & SCB_DCCISW_WAY)  );
      } while (ways--);
    } while(sets--);

    dsb();
    isb();
}
#endif


/** Enable or disable dcache
 * @parm[in] yes Enable or disable
 */
void isix_dcache_enable( bool yes )
{

#if _ISIX_CACHE_PRESENT_ == 1
	if( yes ) dcache_enable();
	else dcache_disable();
#else
	(void)yes;
#endif
}

/** Invalidate data cache  */
void isix_inval_dcache( void )
{
#if _ISIX_CACHE_PRESENT_ == 1
    uint32_t ccsidr;
    uint32_t sets;
    uint32_t ways;

    SCB_CSSELR = (0U << 1U) | 0U;          /* Level 1 data cache */
    dsb();

    ccsidr = SCB_CCSIDR;

                                            /* invalidate D-Cache */
    sets = (uint32_t)(CCSIDR_SETS(ccsidr));
    do {
      ways = (uint32_t)(CCSIDR_WAYS(ccsidr));
      do {
        SCB_DCISW = (((sets << SCB_DCISW_SET_BIT) & SCB_DCISW_SET) |
                      ((ways << SCB_DCISW_WAY_BIT) & SCB_DCISW_WAY)  );
      } while (ways--);
    } while(sets--);

    dsb();
    isb();
#endif
}


/** Clean instruction cache  */
void isix_clean_dcache( void )
{
#if _ISIX_CACHE_PRESENT_ == 1
    uint32_t ccsidr;
    uint32_t sets;
    uint32_t ways;

    SCB_CSSELR = (0U << 1U) | 0U;          /* Level 1 data cache */
    dsb();

    ccsidr = SCB_CCSIDR;

                                            /* clean D-Cache */
    sets = (uint32_t)(CCSIDR_SETS(ccsidr));
    do {
      ways = (uint32_t)(CCSIDR_WAYS(ccsidr));
      do {
        SCB_DCCSW = (((sets << SCB_DCCSW_SET_BIT) & SCB_DCCSW_SET) |
                      ((ways << SCB_DCCSW_WAY_BIT) & SCB_DCCSW_WAY)  );
      } while (ways--);
    } while(sets--);

    dsb();
    isb();
#endif
}

/** Clean and invalidate data cache  */
void isix_clean_inval_dcache( void )
{

#if _ISIX_CACHE_PRESENT_ == 1
    uint32_t ccsidr;
    uint32_t sets;
    uint32_t ways;

    SCB_CSSELR = (0U << 1U) | 0U;          /* Level 1 data cache */
    dsb();

    ccsidr = SCB_CCSIDR;

                                            /* clean & invalidate D-Cache */
    sets = (uint32_t)(CCSIDR_SETS(ccsidr));
    do {
      ways = (uint32_t)(CCSIDR_WAYS(ccsidr));
      do {
        SCB_DCCISW = (((sets << SCB_DCCISW_SET_BIT) & SCB_DCCISW_SET) |
                       ((ways << SCB_DCCISW_WAY_BIT) & SCB_DCCISW_WAY)  );
      } while (ways--);
    } while(sets--);

    dsb();
    isb();
#endif
}

/** Invalidate data cache in selected range
 * @param[in] addr Input starting address
 * @param[in] dsize Area size
 */
void isix_inval_dcache_by_addr( void *addr, size_t dsize )
{
#if _ISIX_CACHE_PRESENT_ == 1
	int32_t op_size = dsize;
	uint32_t op_addr = (uint32_t)addr;
	/* in Cortex-M7 size of cache line is fixed to 8 words (32 bytes) */
	const int32_t linesize = 32U;

	dsb();

	while (op_size > 0) {
		SCB_DCIMVAC = op_addr;
		op_addr += linesize;
		op_size -= linesize;
	}

	dsb();
	isb();
#else
	(void)addr;
	(void)dsize;
#endif
}

/** Clean data cache in selected range
 * @param[in] addr Input starting address
 * @param[in] dsize Area size
 */
void isix_clean_dcache_by_addr( void *addr, size_t dsize )
{
#if _ISIX_CACHE_PRESENT_ == 1
	int32_t op_size = dsize;
	uint32_t op_addr = (uint32_t) addr;
	/* in Cortex-M7 size of cache line is fixed to 8 words (32 bytes) */
	const int32_t linesize = 32U;

	dsb();

	while (op_size > 0) {
		SCB_DCCMVAC = op_addr;
		op_addr += linesize;
		op_size -= linesize;
	}

	dsb();
	isb();
#else
	(void)addr;
	(void)dsize;
#endif
}

/** Clean and invalidate data cache in selected range
 * @param[in] addr Input starting address
 * @param[in] dsize Area size
 */
void isix_clean_inval_dcache_by_addr( void *addr, size_t dsize )
{
#if _ISIX_CACHE_PRESENT_ == 1
	int32_t op_size = dsize;
	uint32_t op_addr = (uint32_t) addr;
	/* in Cortex-M7 size of cache line is fixed to 8 words (32 bytes) */
	const int32_t linesize = 32U;

	dsb();

	while (op_size > 0) {
		SCB_DCCIMVAC = op_addr;
		op_addr += linesize;
		op_size -= linesize;
	}
	dsb();
	dsb();
#else
	(void)addr;
	(void)dsize;
#endif
}
