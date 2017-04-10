/*
 * =====================================================================================
 *
 *       Filename:  mpu_v7m.c
 *
 *    Description:  MPU memory management function helepr
 *
 *        Version:  1.0
 *        Created:  03.05.2015 22:59:05
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include "mpu.h"

// 
static const uint8_t g_ms_regionmask[9] = 
{
	0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80, 0x00
};

static const uint8_t g_ls_regionmask[9] = 
{
	0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff
};

//! Determine the minimal sub-region to be disabled
static inline uint32_t mpu_subregion_ls(size_t offset, uint8_t l2size)
{
	unsigned int nsrs;
	uint32_t     aoffset;
	uint32_t     mask;
	/* Examples with l2size = 12:
	 *
	 *         Shifted Adjusted        Number      Sub-Region
	 * Offset  Mask    Offset    Shift Sub-Regions Bitset
	 * 0x0000  0x01ff  0x0000    9     8           0x00
	 * 0x0400  0x01ff  0x0400    9     6           0x03
	 * 0x02c0  0x01ff  0x0200    9     7           0x01
	 */
	if (l2size < 32)
	{
		mask  = ((1 << l2size)-1) >> 3; /* Shifted mask */
	}
	/* The 4Gb region size is a special case */
	else
	{
		/* NOTE: There is no way to represent a 4Gb region size in the 32-bit
		 * input.
		 */

		mask = 0x1fffffff;           /* Shifted mask */
	}

	aoffset = offset & ~mask;        /* Adjusted offset */
	nsrs    = aoffset >> (l2size-3); /* Number of subregions */
	return g_ls_regionmask[nsrs];
}

//! Determine end of region to be disabled
static inline uint32_t mpu_subregion_ms(size_t size, uint8_t l2size)
{
  unsigned int nsrs;
  uint32_t     asize;
  uint32_t     mask;

  /* Examples with l2size = 12:
   *
   *         Shifted Adjusted        Number      Sub-Region
   * Size    Mask    Size      Shift Sub-Regions Bitset
   * 0x1000  0x01ff  0x1000    9     8           0x00
   * 0x0c00  0x01ff  0x0c00    9     6           0xc0
   * 0x0c40  0x01ff  0x0e00    9     7           0x80
   */

  if (l2size < 32)
    {
      mask  = ((1 << l2size)-1) >> 3; /* Shifted mask */
    }

  /* The 4Gb region size is a special case */

  else
    {
      /* NOTE: There is no way to represent a 4Gb region size in the 32-bit
       * input.
       */

      mask = 0x1fffffff;         /* Shifted mask */
    }

  asize = (size + mask) & ~mask; /* Adjusted size */
  nsrs  = asize >> (l2size-3);   /* Number of subregions */
  return g_ms_regionmask[nsrs];
}

/**  Determine  the minimal sub-region set to span that memory region
 *	 @param[in] base Base address
 *	 @param[in] size requested size
 *	 @param[in] l2size Size for n2 regions
 */
uint32_t mpu_subregion(uintptr_t base, size_t size, uint8_t l2size)
{

	uint32_t mask;
	size_t offset;
	uint32_t ret;
	/* For region sizes of 32, 64, and 128 bytes, the effect of setting
	 * one or more bits of the SRD field to 1 is UNPREDICTABLE.
	 */
	if (l2size < 8)
	{
		return 0;
	}
	/* Calculate the offset of the base address into the aligned region. */
	mask   = (1 << l2size) - 1;
	offset = base & mask;
	/* Calculate the mask need to handle disabled subregions at the end of the
	 * region
	 */
	ret = mpu_subregion_ms(size + offset, l2size);
	/* Then OR in the mask need to handle disabled at the beginning
	 * of the region.
	 */
	ret |= mpu_subregion_ls(offset, l2size);
	return ret;
}

/** Get the pow2 region number
 * @param[in] size Requested size
 * @return  the minimal sub-region set to span that memory region
 */
uint32_t mpu_log2region(size_t size)
{
	/* The minimum permitted region size is 32 bytes (log2(32) = 5. */
	uint32_t l2size;
	for (l2size = 5; l2size < 32 && size > (1U << l2size); l2size++);
	return l2size;
}


/** Get the pow2 region number that is lower than one
 * @param[in] size Requested size
 * @return  the minimal sub-region set to span that memory region
 */
uint32_t mpu_log2region_lt(size_t size)
{
	uint32_t l2size = mpu_log2region(size);
	if (l2size > 4U && size < (1U << l2size))
	{
		l2size--;
	}
	return l2size;
}
