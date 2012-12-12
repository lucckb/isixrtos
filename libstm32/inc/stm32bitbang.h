/*
 * stm32bitbang.h
 *
 *  Created on: 18-07-2012
 *      Author: lucck
 */

#ifndef STM32BITBANG_H_
#define STM32BITBANG_H_
/*----------------------------------------------------------*/
#ifdef __cplusplus
namespace stm32 {
#endif
/*----------------------------------------------------------*/

//! Memory access in bit band region
enum {
	RAM_BASE =      0x20000000,
	RAM_BB_BASE =    0x22000000
};

static inline void resetBit_BB(void *VarAddr, unsigned BitNumber)
{
     (*(vu32 *) (RAM_BB_BASE | ((((u32)VarAddr) - RAM_BASE) << 5) | ((BitNumber) << 2)) = 0);
}

static inline void setBit_BB(void *VarAddr, unsigned BitNumber)
{
    (*(vu32 *) (RAM_BB_BASE | ((((u32)VarAddr) - RAM_BASE) << 5) | ((BitNumber) << 2)) = 1);
}

static inline bool getBit_BB(void *VarAddr, unsigned BitNumber)
{
     return (*(vu32 *) (RAM_BB_BASE | ((((u32)VarAddr) - RAM_BASE) << 5) | ((BitNumber) << 2)));
}
static inline unsigned getBitsAll_BB(void *VarAddr )
{
	return (*(vu32 *)(VarAddr));
}
static inline void resetBitsAll_BB(void *VarAddr )
{
	(*(vu32 *)(VarAddr)) = 0;
}
static inline void setBitsAll_BB(void *VarAddr, unsigned val )
{
	(*(vu32 *)(VarAddr)) = val;
}
/*----------------------------------------------------------*/
#ifdef __cplusplus
}
#endif
/*----------------------------------------------------------*/
#endif /* STM32BITBANG_H_ */
