/*
 * stm32dma.h
 *
 *  Created on: 21-07-2012
 *      Author: lucck
 */
/* ---------------------------------------------------------------------------- */
#ifndef STM32F4DMA_H_
#define STM32F4DMA_H_
/* ---------------------------------------------------------------------------- */
#include <stm32lib.h>
#if defined(STM32MCU_MAJOR_TYPE_F4)
#include "stm32f4xx_dma.h"
#elif defined(STM32MCU_MAJOR_TYPE_F2)
#include "../stm32f2x/stm32f2xx_dma.h"
#else
#error Selected MCU type is invalid
#endif

/* ---------------------------------------------------------------------------- */
#ifdef __cplusplus
namespace stm32 {
#endif

/* ---------------------------------------------------------------------------- */
#define TRANSFER_IT_ENABLE_MASK (uint32_t)(DMA_SxCR_TCIE | DMA_SxCR_HTIE | \
                                           DMA_SxCR_TEIE | DMA_SxCR_DMEIE)

#define DMA_Stream0_IT_MASK     (uint32_t)(DMA_LISR_FEIF0 | DMA_LISR_DMEIF0 | \
                                           DMA_LISR_TEIF0 | DMA_LISR_HTIF0 | \
                                           DMA_LISR_TCIF0)

#define DMA_Stream1_IT_MASK     (uint32_t)(DMA_Stream0_IT_MASK << 6)
#define DMA_Stream2_IT_MASK     (uint32_t)(DMA_Stream0_IT_MASK << 16)
#define DMA_Stream3_IT_MASK     (uint32_t)(DMA_Stream0_IT_MASK << 22)
#define DMA_Stream4_IT_MASK     (uint32_t)(DMA_Stream0_IT_MASK | (uint32_t)0x20000000)
#define DMA_Stream5_IT_MASK     (uint32_t)(DMA_Stream1_IT_MASK | (uint32_t)0x20000000)
#define DMA_Stream6_IT_MASK     (uint32_t)(DMA_Stream2_IT_MASK | (uint32_t)0x20000000)
#define DMA_Stream7_IT_MASK     (uint32_t)(DMA_Stream3_IT_MASK | (uint32_t)0x20000000)
#define TRANSFER_IT_MASK        (uint32_t)0x0F3C0F3C
#define HIGH_ISR_MASK           (uint32_t)0x20000000
#define RESERVED_MASK           (uint32_t)0x0F7D0F7D


/* ---------------------------------------------------------------------------- */
/**
  * @brief  Initializes the DMAy Streamx according to the specified parameters in
  *         the DMA_InitStruct structure.
  * @note   Before calling this function, it is recommended to check that the Stream
  *         is actually disabled using the function DMA_GetCmdStatus().
  * @param  DMAy_Streamx: where y can be 1 or 2 to select the DMA and x can be 0
  *         to 7 to select the DMA Stream.
  * @param  DMA_InitStruct: pointer to a DMA_InitTypeDef structure that contains
  *         the configuration information for the specified DMA Stream.
  * @retval None
  */
