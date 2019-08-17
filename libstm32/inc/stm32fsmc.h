#ifndef  stm32fsmc_INC
#define  stm32fsmc_INC

#include <stdbool.h>
#include <stdint.h>
#include <stm32lib.h>


#ifdef __cplusplus
namespace stm32 {
#endif



/* --------------------- FSMC registers bit mask ---------------------------- */
/* FSMC BCRx Mask */
#define BCR_MBKEN_SET          ((uint32_t)0x00000001)
#define BCR_MBKEN_RESET        ((uint32_t)0x000FFFFE)
#define BCR_FACCEN_SET         ((uint32_t)0x00000040)

/* FSMC PCRx Mask */
#define PCR_PBKEN_SET          ((uint32_t)0x00000004)
#define PCR_PBKEN_RESET        ((uint32_t)0x000FFFFB)
#define PCR_ECCEN_SET          ((uint32_t)0x00000040)
#define PCR_ECCEN_RESET        ((uint32_t)0x000FFFBF)
#define PCR_MEMORYTYPE_NAND    ((uint32_t)0x00000008)
 
/** @defgroup FSMC_Exported_Constants
  * @{
  */

/** @defgroup FSMC_NORSRAM_Bank 
  * @{
  */
#define FSMC_Bank1_NORSRAM1                      ((uint32_t)0x00000000)
#define FSMC_Bank1_NORSRAM2                      ((uint32_t)0x00000002)
#define FSMC_Bank1_NORSRAM3                      ((uint32_t)0x00000004)
#define FSMC_Bank1_NORSRAM4                      ((uint32_t)0x00000006)
/**
  * @}
  */

/** @defgroup FSMC_NAND_Bank 
  * @{
  */  
#define FSMC_Bank2_NAND                          ((uint32_t)0x00000010)
#define FSMC_Bank3_NAND                          ((uint32_t)0x00000100)
/**
  * @}
  */

/** @defgroup FSMC_PCCARD_Bank 
  * @{
  */    
#define FSMC_Bank4_PCCARD                        ((uint32_t)0x00001000)
/**
  * @}
  */


/** @defgroup FSMC_NOR_SRAM_Controller 
  * @{
  */

/** @defgroup FSMC_Data_Address_Bus_Multiplexing 
  * @{
  */

#define FSMC_DataAddressMux_Disable                ((uint32_t)0x00000000)
#define FSMC_DataAddressMux_Enable                 ((uint32_t)0x00000002)

/** @defgroup FSMC_Memory_Type 
  * @{
  */

#define FSMC_MemoryType_SRAM                     ((uint32_t)0x00000000)
#define FSMC_MemoryType_PSRAM                    ((uint32_t)0x00000004)
#define FSMC_MemoryType_NOR                      ((uint32_t)0x00000008)

/** @defgroup FSMC_Data_Width 
  * @{
  */
#define FSMC_MemoryDataWidth_8b                  ((uint32_t)0x00000000)
#define FSMC_MemoryDataWidth_16b                 ((uint32_t)0x00000010)

/** @defgroup FSMC_Burst_Access_Mode 
  * @{
  */

#define FSMC_BurstAccessMode_Disable             ((uint32_t)0x00000000) 
#define FSMC_BurstAccessMode_Enable              ((uint32_t)0x00000100)
/** @defgroup FSMC_AsynchronousWait 
  * @{
  */
#define FSMC_AsynchronousWait_Disable            ((uint32_t)0x00000000)
#define FSMC_AsynchronousWait_Enable             ((uint32_t)0x00008000)

/** @defgroup FSMC_Wait_Signal_Polarity 
  * @{
  */
#define FSMC_WaitSignalPolarity_Low              ((uint32_t)0x00000000)
#define FSMC_WaitSignalPolarity_High             ((uint32_t)0x00000200)

/** @defgroup FSMC_Wrap_Mode 
  * @{
  */
#define FSMC_WrapMode_Disable                    ((uint32_t)0x00000000)
#define FSMC_WrapMode_Enable                     ((uint32_t)0x00000400) 

/** @defgroup FSMC_Wait_Timing 
  * @{
  */
#define FSMC_WaitSignalActive_BeforeWaitState    ((uint32_t)0x00000000)
#define FSMC_WaitSignalActive_DuringWaitState    ((uint32_t)0x00000800) 

/** @defgroup FSMC_Write_Operation 
  * @{
  */
#define FSMC_WriteOperation_Disable                     ((uint32_t)0x00000000)
#define FSMC_WriteOperation_Enable                      ((uint32_t)0x00001000)
/**
  * @}
  */

/** @defgroup FSMC_Wait_Signal 
  * @{
  */
#define FSMC_WaitSignal_Disable                  ((uint32_t)0x00000000)
#define FSMC_WaitSignal_Enable                   ((uint32_t)0x00002000) 

/** @defgroup FSMC_Extended_Mode 
  * @{
  */
#define FSMC_ExtendedMode_Disable                ((uint32_t)0x00000000)
#define FSMC_ExtendedMode_Enable                 ((uint32_t)0x00004000)


/** @defgroup FSMC_Write_Burst 
  * @{
  */

#define FSMC_WriteBurst_Disable                  ((uint32_t)0x00000000)
#define FSMC_WriteBurst_Enable                   ((uint32_t)0x00080000) 

/** @defgroup FSMC_Address_Setup_Time 
  * @{
  */

/** @defgroup FSMC_Access_Mode 
  * @{
  */
#define FSMC_AccessMode_A                        ((uint32_t)0x00000000)
#define FSMC_AccessMode_B                        ((uint32_t)0x10000000) 
#define FSMC_AccessMode_C                        ((uint32_t)0x20000000)
#define FSMC_AccessMode_D                        ((uint32_t)0x30000000)

/**
  * @}
  */
  
/** @defgroup FSMC_NAND_PCCARD_Controller 
  * @{
  */

/** @defgroup FSMC_Wait_feature 
  * @{
  */
#define FSMC_Waitfeature_Disable                 ((uint32_t)0x00000000)
#define FSMC_Waitfeature_Enable                  ((uint32_t)0x00000002)


/** @defgroup FSMC_ECC 
  * @{
  */
#define FSMC_ECC_Disable                         ((uint32_t)0x00000000)
#define FSMC_ECC_Enable                          ((uint32_t)0x00000040)

/** @defgroup FSMC_ECC_Page_Size 
  * @{
  */
#define FSMC_ECCPageSize_256Bytes                ((uint32_t)0x00000000)
#define FSMC_ECCPageSize_512Bytes                ((uint32_t)0x00020000)
#define FSMC_ECCPageSize_1024Bytes               ((uint32_t)0x00040000)
#define FSMC_ECCPageSize_2048Bytes               ((uint32_t)0x00060000)
#define FSMC_ECCPageSize_4096Bytes               ((uint32_t)0x00080000)
#define FSMC_ECCPageSize_8192Bytes               ((uint32_t)0x000A0000)

/** @defgroup FSMC_Interrupt_sources 
  * @{
  */
#define FSMC_IT_RisingEdge                       ((uint32_t)0x00000008)
#define FSMC_IT_Level                            ((uint32_t)0x00000010)
#define FSMC_IT_FallingEdge                      ((uint32_t)0x00000020)


/** @defgroup FSMC_Flags 
  * @{
  */
#define FSMC_FLAG_RisingEdge                     ((uint32_t)0x00000001)
#define FSMC_FLAG_Level                          ((uint32_t)0x00000002)
#define FSMC_FLAG_FallingEdge                    ((uint32_t)0x00000004)
#define FSMC_FLAG_FEMPT                          ((uint32_t)0x00000040)


 
struct fsmc_timing {
	uint8_t address_setup_time;
	uint8_t address_hold_time;
	uint8_t data_setup_time;
	uint8_t bus_turn_arround_duration;
	uint8_t clk_div;
	uint8_t data_latency;
	uint32_t access_mode;
};


static inline uint32_t _fsmc_nor_timing_to_reg( const struct fsmc_timing* t )
{
	return ((uint32_t)t->address_setup_time&0xf) |
		   (((uint32_t)t->address_hold_time&0xf) <<4) |
		   ((uint32_t)t->data_setup_time <<8) |
		   (((uint32_t)t->bus_turn_arround_duration&0xf) <<16) |
		   (((uint32_t)t->clk_div&0xf) <<20) |
		   (((uint32_t)t->data_latency&0xf) <<24 ) |
		   ((uint32_t)t->access_mode );

}

static inline uint32_t _fsmc_nor_timing_to_wrreg( const struct fsmc_timing* t )
{
	return ((uint32_t)t->address_setup_time&0xf) |
		   (((uint32_t)t->address_hold_time&0xf) <<4) |
		   ((uint32_t)t->data_setup_time <<8 ) |
		   (((uint32_t)t->clk_div&0xf) <<20) |
		   (((uint32_t)t->data_latency&0xf) <<24) |
		   ((uint32_t)t->access_mode );

}




/**
  * @brief  Initializes the FSMC NOR/SRAM Banks according to the specified
  *         parameters in the FSMC_NORSRAMInitStruct.
  */
static inline void fsmc_nor_setup( uint32_t bank, uint32_t mode,
		const struct fsmc_timing* tim, const struct fsmc_timing* ext_tim )
{
	FSMC_Bank1->BTCR[bank] = mode;
	if( mode & FSMC_MemoryType_NOR ) {
		FSMC_Bank1->BTCR[bank] |= (uint32_t)BCR_FACCEN_SET;
	}
	FSMC_Bank1->BTCR[bank+1] = _fsmc_nor_timing_to_reg(tim);
	if( mode & FSMC_ExtendedMode_Enable && ext_tim ) {
		FSMC_Bank1E->BWTR[bank] =  _fsmc_nor_timing_to_wrreg(ext_tim);
	} else {
		FSMC_Bank1E->BWTR[bank] =  0xffffffff;
	}
}

/**
  * @brief  Enables or disables the specified NOR/SRAM Memory Bank.
  * @param  FSMC_Bank: specifies the FSMC Bank to be used
  *          This parameter can be one of the following values:
  *            @arg FSMC_Bank1_NORSRAM1: FSMC Bank1 NOR/SRAM1
  *            @arg FSMC_Bank1_NORSRAM2: FSMC Bank1 NOR/SRAM2
  *            @arg FSMC_Bank1_NORSRAM3: FSMC Bank1 NOR/SRAM3
  *            @arg FSMC_Bank1_NORSRAM4: FSMC Bank1 NOR/SRAM4
  */
static inline void fsmc_nor_cmd( uint32_t bank, bool en )
{
	if( en ) {
		FSMC_Bank1->BTCR[bank] |= BCR_MBKEN_SET;
	} else {
		FSMC_Bank1->BTCR[bank] &= BCR_MBKEN_RESET;
	}
}

 
#undef BCR_MBKEN_SET
#undef BCR_MBKEN_RESET
#undef BCR_FACCEN_SET

/* PCRx Mask */
#undef PCR_PBKEN_SET
#undef PCR_PBKEN_RESET
#undef PCR_ECCEN_SET
#undef PCR_ECCEN_RESET
#undef PCR_MEMORYTYPE_NAND

#ifdef __cplusplus
}
#endif
#endif   /* ----- #ifndef stm32fsmc_INC  ----- */
