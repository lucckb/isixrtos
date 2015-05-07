/*
 * =====================================================================================
 *
 *       Filename:  mpu.h
 *
 *    Description:  Memory protection unit header hile
 *
 *        Version:  1.0
 *        Created:  03.05.2015 20:52:25
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

//!Internal defs undefined at end of the file
#define MPU_REG32(offset) *((volatile uint32_t*)(0xE000ED90+offset))
#define MPU_TYPE MPU_REG32(0x0)
#define MPU_CTRL MPU_REG32(0x4)
#define MPU_RNR MPU_REG32(0x8)
#define MPU_RBAR MPU_REG32(0xC)
#define MPU_RASR MPU_REG32(0x10)
#define MPU_RBAR_A1 MPU_REG32(0x14)
#define MPU_RASR_A1 MPU_REG32(0x18)
#define MPU_RBAR_A2 MPU_REG32(0x1C)
#define MPU_RASR_A2 MPU_REG32(0x20)
#define MPU_RBAR_A3 MPU_REG32(0x24)
#define MPU_RASR_A3 MPU_REG32(0x28)
#define MPU_TYPE_DREGION_S 8
#define MPU_TYPE_DREGION_M 0x0000FF00  // Number of D Regions
#define MPU_CTRL_ENABLE    0x00000001  // MPU Enable
#define MPU_RASR_ENABLE (1U << 0)
#define MPU_RBAR_VALID  (1U << 4) 
#define MPU_RASR_TEX_M                                     (0x00380000)          /*  */
#define MPU_RBAR_ADDR_MASK      0xffffffe0 /* Bits N-31:  Region base addrese */

/** Read the DREGION field of the MPU type register and mask of
    the bits of interest to get the count of regions.
*/
static inline uint32_t mpu_get_region_count(void) 
{
     return ((MPU_TYPE & MPU_TYPE_DREGION_M) >> MPU_TYPE_DREGION_S);
}

//! Priviledged default map enable as background region
#define MPU_CONFIG_PRIV_DEFAULT     (1U<<2)
//! Enable MPU of executing NMI handler
#define MPU_CONFIG_HARDFLT_NMI      (1U<<1)
//! No special configuration
#define MPU_CONFIG_NONE             0U

 /**  Enables and configures the MPU for use.
  *  @see MPU_CONFIG_
  */
static inline void mpu_enable( uint32_t config )
{	
	MPU_CTRL = config | MPU_CTRL_ENABLE;
}

/** Disable the MPU engine */
static inline void mpu_disable(void)
{
	MPU_CTRL &= ~MPU_CTRL_ENABLE;
}

/** Enable specified region of the MPU 
 * @param[in] region number (standard CM3/4 values 0 to 7) 
 */
static inline void mpu_enable_region(uint32_t region) 
{
	MPU_RNR = region;
	MPU_RASR |= MPU_RASR_ENABLE;
}


/** Disable specified region of the MPU 
 * @param[in] region number (standard CM3/4 values 0 to 7) 
 */
static inline void mpu_disable_region(uint32_t region)
{
	MPU_RNR = region;
	MPU_RASR &= ~MPU_RASR_ENABLE;
}


//* Set region parameter definitions  */
// Region size
#define MPU_RGN_SIZE_32B            (4U << 1)
#define MPU_RGN_SIZE_64B            (5U << 1)
#define MPU_RGN_SIZE_128B           (6U << 1)
#define MPU_RGN_SIZE_256B           (7U << 1)
#define MPU_RGN_SIZE_512B           (8U << 1)

#define MPU_RGN_SIZE_1K             (9U << 1)
#define MPU_RGN_SIZE_2K             (10U << 1)
#define MPU_RGN_SIZE_4K             (11U << 1)
#define MPU_RGN_SIZE_8K             (12U << 1)
#define MPU_RGN_SIZE_16K            (13U << 1)
#define MPU_RGN_SIZE_32K            (14U << 1)
#define MPU_RGN_SIZE_64K            (15U << 1)
#define MPU_RGN_SIZE_128K           (16U << 1)
#define MPU_RGN_SIZE_256K           (17U << 1)
#define MPU_RGN_SIZE_512K           (18U << 1)

#define MPU_RGN_SIZE_1M             (19U << 1)
#define MPU_RGN_SIZE_2M             (20U << 1)
#define MPU_RGN_SIZE_4M             (21U << 1)
#define MPU_RGN_SIZE_8M             (22U << 1)
#define MPU_RGN_SIZE_16M            (23U << 1)
#define MPU_RGN_SIZE_32M            (24U << 1)
#define MPU_RGN_SIZE_64M            (25U << 1)
#define MPU_RGN_SIZE_128M           (26U << 1)
#define MPU_RGN_SIZE_256M           (27U << 1)
#define MPU_RGN_SIZE_512M           (28U << 1)

