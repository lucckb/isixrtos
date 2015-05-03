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
#define MPU_RASR_B              (1 << 16) /* Bit 16: Bufferable */
#define MPU_RASR_C              (1 << 17) /* Bit 17: Cacheable */
#define MPU_RASR_S              (1 << 18) /* Bit 18: Shareable */
#define MPU_RASR_TEX_M                                     (0x00380000)          /*  */

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
#define MPU_RGN_SIZE_32B            (4 << 1)
#define MPU_RGN_SIZE_64B            (5 << 1)
#define MPU_RGN_SIZE_128B           (6 << 1)
#define MPU_RGN_SIZE_256B           (7 << 1)
#define MPU_RGN_SIZE_512B           (8 << 1)

#define MPU_RGN_SIZE_1K             (9 << 1)
#define MPU_RGN_SIZE_2K             (10 << 1)
#define MPU_RGN_SIZE_4K             (11 << 1)
#define MPU_RGN_SIZE_8K             (12 << 1)
#define MPU_RGN_SIZE_16K            (13 << 1)
#define MPU_RGN_SIZE_32K            (14 << 1)
#define MPU_RGN_SIZE_64K            (15 << 1)
#define MPU_RGN_SIZE_128K           (16 << 1)
#define MPU_RGN_SIZE_256K           (17 << 1)
#define MPU_RGN_SIZE_512K           (18 << 1)

#define MPU_RGN_SIZE_1M             (19 << 1)
#define MPU_RGN_SIZE_2M             (20 << 1)
#define MPU_RGN_SIZE_4M             (21 << 1)
#define MPU_RGN_SIZE_8M             (22 << 1)
#define MPU_RGN_SIZE_16M            (23 << 1)
#define MPU_RGN_SIZE_32M            (24 << 1)
#define MPU_RGN_SIZE_64M            (25 << 1)
#define MPU_RGN_SIZE_128M           (26 << 1)
#define MPU_RGN_SIZE_256M           (27 << 1)
#define MPU_RGN_SIZE_512M           (28 << 1)

#define MPU_RGN_SIZE_1G             (29 << 1)
#define MPU_RGN_SIZE_2G             (30 << 1)
#define MPU_RGN_SIZE_4G             (31 << 1)

//Region permissions
#define MPU_RGN_PERM_EXEC           0x00000000
#define MPU_RGN_PERM_NOEXEC         0x10000000
#define MPU_RGN_PERM_PRV_NO_USR_NO  0x00000000
#define MPU_RGN_PERM_PRV_RW_USR_NO  0x01000000
#define MPU_RGN_PERM_PRV_RW_USR_RO  0x02000000
#define MPU_RGN_PERM_PRV_RW_USR_RW  0x03000000
#define MPU_RGN_PERM_PRV_RO_USR_NO  0x05000000
#define MPU_RGN_PERM_PRV_RO_USR_RO  0x06000000

// Subregion disable
#define MPU_SUB_RGN_DISABLE_0       0x00000100
#define MPU_SUB_RGN_DISABLE_1       0x00000200
#define MPU_SUB_RGN_DISABLE_2       0x00000400
#define MPU_SUB_RGN_DISABLE_3       0x00000800
#define MPU_SUB_RGN_DISABLE_4       0x00001000
#define MPU_SUB_RGN_DISABLE_5       0x00002000
#define MPU_SUB_RGN_DISABLE_6       0x00004000
#define MPU_SUB_RGN_DISABLE_7       0x00008000

//Flag for enable or disable region
#define MPU_RGN_ENABLE              1
#define MPU_RGN_DISABLE             0


/** Setup region address register 
 * @param[in] region Input region number
 * @param[in] address Startup address
 * @param[in] flags Region flags
 *
 */
static inline void mpu_set_region( uint32_t region, uintptr_t addr, uint32_t flags )
{
	MPU_RBAR = addr | region | MPU_RBAR_VALID;
	MPU_RASR = (flags & ~(MPU_RASR_TEX_M | MPU_RASR_C)) | MPU_RASR_S
            | MPU_RASR_B;
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

/** Get the pow2 region number
 * @param[in] size Requested size
 * @return  the minimal sub-region set to span that memory region
 */
uint32_t mpu_log2region(size_t size);

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
#undef MPU_RASR_B
#undef MPU_RASR_C
#undef MPU_RASR_S
#undef MPU_RASR_TEX_M
