/*
 * stm32_spi_master_dma.cpp
 *
 *  Created on: 11-12-2012
 *      Author: lucck
 */
/*----------------------------------------------------------*/
#include "stm32_spi_master_dma.hpp"
#include <stm32rcc.h>
#include <stm32system.h>
#include <stm32dma.h>
#include <stm32spi.h>
#include <dbglog.h>
/*----------------------------------------------------------*/
#if defined(CONFIG_ISIX_DRV_SPI_ENABLE_DMA_MASK) && (CONFIG_ISIX_DRV_SPI_ENABLE_DMA_MASK!=0)

//Temporary check
#ifndef STM32MCU_MAJOR_TYPE_F1
#error SPI DMA not implemented for F2 F4 platform
#endif

#if CONFIG_ISIX_DRV_SPI_ENABLE_DMA_MASK != ISIX_DRV_SPI_DMA_SPI1_ENABLE
#error DMA mode for SPI1 is implemented now
#endif
/*----------------------------------------------------------*/
namespace stm32 {
namespace drv {
/*----------------------------------------------------------*/

/* Interrupt handler section */
namespace {
#if CONFIG_ISIX_DRV_SPI_SUPPORTED_DEVS == ISIX_DRV_SPI_ALL_ENABLE
	spi_master_dma *p_spi1;
	spi_master_dma *p_spi2;
	spi_master_dma *p_spi3;
#elif CONFIG_ISIX_DRV_SPI_SUPPORTED_DEVS == ISIX_DRV_SPI_SPI1_ENABLE
	spi_master_dma* p_spi;
	spi_master_dma* &p_spi1 = p_spi;
#elif CONFIG_ISIX_DRV_SPI_SUPPORTED_DEVS == ISIX_DRV_SPI_SPI2_ENABLE
	spi_master_dma *p_spi;
#elif CONFIG_ISIX_DRV_SPI_SUPPORTED_DEVS == ISIX_DRV_SPI_SPI3_ENABLE
	spi_master_dma *p_spi;
#else
#error Unknown SPI
#endif
}

/*----------------------------------------------------------*/
extern "C" {
#if CONFIG_ISIX_DRV_SPI_ENABLE_DMA_MASK & ISIX_DRV_SPI_DMA_SPI1_ENABLE
	// Interrupt handlers SPI1 RX
	void __attribute__((__interrupt__)) dma1_channel2_isr_vector(void)
	{
		if( p_spi1 ) p_spi1->handle_dma_rx_isr();
	}
	// Interrupt handlers SPI1 TX
	void __attribute__((__interrupt__)) dma1_channel3_isr_vector(void)
	{
		if( p_spi1 ) p_spi1->handle_dma_tx_isr();
	}
#endif
}

/*----------------------------------------------------------*/
#if(!CONFIG_ISIX_DRV_SPI_SUPPORTED_DEVS)
	spi_master_dma::spi_master_dma(SPI_TypeDef *spi, unsigned pclk1, unsigned pclk2)
		: spi_master(spi, pclk1, pclk2), m_notify_sem(0, 1)
#else
	spi_master_dma::spi_master_dma()
		: m_notify_sem(0, 1)
#endif
{
#if(!CONFIG_ISIX_DRV_SPI_SUPPORTED_DEVS)
	if(spi == SPI1 ) p_spi1 = this;
	else if(spi == SPI2) p_spi2 = this;
	else if(spi == SPI3) p_spi3 = this;
#else
	p_spi = this;
#endif
#ifdef STM32MCU_MAJOR_TYPE_F1
#if CONFIG_ISIX_DRV_SPI_ENABLE_DMA_MASK & (ISIX_DRV_SPI_DMA_SPI1_ENABLE|ISIX_DRV_SPI_DMA_SPI2_ENABLE)
	if( m_spi == SPI1 || m_spi == SPI2 )
		stm32::rcc_ahb_periph_clock_cmd( RCC_AHBPeriph_DMA1, true);
#endif
#if (CONFIG_ISIX_DRV_SPI_ENABLE_DMA_MASK & ISIX_DRV_SPI_DMA_SPI3_ENABLE) && defined(SPI3)
	if( m_spi == SPI3)
	stm32::rcc_ahb_periph_clock_cmd( RCC_AHBPeriph_DMA2, true);
#endif
#if CONFIG_ISIX_DRV_SPI_ENABLE_DMA_MASK & ISIX_DRV_SPI_DMA_SPI1_ENABLE
	if( m_spi == SPI1 )
	{
		stm32::nvic_set_priority( DMA1_Channel2_IRQn, IRQPRIO, IRQSUB );
		stm32::nvic_set_priority( DMA1_Channel3_IRQn, IRQPRIO, IRQSUB );
		//stm32::nvic_irq_enable( DMA1_Channel2_IRQn, true );
		//stm32::nvic_irq_enable( DMA1_Channel3_IRQn, true );
	}
#endif
#endif
	stm32::spi_i2s_dma_cmd( m_spi, SPI_I2S_DMAReq_Rx| SPI_I2S_DMAReq_Tx, true );
}
/*----------------------------------------------------------*/
/* Destructor */
spi_master_dma::~spi_master_dma()
{
#ifdef STM32MCU_MAJOR_TYPE_F1
#if CONFIG_ISIX_DRV_SPI_ENABLE_DMA_MASK & (ISIX_DRV_SPI_DMA_SPI1_ENABLE|ISIX_DRV_SPI_DMA_SPI2_ENABLE)
	stm32::rcc_ahb_periph_clock_cmd( RCC_AHBPeriph_DMA1, false);
#endif
#if CONFIG_ISIX_DRV_SPI_ENABLE_DMA_MASK & ISIX_DRV_SPI_DMA_SPI3_ENABLE
	stm32::rcc_ahb_periph_clock_cmd( RCC_AHBPeriph_DMA2, false);
#endif
#endif
	p_spi = 0;
}
/*----------------------------------------------------------*/
/* Write to the device */
int spi_master_dma::write( const void *buf, size_t len)
{
#if CONFIG_ISIX_DRV_SPI_ENABLE_DMA_MASK & ISIX_DRV_SPI_DMA_SPI1_ENABLE
	if( m_spi == SPI1 )
	{
		//for(;;)
		{
			stm32::dma_channel_config( DMA1_Channel3,
			DMA_MemoryInc_Enable|DMA_PeripheralInc_Disable|DMA_PeripheralDataSize_Byte|
			 DMA_MemoryDataSize_Byte| DMA_Mode_Normal | DMA_Priority_High | DMA_M2M_Disable |DMA_DIR_PeripheralDST,
			 (void*)buf, &m_spi->DR, len );
			DMA1->IFCR = (1<<9);
			stm32::dma_channel_enable(DMA1_Channel3);
			//dbprintf("TRansfered %08x", DMA1->ISR);
			while( !(DMA1->ISR & (1<<9)) );
			//dbprintf("TRansfered %08x", DMA1->ISR);
			stm32::dma_channel_disable(DMA1_Channel3);
		}
		while(spi_i2s_get_flag_status(m_spi, SPI_I2S_FLAG_RXNE))
			 spi_i2s_receive_data( m_spi );
		while(spi_i2s_get_flag_status(m_spi, SPI_I2S_FLAG_BSY));
	}
#endif
	return 0;
	//return spi_master::write( buf, len );
}
/*----------------------------------------------------------*/
/* Read from the device */
int spi_master_dma::read ( void *buf, size_t len)
{
	return spi_master::read( buf, len );
}
/*----------------------------------------------------------*/
/* Transfer (BIDIR) */
int spi_master_dma::transfer( const void *inbuf, void *outbuf, size_t len )
{
	return spi_master::transfer( inbuf, outbuf, len );
}
/*----------------------------------------------------------*/
//Handle DMA TX IRQ
void spi_master_dma::handle_dma_tx_isr()
{

}
/*----------------------------------------------------------*/
//Handle DMA RX IRQ
void spi_master_dma::handle_dma_rx_isr()
{
}
/*----------------------------------------------------------*/
} /* namespace drv */
} /* namespace stm32 */
/*----------------------------------------------------------*/
#endif