static inline void dma_init(DMA_Stream_TypeDef* DMAy_Streamx,
		uint32_t config, uint32_t fifo_config, uint32_t buffer_size,
		volatile void *periph, volatile void *memory0base )
{
  uint32_t tmpreg = 0;


  /*------------------------- DMAy Streamx CR Configuration ------------------*/
  /* Get the DMAy_Streamx CR value */
  tmpreg = DMAy_Streamx->CR;

  /* Clear CHSEL, MBURST, PBURST, PL, MSIZE, PSIZE, MINC, PINC, CIRC and DIR bits */
  tmpreg &= ((uint32_t)~(DMA_SxCR_CHSEL | DMA_SxCR_MBURST | DMA_SxCR_PBURST | \
                         DMA_SxCR_PL | DMA_SxCR_MSIZE | DMA_SxCR_PSIZE | \
                         DMA_SxCR_MINC | DMA_SxCR_PINC | DMA_SxCR_CIRC | \
                         DMA_SxCR_DIR));

  /* Configure DMAy Streamx: */
  /* Set CHSEL bits according to DMA_CHSEL value */
  /* Set DIR bits according to DMA_DIR value */
  /* Set PINC bit according to DMA_PeripheralInc value */
  /* Set MINC bit according to DMA_MemoryInc value */
  /* Set PSIZE bits according to DMA_PeripheralDataSize value */
  /* Set MSIZE bits according to DMA_MemoryDataSize value */
  /* Set CIRC bit according to DMA_Mode value */
  /* Set PL bits according to DMA_Priority value */
  /* Set MBURST bits according to DMA_MemoryBurst value */
  /* Set PBURST bits according to DMA_PeripheralBurst value */
  tmpreg |= config;

  /* Write to DMAy Streamx CR register */
  DMAy_Streamx->CR = tmpreg;

  /*------------------------- DMAy Streamx FCR Configuration -----------------*/
  /* Get the DMAy_Streamx FCR value */
  tmpreg = DMAy_Streamx->FCR;

  /* Clear DMDIS and FTH bits */
  tmpreg &= (uint32_t)~(DMA_SxFCR_DMDIS | DMA_SxFCR_FTH);

  /* Configure DMAy Streamx FIFO:
    Set DMDIS bits according to DMA_FIFOMode value
    Set FTH bits according to DMA_FIFOThreshold value */
  tmpreg |= fifo_config;

  /* Write to DMAy Streamx CR */
  DMAy_Streamx->FCR = tmpreg;

  /*------------------------- DMAy Streamx NDTR Configuration ----------------*/
  /* Write to DMAy Streamx NDTR register */
  DMAy_Streamx->NDTR =buffer_size;

  /*------------------------- DMAy Streamx PAR Configuration -----------------*/
  /* Write to DMAy Streamx PAR */
  DMAy_Streamx->PAR = (uint32_t)periph;

  /*------------------------- DMAy Streamx M0AR Configuration ----------------*/
  /* Write to DMAy Streamx M0AR */
  DMAy_Streamx->M0AR = (uint32_t)memory0base;
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Enables or disables the specified DMAy Streamx.
  * @param  DMAy_Streamx: where y can be 1 or 2 to select the DMA and x can be 0
  *         to 7 to select the DMA Stream.
  * @param  NewState: new state of the DMAy Streamx.
  *          This parameter can be: ENABLE or DISABLE.
  *
  * @note  This function may be used to perform Pause-Resume operation. When a
  *        transfer is ongoing, calling this function to disable the Stream will
  *        cause the transfer to be paused. All configuration registers and the
  *        number of remaining data will be preserved. When calling again this
  *        function to re-enable the Stream, the transfer will be resumed from
  *        the point where it was paused.
  *
  * @note  After configuring the DMA Stream (DMA_Init() function) and enabling the
  *        stream, it is recommended to check (or wait until) the DMA Stream is
  *        effectively enabled. A Stream may remain disabled if a configuration
  *        parameter is wrong.
  *        After disabling a DMA Stream, it is also recommended to check (or wait
  *        until) the DMA Stream is effectively disabled. If a Stream is disabled
  *        while a data transfer is ongoing, the current data will be transferred
  *        and the Stream will be effectively disabled only after the transfer of
  *        this single data is finished.
  *
  * @retval None
  */
static inline void dma_cmd(DMA_Stream_TypeDef* DMAy_Streamx, bool enable)
{
  if ( enable )
  {
    /* Enable the selected DMAy Streamx by setting EN bit */
    DMAy_Streamx->CR |= (uint32_t)DMA_SxCR_EN;
  }
  else
  {
    /* Disable the selected DMAy Streamx by clearing EN bit */
    DMAy_Streamx->CR &= ~(uint32_t)DMA_SxCR_EN;
  }
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Configures, when the PINC (Peripheral Increment address mode) bit is
  *         set, if the peripheral address should be incremented with the data
  *         size (configured with PSIZE bits) or by a fixed offset equal to 4
  *         (32-bit aligned addresses).
  *
  * @note   This function has no effect if the Peripheral Increment mode is disabled.
  *
  * @param  DMAy_Streamx: where y can be 1 or 2 to select the DMA and x can be 0
  *          to 7 to select the DMA Stream.
  * @param  DMA_Pincos: specifies the Peripheral increment offset size.
  *          This parameter can be one of the following values:
  *            @arg DMA_PINCOS_Psize: Peripheral address increment is done
  *                                   accordingly to PSIZE parameter.
  *            @arg DMA_PINCOS_WordAligned: Peripheral address increment offset is
  *                                         fixed to 4 (32-bit aligned addresses).
  * @retval None
  */
static inline void dma_periph_inc_offset_size_config(DMA_Stream_TypeDef* DMAy_Streamx, uint32_t DMA_Pincos)
{
  /* Check the needed Peripheral increment offset */
  if(DMA_Pincos != DMA_PINCOS_Psize)
  {
    /* Configure DMA_SxCR_PINCOS bit with the input parameter */
    DMAy_Streamx->CR |= (uint32_t)DMA_SxCR_PINCOS;
  }
  else
  {
    /* Clear the PINCOS bit: Peripheral address incremented according to PSIZE */
    DMAy_Streamx->CR &= ~(uint32_t)DMA_SxCR_PINCOS;
  }
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Configures, when the DMAy Streamx is disabled, the flow controller for
  *         the next transactions (Peripheral or Memory).
  *
  * @note   Before enabling this feature, check if the used peripheral supports
  *         the Flow Controller mode or not.
  *
  * @param  DMAy_Streamx: where y can be 1 or 2 to select the DMA and x can be 0
  *          to 7 to select the DMA Stream.
  * @param  DMA_FlowCtrl: specifies the DMA flow controller.
  *          This parameter can be one of the following values:
  *            @arg DMA_FlowCtrl_Memory: DMAy_Streamx transactions flow controller is
  *                                      the DMA controller.
  *            @arg DMA_FlowCtrl_Peripheral: DMAy_Streamx transactions flow controller
  *                                          is the peripheral.
  * @retval None
  */
static inline void dma_flow_controller_config(DMA_Stream_TypeDef* DMAy_Streamx, uint32_t DMA_FlowCtrl)
{
  /* Check the needed flow controller  */
  if(DMA_FlowCtrl != DMA_FlowCtrl_Memory)
  {
    /* Configure DMA_SxCR_PFCTRL bit with the input parameter */
    DMAy_Streamx->CR |= (uint32_t)DMA_SxCR_PFCTRL;
  }
  else
  {
    /* Clear the PFCTRL bit: Memory is the flow controller */
    DMAy_Streamx->CR &= ~(uint32_t)DMA_SxCR_PFCTRL;
  }
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Writes the number of data units to be transferred on the DMAy Streamx.
  * @param  DMAy_Streamx: where y can be 1 or 2 to select the DMA and x can be 0
  *          to 7 to select the DMA Stream.
  * @param  Counter: Number of data units to be transferred (from 0 to 65535)
  *          Number of data items depends only on the Peripheral data format.
  *
  * @note   If Peripheral data format is Bytes: number of data units is equal
  *         to total number of bytes to be transferred.
  *
  * @note   If Peripheral data format is Half-Word: number of data units is
  *         equal to total number of bytes to be transferred / 2.
  *
  * @note   If Peripheral data format is Word: number of data units is equal
  *         to total  number of bytes to be transferred / 4.
  *
  * @note   In Memory-to-Memory transfer mode, the memory buffer pointed by
  *         DMAy_SxPAR register is considered as Peripheral.
  *
  * @retval The number of remaining data units in the current DMAy Streamx transfer.
  */
static inline void dma_set_curr_data_counter(DMA_Stream_TypeDef* DMAy_Streamx, uint16_t Counter)
{
  /* Write the number of data units to be transferred */
  DMAy_Streamx->NDTR = (uint16_t)Counter;
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Returns the number of remaining data units in the current DMAy Streamx transfer.
  * @param  DMAy_Streamx: where y can be 1 or 2 to select the DMA and x can be 0
  *          to 7 to select the DMA Stream.
  * @retval The number of remaining data units in the current DMAy Streamx transfer.
  */
static inline uint16_t dma_get_curr_data_counter(DMA_Stream_TypeDef* DMAy_Streamx)
{

  /* Return the number of remaining data units for DMAy Streamx */
  return ((uint16_t)(DMAy_Streamx->NDTR));
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Configures, when the DMAy Streamx is disabled, the double buffer mode
  *         and the current memory target.
  * @param  DMAy_Streamx: where y can be 1 or 2 to select the DMA and x can be 0
  *          to 7 to select the DMA Stream.
  * @param  Memory1BaseAddr: the base address of the second buffer (Memory 1)
  * @param  DMA_CurrentMemory: specifies which memory will be first buffer for
  *         the transactions when the Stream will be enabled.
  *          This parameter can be one of the following values:
  *            @arg DMA_Memory_0: Memory 0 is the current buffer.
  *            @arg DMA_Memory_1: Memory 1 is the current buffer.
  *
  * @note   Memory0BaseAddr is set by the DMA structure configuration in DMA_Init().
  *
  * @retval None
  */
static inline void dma_double_buffer_mode_config(DMA_Stream_TypeDef* DMAy_Streamx, void* Memory1BaseAddr,
                                uint32_t DMA_CurrentMemory)
{

  if (DMA_CurrentMemory != DMA_Memory_0)
  {
    /* Set Memory 1 as current memory address */
    DMAy_Streamx->CR |= (uint32_t)(DMA_SxCR_CT);
  }
  else
  {
    /* Set Memory 0 as current memory address */
    DMAy_Streamx->CR &= ~(uint32_t)(DMA_SxCR_CT);
  }

  /* Write to DMAy Streamx M1AR */
  DMAy_Streamx->M1AR = (uint32_t)Memory1BaseAddr;
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Enables or disables the double buffer mode for the selected DMA stream.
  * @note   This function can be called only when the DMA Stream is disabled.
  * @param  DMAy_Streamx: where y can be 1 or 2 to select the DMA and x can be 0
  *          to 7 to select the DMA Stream.
  * @param  NewState: new state of the DMAy Streamx double buffer mode.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void dma_double_buffer_mode_cmd(DMA_Stream_TypeDef* DMAy_Streamx, bool enable )
{
  /* Configure the Double Buffer mode */
  if ( enable )
  {
    /* Enable the Double buffer mode */
    DMAy_Streamx->CR |= (uint32_t)DMA_SxCR_DBM;
  }
  else
  {
    /* Disable the Double buffer mode */
    DMAy_Streamx->CR &= ~(uint32_t)DMA_SxCR_DBM;
  }
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Configures the Memory address for the next buffer transfer in double
  *         buffer mode (for dynamic use). This function can be called when the
  *         DMA Stream is enabled and when the transfer is ongoing.
  * @param  DMAy_Streamx: where y can be 1 or 2 to select the DMA and x can be 0
  *          to 7 to select the DMA Stream.
  * @param  MemoryBaseAddr: The base address of the target memory buffer
  * @param  DMA_MemoryTarget: Next memory target to be used.
  *         This parameter can be one of the following values:
  *            @arg DMA_Memory_0: To use the memory address 0
  *            @arg DMA_Memory_1: To use the memory address 1
  *
  * @note    It is not allowed to modify the Base Address of a target Memory when
  *          this target is involved in the current transfer. ie. If the DMA Stream
  *          is currently transferring to/from Memory 1, then it not possible to
  *          modify Base address of Memory 1, but it is possible to modify Base
  *          address of Memory 0.
  *          To know which Memory is currently used, you can use the function
  *          DMA_GetCurrentMemoryTarget().
  *
  * @retval None
  */
static inline void dma_memory_target_config(DMA_Stream_TypeDef* DMAy_Streamx, void* MemoryBaseAddr,
                           uint32_t DMA_MemoryTarget)
{

  /* Check the Memory target to be configured */
  if (DMA_MemoryTarget != DMA_Memory_0)
  {
    /* Write to DMAy Streamx M1AR */
    DMAy_Streamx->M1AR = (uint32_t)MemoryBaseAddr;
  }
  else
  {
    /* Write to DMAy Streamx M0AR */
    DMAy_Streamx->M0AR = (uint32_t)MemoryBaseAddr;
  }
}
/* ---------------------------------------------------------------------------- */
static inline void* dma_get_memory_target( DMA_Stream_TypeDef* DMAy_Streamx, 
		uint32_t DMA_MemoryTarget )
{

  /* Check the Memory target to be configured */
  if (DMA_MemoryTarget != DMA_Memory_0)
  {
    /* Write to DMAy Streamx M1AR */
    return (void*)DMAy_Streamx->M1AR;
  }
  else
  {
    /* Write to DMAy Streamx M0AR */
    return (void*)DMAy_Streamx->M0AR;
  }
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Returns the current memory target used by double buffer transfer.
  * @param  DMAy_Streamx: where y can be 1 or 2 to select the DMA and x can be 0
  *          to 7 to select the DMA Stream.
  * @retval The memory target number: 0 for Memory0 or 1 for Memory1.
  */
static inline uint32_t dma_get_current_memory_target(DMA_Stream_TypeDef* DMAy_Streamx)
{
  /* Get the current memory target */
  return ((DMAy_Streamx->CR & DMA_SxCR_CT) != 0)?(1):(0);
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Returns the status of EN bit for the specified DMAy Streamx.
  * @param  DMAy_Streamx: where y can be 1 or 2 to select the DMA and x can be 0
  *          to 7 to select the DMA Stream.
  *
  * @note    After configuring the DMA Stream (DMA_Init() function) and enabling
  *          the stream, it is recommended to check (or wait until) the DMA Stream
  *          is effectively enabled. A Stream may remain disabled if a configuration
  *          parameter is wrong.
  *          After disabling a DMA Stream, it is also recommended to check (or wait
  *          until) the DMA Stream is effectively disabled. If a Stream is disabled
  *          while a data transfer is ongoing, the current data will be transferred
  *          and the Stream will be effectively disabled only after the transfer
  *          of this single data is finished.
  *
  * @retval Current state of the DMAy Streamx (ENABLE or DISABLE).
  */
static inline bool dma_get_cmd_status(DMA_Stream_TypeDef* DMAy_Streamx)
{
  return ((DMAy_Streamx->CR & (uint32_t)DMA_SxCR_EN) != 0);
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Returns the current DMAy Streamx FIFO filled level.
  * @param  DMAy_Streamx: where y can be 1 or 2 to select the DMA and x can be 0
  *         to 7 to select the DMA Stream.
  * @retval The FIFO filling state.
  *           - DMA_FIFOStatus_Less1QuarterFull: when FIFO is less than 1 quarter-full
  *                                               and not empty.
  *           - DMA_FIFOStatus_1QuarterFull: if more than 1 quarter-full.
  *           - DMA_FIFOStatus_HalfFull: if more than 1 half-full.
  *           - DMA_FIFOStatus_3QuartersFull: if more than 3 quarters-full.
  *           - DMA_FIFOStatus_Empty: when FIFO is empty
  *           - DMA_FIFOStatus_Full: when FIFO is full
  */
static inline uint32_t dma_get_fifo_status(DMA_Stream_TypeDef* DMAy_Streamx)
{
  /* Get the FIFO level bits */
  return (uint32_t)((DMAy_Streamx->FCR & DMA_SxFCR_FS));
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Checks whether the specified DMAy Streamx flag is set or not.
  * @param  DMAy_Streamx: where y can be 1 or 2 to select the DMA and x can be 0
  *          to 7 to select the DMA Stream.
  * @param  DMA_FLAG: specifies the flag to check.
  *          This parameter can be one of the following values:
  *            @arg DMA_FLAG_TCIFx:  Streamx transfer complete flag
  *            @arg DMA_FLAG_HTIFx:  Streamx half transfer complete flag
  *            @arg DMA_FLAG_TEIFx:  Streamx transfer error flag
  *            @arg DMA_FLAG_DMEIFx: Streamx direct mode error flag
  *            @arg DMA_FLAG_FEIFx:  Streamx FIFO error flag
  *         Where x can be 0 to 7 to select the DMA Stream.
  * @retval The new state of DMA_FLAG (SET or RESET).
  */
static inline bool dma_get_flag_status(DMA_Stream_TypeDef* DMAy_Streamx, uint32_t DMA_FLAG)
{
  DMA_TypeDef* DMAy;
  uint32_t tmpreg = 0;

  /* Determine the DMA to which belongs the stream */
  if (DMAy_Streamx < DMA2_Stream0)
  {
    /* DMAy_Streamx belongs to DMA1 */
    DMAy = DMA1;
  }
  else
  {
    /* DMAy_Streamx belongs to DMA2 */
    DMAy = DMA2;
  }

  /* Check if the flag is in HISR or LISR */
  if ((DMA_FLAG & HIGH_ISR_MASK) != 0)
  {
    /* Get DMAy HISR register value */
    tmpreg = DMAy->HISR;
  }
  else
  {
    /* Get DMAy LISR register value */
    tmpreg = DMAy->LISR;
  }

  /* Mask the reserved bits */
  tmpreg &= (uint32_t)RESERVED_MASK;

  /* Check the status of the specified DMA flag */
  return ((tmpreg & DMA_FLAG) != 0);
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Clears the DMAy Streamx's pending flags.
  * @param  DMAy_Streamx: where y can be 1 or 2 to select the DMA and x can be 0
  *          to 7 to select the DMA Stream.
  * @param  DMA_FLAG: specifies the flag to clear.
  *          This parameter can be any combination of the following values:
  *            @arg DMA_FLAG_TCIFx:  Streamx transfer complete flag
  *            @arg DMA_FLAG_HTIFx:  Streamx half transfer complete flag
  *            @arg DMA_FLAG_TEIFx:  Streamx transfer error flag
  *            @arg DMA_FLAG_DMEIFx: Streamx direct mode error flag
  *            @arg DMA_FLAG_FEIFx:  Streamx FIFO error flag
  *         Where x can be 0 to 7 to select the DMA Stream.
  * @retval None
  */
static inline void dma_clear_flag(DMA_Stream_TypeDef* DMAy_Streamx, uint32_t DMA_FLAG)
{
  DMA_TypeDef* DMAy;

  /* Determine the DMA to which belongs the stream */
  if (DMAy_Streamx < DMA2_Stream0)
  {
    /* DMAy_Streamx belongs to DMA1 */
    DMAy = DMA1;
  }
  else
  {
    /* DMAy_Streamx belongs to DMA2 */
    DMAy = DMA2;
  }

  /* Check if LIFCR or HIFCR register is targeted */
  if ((DMA_FLAG & HIGH_ISR_MASK) != (uint32_t)0)
  {
    /* Set DMAy HIFCR register clear flag bits */
    DMAy->HIFCR = (uint32_t)(DMA_FLAG & RESERVED_MASK);
  }
  else
  {
    /* Set DMAy LIFCR register clear flag bits */
    DMAy->LIFCR = (uint32_t)(DMA_FLAG & RESERVED_MASK);
  }
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Enables or disables the specified DMAy Streamx interrupts.
  * @param  DMAy_Streamx: where y can be 1 or 2 to select the DMA and x can be 0
  *          to 7 to select the DMA Stream.
  * @param DMA_IT: specifies the DMA interrupt sources to be enabled or disabled.
  *          This parameter can be any combination of the following values:
  *            @arg DMA_IT_TC:  Transfer complete interrupt mask
  *            @arg DMA_IT_HT:  Half transfer complete interrupt mask
  *            @arg DMA_IT_TE:  Transfer error interrupt mask
  *            @arg DMA_IT_FE:  FIFO error interrupt mask
  * @param  NewState: new state of the specified DMA interrupts.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void dma_it_config(DMA_Stream_TypeDef* DMAy_Streamx, uint32_t DMA_IT, bool enable)
{

  /* Check if the DMA_IT parameter contains a FIFO interrupt */
  if ((DMA_IT & DMA_IT_FE) != 0)
  {
    if ( enable )
    {
      /* Enable the selected DMA FIFO interrupts */
      DMAy_Streamx->FCR |= (uint32_t)DMA_IT_FE;
    }
    else
    {
      /* Disable the selected DMA FIFO interrupts */
      DMAy_Streamx->FCR &= ~(uint32_t)DMA_IT_FE;
    }
  }

  /* Check if the DMA_IT parameter contains a Transfer interrupt */
  if (DMA_IT != DMA_IT_FE)
  {
    if ( enable )
    {
      /* Enable the selected DMA transfer interrupts */
      DMAy_Streamx->CR |= (uint32_t)(DMA_IT  & TRANSFER_IT_ENABLE_MASK);
    }
    else
    {
      /* Disable the selected DMA transfer interrupts */
      DMAy_Streamx->CR &= ~(uint32_t)(DMA_IT & TRANSFER_IT_ENABLE_MASK);
    }
  }
}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Checks whether the specified DMAy Streamx interrupt has occurred or not.
  * @param  DMAy_Streamx: where y can be 1 or 2 to select the DMA and x can be 0
  *          to 7 to select the DMA Stream.
  * @param  DMA_IT: specifies the DMA interrupt source to check.
  *          This parameter can be one of the following values:
  *            @arg DMA_IT_TCIFx:  Streamx transfer complete interrupt
  *            @arg DMA_IT_HTIFx:  Streamx half transfer complete interrupt
  *            @arg DMA_IT_TEIFx:  Streamx transfer error interrupt
  *            @arg DMA_IT_DMEIFx: Streamx direct mode error interrupt
  *            @arg DMA_IT_FEIFx:  Streamx FIFO error interrupt
  *         Where x can be 0 to 7 to select the DMA Stream.
  * @retval The new state of DMA_IT (SET or RESET).
  */
static inline bool dma_get_it_status(DMA_Stream_TypeDef* DMAy_Streamx, uint32_t DMA_IT)
{
  DMA_TypeDef* DMAy;
  uint32_t tmpreg = 0, enablestatus = 0;

  /* Determine the DMA to which belongs the stream */
  if (DMAy_Streamx < DMA2_Stream0)
  {
    /* DMAy_Streamx belongs to DMA1 */
    DMAy = DMA1;
  }
  else
  {
    /* DMAy_Streamx belongs to DMA2 */
    DMAy = DMA2;
  }

  /* Check if the interrupt enable bit is in the CR or FCR register */
  if ((DMA_IT & TRANSFER_IT_MASK) != (uint32_t)0)
  {
    /* Get the interrupt enable position mask in CR register */
    tmpreg = (uint32_t)((DMA_IT >> 11) & TRANSFER_IT_ENABLE_MASK);

    /* Check the enable bit in CR register */
    enablestatus = (uint32_t)(DMAy_Streamx->CR & tmpreg);
  }
  else
  {
    /* Check the enable bit in FCR register */
    enablestatus = (uint32_t)(DMAy_Streamx->FCR & DMA_IT_FE);
  }

  /* Check if the interrupt pending flag is in LISR or HISR */
  if ((DMA_IT & HIGH_ISR_MASK) != (uint32_t)0)
  {
    /* Get DMAy HISR register value */
    tmpreg = DMAy->HISR ;
  }
  else
  {
    /* Get DMAy LISR register value */
    tmpreg = DMAy->LISR ;
  }

  /* mask all reserved bits */
  tmpreg &= (uint32_t)RESERVED_MASK;

  /* Check the status of the specified DMA interrupt */
  return (((tmpreg & DMA_IT) != (uint32_t)0) && (enablestatus != (uint32_t)0));

}
/* ---------------------------------------------------------------------------- */
/**
  * @brief  Clears the DMAy Streamx's interrupt pending bits.
  * @param  DMAy_Streamx: where y can be 1 or 2 to select the DMA and x can be 0
  *          to 7 to select the DMA Stream.
  * @param  DMA_IT: specifies the DMA interrupt pending bit to clear.
  *          This parameter can be any combination of the following values:
  *            @arg DMA_IT_TCIFx:  Streamx transfer complete interrupt
  *            @arg DMA_IT_HTIFx:  Streamx half transfer complete interrupt
  *            @arg DMA_IT_TEIFx:  Streamx transfer error interrupt
  *            @arg DMA_IT_DMEIFx: Streamx direct mode error interrupt
  *            @arg DMA_IT_FEIFx:  Streamx FIFO error interrupt
  *         Where x can be 0 to 7 to select the DMA Stream.
  * @retval None
  */
static inline void dma_clear_it_pending_bit(DMA_Stream_TypeDef* DMAy_Streamx, uint32_t DMA_IT)
{
  DMA_TypeDef* DMAy;

  /* Determine the DMA to which belongs the stream */
  if (DMAy_Streamx < DMA2_Stream0)
  {
    /* DMAy_Streamx belongs to DMA1 */
    DMAy = DMA1;
  }
  else
  {
    /* DMAy_Streamx belongs to DMA2 */
    DMAy = DMA2;
  }

  /* Check if LIFCR or HIFCR register is targeted */
  if ((DMA_IT & HIGH_ISR_MASK) != (uint32_t)0)
  {
    /* Set DMAy HIFCR register clear interrupt bits */
    DMAy->HIFCR = (uint32_t)(DMA_IT & RESERVED_MASK);
  }
  else
  {
    /* Set DMAy LIFCR register clear interrupt bits */
    DMAy->LIFCR = (uint32_t)(DMA_IT & RESERVED_MASK);
  }
}

/**
  * @brief  Deinitialize the DMAy Streamx registers to their default reset values.
  * @param  DMAy_Streamx: where y can be 1 or 2 to select the DMA and x can be 0
  *         to 7 to select the DMA Stream.
  * @retval None
  */
static inline void dma_deinit(DMA_Stream_TypeDef* DMAy_Streamx)
{

  /* Disable the selected DMAy Streamx */
  DMAy_Streamx->CR &= ~((uint32_t)DMA_SxCR_EN);

  /* Reset DMAy Streamx control register */
  DMAy_Streamx->CR  = 0;

  /* Reset DMAy Streamx Number of Data to Transfer register */
  DMAy_Streamx->NDTR = 0;

  /* Reset DMAy Streamx peripheral address register */
  DMAy_Streamx->PAR  = 0;

  /* Reset DMAy Streamx memory 0 address register */
  DMAy_Streamx->M0AR = 0;

  /* Reset DMAy Streamx memory 1 address register */
  DMAy_Streamx->M1AR = 0;

  /* Reset DMAy Streamx FIFO control register */
  DMAy_Streamx->FCR = (uint32_t)0x00000021;

  /* Reset interrupt pending bits for the selected stream */
  if (DMAy_Streamx == DMA1_Stream0)
  {
    /* Reset interrupt pending bits for DMA1 Stream0 */
    DMA1->LIFCR = DMA_Stream0_IT_MASK;
  }
  else if (DMAy_Streamx == DMA1_Stream1)
  {
    /* Reset interrupt pending bits for DMA1 Stream1 */
    DMA1->LIFCR = DMA_Stream1_IT_MASK;
  }
  else if (DMAy_Streamx == DMA1_Stream2)
  {
    /* Reset interrupt pending bits for DMA1 Stream2 */
    DMA1->LIFCR = DMA_Stream2_IT_MASK;
  }
  else if (DMAy_Streamx == DMA1_Stream3)
  {
    /* Reset interrupt pending bits for DMA1 Stream3 */
    DMA1->LIFCR = DMA_Stream3_IT_MASK;
  }
  else if (DMAy_Streamx == DMA1_Stream4)
  {
    /* Reset interrupt pending bits for DMA1 Stream4 */
    DMA1->HIFCR = DMA_Stream4_IT_MASK;
  }
  else if (DMAy_Streamx == DMA1_Stream5)
  {
    /* Reset interrupt pending bits for DMA1 Stream5 */
    DMA1->HIFCR = DMA_Stream5_IT_MASK;
  }
  else if (DMAy_Streamx == DMA1_Stream6)
  {
    /* Reset interrupt pending bits for DMA1 Stream6 */
    DMA1->HIFCR = (uint32_t)DMA_Stream6_IT_MASK;
  }
  else if (DMAy_Streamx == DMA1_Stream7)
  {
    /* Reset interrupt pending bits for DMA1 Stream7 */
    DMA1->HIFCR = DMA_Stream7_IT_MASK;
  }
  else if (DMAy_Streamx == DMA2_Stream0)
  {
    /* Reset interrupt pending bits for DMA2 Stream0 */
    DMA2->LIFCR = DMA_Stream0_IT_MASK;
  }
  else if (DMAy_Streamx == DMA2_Stream1)
  {
    /* Reset interrupt pending bits for DMA2 Stream1 */
    DMA2->LIFCR = DMA_Stream1_IT_MASK;
  }
  else if (DMAy_Streamx == DMA2_Stream2)
  {
    /* Reset interrupt pending bits for DMA2 Stream2 */
    DMA2->LIFCR = DMA_Stream2_IT_MASK;
  }
  else if (DMAy_Streamx == DMA2_Stream3)
  {
    /* Reset interrupt pending bits for DMA2 Stream3 */
    DMA2->LIFCR = DMA_Stream3_IT_MASK;
  }
  else if (DMAy_Streamx == DMA2_Stream4)
  {
    /* Reset interrupt pending bits for DMA2 Stream4 */
    DMA2->HIFCR = DMA_Stream4_IT_MASK;
  }
  else if (DMAy_Streamx == DMA2_Stream5)
  {
    /* Reset interrupt pending bits for DMA2 Stream5 */
    DMA2->HIFCR = DMA_Stream5_IT_MASK;
  }
  else if (DMAy_Streamx == DMA2_Stream6)
  {
    /* Reset interrupt pending bits for DMA2 Stream6 */
    DMA2->HIFCR = DMA_Stream6_IT_MASK;
  }
  else
  {
    if (DMAy_Streamx == DMA2_Stream7)
    {
      /* Reset interrupt pending bits for DMA2 Stream7 */
      DMA2->HIFCR = DMA_Stream7_IT_MASK;
    }
  }
}

/* ---------------------------------------------------------------------------- */
//Undefined private defs
#undef TRANSFER_IT_ENABLE_MASK
#undef DMA_Stream0_IT_MASK
#undef DMA_Stream1_IT_MASK
#undef DMA_Stream2_IT_MASK
#undef DMA_Stream3_IT_MASK
#undef DMA_Stream4_IT_MASK
#undef DMA_Stream5_IT_MASK
#undef DMA_Stream6_IT_MASK
#undef DMA_Stream7_IT_MASK
#undef TRANSFER_IT_MASK
#undef HIGH_ISR_MASK
#undef RESERVED_MASK
/* ---------------------------------------------------------------------------- */
#ifdef __cplusplus
}
#endif /* __cplusplus */
 /* ---------------------------------------------------------------------------- */
#endif /* STM32DMA_H_ */