#define MPU_RGN_SIZE_1G             (29U << 1)
#define MPU_RGN_SIZE_2G             (30U << 1)
#define MPU_RGN_SIZE_4G             (31U << 1)

//Region permissions
#define MPU_RGN_PERM_NX         	(1U<<28)
#define MPU_RGN_PERM_PRV_NO_USR_NO  0x00000000U
#define MPU_RGN_PERM_PRV_RW_USR_NO  0x01000000U
#define MPU_RGN_PERM_PRV_RW_USR_RO  0x02000000U
#define MPU_RGN_PERM_PRV_RW_USR_RW  0x03000000U
#define MPU_RGN_PERM_PRV_RO_USR_NO  0x05000000U
#define MPU_RGN_PERM_PRV_RO_USR_RO  0x06000000U

// Subregion disable
#define MPU_SUB_RGN_DISABLE_0       0x00000100U
#define MPU_SUB_RGN_DISABLE_1       0x00000200U
#define MPU_SUB_RGN_DISABLE_2       0x00000400U
#define MPU_SUB_RGN_DISABLE_3       0x00000800U
#define MPU_SUB_RGN_DISABLE_4       0x00001000U
#define MPU_SUB_RGN_DISABLE_5       0x00002000U
#define MPU_SUB_RGN_DISABLE_6       0x00004000U
#define MPU_SUB_RGN_DISABLE_7       0x00008000U

//Flag for enable or disable region
#define MPU_RGN_ENABLE              1U
#define MPU_RGN_DISABLE             0U

//! Caching flags
#define MPU_RGN_B              (1U << 16) /* Bit 16: Bufferable */
#define MPU_RGN_C              (1U << 17) /* Bit 17: Cacheable */
#define MPU_RGN_S              (1U << 18) /* Bit 18: Shareable */
#define MPU_RGN_PERIPH MPU_RGN_S
#define MPU_RGN_MEMORY (MPU_RGN_B|MPU_RGN_C|MPU_RGN_S)

/** Setup region address register 
 * @param[in] region Input region number
 * @param[in] address Startup address
 * @param[in] flags Region flags
 *
 */
static inline void mpu_set_region( uint32_t region, uintptr_t addr, uint32_t flags )
{
	addr &= MPU_RBAR_ADDR_MASK;
	MPU_RBAR = addr | region | MPU_RBAR_VALID;
	MPU_RASR = flags;
}

//! Subregion shift
#define MPU_RASR_SRD_SHIFT      (8)  
#define MPU_RASR_SIZE_SHIFT     (1)  
#define MPU_RASR_SIZE_LOG2(n) ((n-1) << MPU_RASR_SIZE_SHIFT)

/**  Determine  the minimal sub-region set to span that memory region
 *	 @param[in] base Base address
 *	 @param[in] size requested size
 *	 @param[in] l2size Size for n2 regions
 	 @return the subregion mask which should be passed to set region with 
	  MPU_RASR_SZE_LOG2
 */
uint32_t mpu_subregion(uintptr_t base, size_t size, uint8_t l2size);

/** Get the pow2 region number that fill in it
 * @param[in] size Requested size
 * @return  the minimal sub-region set to span that memory region
 */
uint32_t mpu_log2region(size_t size);


/** Get the pow2 region number that is lower than one
 * @param[in] size Requested size
 * @return  the minimal sub-region set to span that memory region
 */
uint32_t mpu_log2region_lt(size_t size);



/** Setup the region with required size and disable the subregion to handle
 * required minimal memory layout 
 * @param[in] region Input region
 * @param[in] addr Input address
 * @param[in] len Input length
 * @param[in] flags MPU_RGN_PERM_ and MPU_RGN_ENABLE or MPU_RGN_DISABLE only
 */
static inline
void mpu_set_region_size( uint32_t region, uintptr_t addr, size_t len, uint32_t flags )
{
	uint32_t log2r = mpu_log2region(len);
	flags |= mpu_subregion(addr, len, log2r);
	flags |= MPU_RASR_SIZE_LOG2(log2r);
	mpu_set_region( region, addr, flags );
}

//Undefine  private data
#undef MPU_REG32
#undef MPU_TYPE 
#undef MPU_CTRL 
#undef MPU_RNR
#undef MPU_RBAR 
#undef MPU_RASR 
#undef MPU_RBAR_A1 
#undef MPU_RASR_A1 
#undef MPU_RBAR_A2 
#undef MPU_RASR_A2 
#undef MPU_RBAR_A3 
#undef MPU_RASR_A3 
#undef MPU_TYPE_DREGION_S 
#undef MPU_TYPE_DREGION_M 
#undef MPU_CTRL_ENABLE
#undef MPU_RASR_ENABLE
#undef MPU_RBAR_VALID  
#undef MPU_RASR_TEX_M
#undef MPU_RBAR_ADDR_MASK      


