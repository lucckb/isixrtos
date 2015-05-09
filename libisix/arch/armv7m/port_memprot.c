/*
 * =====================================================================================
 *
 *       Filename:  port_memprot.c
 *
 *    Description:  Port memory protection implementation
 *
 *        Version:  1.0
 *        Created:  06.05.2015 19:52:30
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */


#include <isix/config.h>
#include "port_memprot.h"
#include <arm-v7m/mpu.h>
#include <isix/prv/printk.h>
#define _ISIX_KERNEL_CORE_
#include <isix/prv/scheduler.h>

#ifndef ISIX_CONFIG_MEMORY_PROTECTION_MODEL 
#error ISIX_CONFIG_MEMORY_PROTECTION_MODEL not defined
#endif

#if ISIX_CONFIG_MEMORY_PROTECTION_MODEL == ISIX_MPROT_LITE
static void setup_regions()
{
	mpu_set_region( 0, 0x20000000, 
			MPU_RGN_PERM_PRV_RW_USR_RW|
			MPU_RGN_PERM_NX|
			MPU_RGN_MEMORY|
			MPU_RGN_SIZE_64M );
	//!Protect region of the code
	mpu_set_region( 1, 0, 
			MPU_RGN_PERM_PRV_NO_USR_NO|
			MPU_RGN_PERM_NX|
			MPU_RGN_SIZE_2M );

	mpu_enable_region(0);
	mpu_enable_region(1);
}

#elif ISIX_CONFIG_MEMORY_PROTECTION_MODEL == ISIX_MPROT_FULL
#error ISIX_MPROT_FULL not supported yet
static void setup_regions()
{
}
#endif /* ISIX_CONFIG_MEMORY_PROTECTION_MODEL */

#if ISIX_CONFIG_MEMORY_PROTECTION_MODEL > 0

/** Function initialize default memory protection layout just before run */
void port_memory_protection_set_default_map(void)
{
	//If mpu not present don't touch memory map
	if( mpu_get_region_count() == 0 ) {
		pr_err("MPU device not present" );
		return;
	}
	//! Setup initial region maps
	setup_regions();
	//Enable MPU using default memory map in system mode
	mpu_enable( MPU_CONFIG_PRIV_DEFAULT );
}


/**  Set electric fence on the selected address
 *   this function is used by rtos to protect general heap
 *   memory region  it must be 32 byte aligned 
 *   @param[in] addr Set address 
 */
#define EFENCE_RGN_SIZE ISIX_MEMORY_PROTECTION_EFENCE_SIZE
void port_memory_protection_set_efence( uintptr_t estack ) 
{
#ifndef ISIX_CONFIG_STACK_GROWTH
	estack -= EFENCE_RGN_SIZE;
#endif
	// Fence region must be inside of alloated space
	if( estack & (EFENCE_RGN_SIZE-1) ) {
#ifdef ISIX_CONFIG_STACK_GROWTH
		estack += EFENCE_RGN_SIZE;
#else
		estack -= EFENCE_RGN_SIZE;
#endif
	}
	estack -= estack&(EFENCE_RGN_SIZE-1);
	int efregion = mpu_get_region_count();
	if( efregion == 0 ) {
		return;
	} else {
		--efregion;
	}
	asm volatile( "cpsid i\n" );
	//Disable the region first
	mpu_disable_region( efregion );
	//Make sure that was apply
	asm volatile( 
		"dsb\n"
		"isb\n" 
	);
	//Setup the region again
	mpu_set_region( efregion, estack, 
		MPU_RGN_PERM_PRV_NO_USR_NO |
		MPU_RGN_PERM_NX |
		MPU_RGN_SIZE_32B |
		MPU_RGN_ENABLE
	);
	//Make sure that was apply
	asm volatile( 
		"dsb\n"
		"isb\n"
		"cpsie i\n"
	);
}

//! Clear the memory protection efence
void port_memory_protection_reset_efence(void)
{
	int efregion = mpu_get_region_count();
	if( efregion == 0 ) {
		return;
	} else {
		--efregion;
	}
	asm volatile( "cpsid i\n" );
	//Disable the region first
	mpu_disable_region( efregion );
	//Make sure that was apply
	asm volatile( 
		"dsb\n"
		"isb\n" 
		"cpsie i\n"
	);
}

#endif /* ISIX_CONFIG_MEMORY_PROTECTION_MODEL  */



