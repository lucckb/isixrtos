/*
 * stm32_spi_master_dma.cpp
 *
 *  Created on: 11-12-2012
 *      Author: lucck
 */

#include "stm32_spi_master_dma.hpp"
#include <stm32rcc.h>
#include <stm32system.h>
#include <stm32dma.h>
#include <stm32spi.h>
#include <stm32bitbang.h>
#include <foundation/sys/dbglog.h>
#include <cstring>


//Temporary check
#ifndef STM32MCU_MAJOR_TYPE_F1
#error SPI DMA not implemented for F2 F4 platform
#endif



namespace stm32 {
namespace drv {

#if CONFIG_ISIXDRV_SPI_ENABLE_DMAIRQ
/* Interrupt handler section */
namespace {
#if CONFIG_ISIXDRV_SPI_SUPPORTED_DEVS & ISIX_DRV_SPI_SPI1_ENABLE
	spi_master_dma *p_spi1;
#endif
#if CONFIG_ISIXDRV_SPI_SUPPORTED_DEVS & ISIX_DRV_SPI_SPI2_ENABLE
	spi_master_dma *p_spi2;
#endif
#if CONFIG_ISIXDRV_SPI_SUPPORTED_DEVS & ISIX_DRV_SPI_SPI3_ENABLE
	spi_master_dma *p_spi3;
#endif
}
#endif



extern "C" {
#if  CONFIG_ISIXDRV_SPI_ENABLE_DMAIRQ
#if CONFIG_ISIXDRV_SPI_SUPPORTED_DEVS & ISIX_DRV_SPI_SPI1_ENABLE
	// Interrupt handlers SPI1 RX
	void __attribute__((__interrupt__)) dma1_channel2_isr_vector(void)
	{
		if( p_spi1 ) p_spi1->handle_isr( stm32::dma_get_flag_status(DMA1_FLAG_TE2)?(spi_master_dma::irqs_te_rx):(spi_master_dma::irqs_tc_rx) );
		stm32::dma_clear_flag( DMA1_FLAG_GL2 );
	}
	// Interrupt handlers SPI1 TX
	void __attribute__((__interrupt__)) dma1_channel3_isr_vector(void)
	{
		if( p_spi1 ) p_spi1->handle_isr( stm32::dma_get_flag_status(DMA1_FLAG_TE3)?(spi_master_dma::irqs_te_tx):(spi_master_dma::irqs_tc_tx) );
		stm32::dma_clear_flag( DMA1_FLAG_GL3 );
	}
#endif
#endif
}


namespace
{
	//Wait for tx dma complete
	inline void dma_tx_config(SPI_TypeDef *spi, const void* buf, std::size_t len )
	{
	#if (CONFIG_ISIXDRV_SPI_SUPPORTED_DEVS & ISIX_DRV_SPI_SPI1_ENABLE)
		if( spi == SPI1 )
		{
			dma_set_memory_address(DMA1_Channel3, buf );
			DMA1_Channel3->CCR |= DMA_MemoryInc_Enable;
			stm32::dma_set_curr_data_counter( DMA1_Channel3, len );
			stm32::dma_clear_flag( DMA1_FLAG_GL3|DMA1_FLAG_TC3|DMA1_FLAG_HT3|DMA1_FLAG_TE3);
			stm32::dma_channel_enable(DMA1_Channel3);
		}
	#endif
	}
	//Wait for rx dma complete
	inline void dma_rx_config(SPI_TypeDef *spi, const void* inbuf, void *outbuf, std::size_t len )
	{
		static const unsigned char dummy = 0xff;
	#if (CONFIG_ISIXDRV_SPI_SUPPORTED_DEVS & ISIX_DRV_SPI_SPI1_ENABLE)
		if( spi == SPI1 )
		{
			/* Setup empty TX trn */
			if(outbuf)
			{
				DMA1_Channel3->CCR |= DMA_MemoryInc_Enable;
				dma_set_memory_address(DMA1_Channel3, outbuf );
			}
			else
			{
				DMA1_Channel3->CCR &= ~DMA_MemoryInc_Enable;
				dma_set_memory_address(DMA1_Channel3, &dummy );
			}
			stm32::dma_set_curr_data_counter( DMA1_Channel3, len );
			stm32::dma_clear_flag( DMA1_FLAG_GL3 );
			/* RX tran */
			dma_set_memory_address(DMA1_Channel2, inbuf );
			stm32::dma_set_curr_data_counter( DMA1_Channel2, len );
			stm32::dma_clear_flag( DMA1_FLAG_GL2 );
			stm32::dma_channel_enable(DMA1_Channel2);
			stm32::dma_channel_enable(DMA1_Channel3);
		}
	#endif
	}
	/* Dma disable */
	inline void dma_rx_disable(SPI_TypeDef *spi)
	{
	#if (CONFIG_ISIXDRV_SPI_SUPPORTED_DEVS & ISIX_DRV_SPI_SPI1_ENABLE)
		if( spi == SPI1 )
		{
			stm32::dma_channel_disable(DMA1_Channel3);
			stm32::dma_channel_disable(DMA1_Channel2);
		}
	#endif
	}
	/* Dma Disable */
	inline void dma_tx_disable(SPI_TypeDef *spi)
	{
	#if (CONFIG_ISIXDRV_SPI_SUPPORTED_DEVS & ISIX_DRV_SPI_SPI1_ENABLE)
		if( spi == SPI1 )
		{
			stm32::dma_channel_disable(DMA1_Channel3);
		}
	#endif
	}
	/* Busy wait */
	inline int dma_tx_busy_wait(SPI_TypeDef *spi)
	{
	#if (CONFIG_ISIXDRV_SPI_SUPPORTED_DEVS & ISIX_DRV_SPI_SPI1_ENABLE)
		if( spi == SPI1 )
		{
			 while(!stm32::dma_get_flag_status(DMA1_FLAG_TC3|DMA1_FLAG_TE3));
			 if( stm32::dma_get_flag_status(DMA1_FLAG_TE3) )
			 {
				 return ::drv::spi_device::err_dma;
			 }
		}
	#endif /*(CONFIG_ISIXDRV_SPI_SUPPORTED_DEVS & ISIX_DRV_SPI_SPI1_ENABLE)*/
		return ::drv::spi_device::err_ok;
	}
	inline int dma_rx_busy_wait(SPI_TypeDef *spi)
	{
	#if (CONFIG_ISIXDRV_SPI_SUPPORTED_DEVS & ISIX_DRV_SPI_SPI1_ENABLE)
		/* Busy wait */
		if( spi == SPI1 )
		{
			while(!(stm32::dma_get_flag_status(DMA1_FLAG_TC3) &&  stm32::dma_get_flag_status(DMA1_FLAG_TC2)))
			{
				if( stm32::dma_get_flag_status(DMA1_FLAG_TE3|DMA1_FLAG_TE2) )
				{
					 return ::drv::spi_device::err_dma;
				}
			}
		}
	#endif /*CONFIG_ISIXDRV_SPI_ENABLE_DMAIRQ & ISIX_DRV_SPI_DMAIRQ_SPI1_ENABLE */
		return ::drv::spi_device::err_ok;
	}
}


spi_master_dma::spi_master_dma(SPI_TypeDef *spi, unsigned pclk1, unsigned pclk2)
		: spi_master(spi, pclk1, pclk2)
#if CONFIG_ISIXDRV_SPI_ENABLE_DMAIRQ
	,m_notify_sem(0, 1), m_irq_flags(0)
#endif /*CONFIG_ISIXDRV_SPI_ENABLE_DMAIRQ */
{
#if CONFIG_ISIXDRV_SPI_ENABLE_DMAIRQ
#if (CONFIG_ISIXDRV_SPI_SUPPORTED_DEVS & ISIX_DRV_SPI_SPI1_ENABLE)
	if(spi == SPI1 ) p_spi1 = this;
#endif
#if (CONFIG_ISIXDRV_SPI_SUPPORTED_DEVS & ISIX_DRV_SPI_SPI2_ENABLE)
	if(spi == SPI2) p_spi2 = this;
#endif
#if (CONFIG_ISIXDRV_SPI_SUPPORTED_DEVS & ISIX_DRV_SPI_SPI3_ENABLE)
	if(spi == SPI3) p_spi3 = this;
#endif

#endif /* CONFIG_ISIXDRV_SPI_ENABLE_DMAIRQ */
#ifdef STM32MCU_MAJOR_TYPE_F1
	if( m_spi == SPI1 || m_spi == SPI2 )
		stm32::rcc_ahb_periph_clock_cmd( RCC_AHBPeriph_DMA1, true);
#if defined(SPI3)
	if( m_spi == SPI3)
		stm32::rcc_ahb_periph_clock_cmd( RCC_AHBPeriph_DMA2, true);
#endif
#if CONFIG_ISIXDRV_SPI_SUPPORTED_DEVS & ISIX_DRV_SPI_SPI1_ENABLE
	if( m_spi == SPI1 )
	{
#if CONFIG_ISIXDRV_SPI_ENABLE_DMAIRQ
		stm32::nvic_set_priority( DMA1_Channel2_IRQn, IRQPRIO, IRQSUB );
		stm32::nvic_set_priority( DMA1_Channel3_IRQn, IRQPRIO, IRQSUB );
		stm32::nvic_irq_enable( DMA1_Channel2_IRQn, true );
		stm32::nvic_irq_enable( DMA1_Channel3_IRQn, true );
#endif /*CONFIG_ISIXDRV_SPI_ENABLE_DMAIRQ*/
		//Enable channel3 for TX
		stm32::dma_channel_config( DMA1_Channel3,
				DMA_MemoryInc_Enable|DMA_PeripheralInc_Disable|DMA_PeripheralDataSize_Byte|
				 DMA_MemoryDataSize_Byte| DMA_Mode_Normal | DMA_Priority_High |
				 DMA_M2M_Disable |DMA_DIR_PeripheralDST,  0, &m_spi->DR, 0 );
		//Enable channel2 for RX
		stm32::dma_channel_config( DMA1_Channel2,
			DMA_MemoryInc_Enable|DMA_PeripheralInc_Disable|DMA_PeripheralDataSize_Byte|
			DMA_MemoryDataSize_Byte| DMA_Mode_Normal | DMA_Priority_High |
			DMA_M2M_Disable |DMA_DIR_PeripheralSRC,  0, &m_spi->DR, 0 );
		//Enable interrupts
		stm32::spi_i2s_dma_cmd( m_spi, SPI_I2S_DMAReq_Rx| SPI_I2S_DMAReq_Tx, true );
#if CONFIG_ISIXDRV_SPI_ENABLE_DMAIRQ
		stm32::dma_irq_enable ( DMA1_Channel2, DMA_IT_TE | DMA_IT_TC );
		stm32::dma_irq_enable ( DMA1_Channel3, DMA_IT_TE | DMA_IT_TC );
#endif /* CONFIG_ISIXDRV_SPI_ENABLE_DMAIRQ */
	}
#endif /*(CONFIG_ISIXDRV_SPI_SUPPORTED_DEVS == ISIX_DRV_SPI_SPI1_ENABLE) || !(CONFIG_ISIXDRV_SPI_SUPPORTED_DEVS) */
#endif /* STM32MCU_MAJOR_TYPE_F1 */
}

/* Destructor */
spi_master_dma::~spi_master_dma()
{
	dma_rx_disable(m_spi);

#if CONFIG_ISIXDRV_SPI_ENABLE_DMAIRQ
#if (CONFIG_ISIXDRV_SPI_SUPPORTED_DEVS & ISIX_DRV_SPI_SPI1_ENABLE)
	if( m_spi == SPI1 )  p_spi1 = 0;
#endif
#if (CONFIG_ISIXDRV_SPI_SUPPORTED_DEVS & ISIX_DRV_SPI_SPI2_ENABLE)
	if( m_spi == SPI2 )  p_spi2 = 0;
#endif
#if (CONFIG_ISIXDRV_SPI_SUPPORTED_DEVS & ISIX_DRV_SPI_SPI3_ENABLE)
	if( m_spi == SPI3 )  p_spi3 = 0;
#endif
#endif /* CONFIG_ISIXDRV_SPI_ENABLE_DMAIRQ */
}

/* Write to the device */
int spi_master_dma::write( const void *buf, size_t len)
{
#if CONFIG_ISIXDRV_SPI_ENABLE_DMAIRQ
	stm32::resetBitsAll_BB(&m_irq_flags);
#endif
	dma_tx_config( m_spi, buf, len );
	int ret = spi_device::err_ok;
#if CONFIG_ISIXDRV_SPI_ENABLE_DMAIRQ
	do {
		if( (ret=m_notify_sem.wait( spi_device::C_spi_timeout )) )
			break;
	    if( getBitsAll_BB(&m_irq_flags)&irqs_err_msk )
	    {
	        ret = spi_device::err_dma;
	       break;
	    }
	} while(0);
#else
	ret = dma_tx_busy_wait(m_spi);
#endif /*ISIX_DRV_SPI_DMA_WITH_IRQ*/
	while(spi_i2s_get_flag_status(m_spi, SPI_I2S_FLAG_BSY));
	while(spi_i2s_get_flag_status(m_spi, SPI_I2S_FLAG_RXNE))
		spi_i2s_receive_data( m_spi );
	dma_tx_disable( m_spi );
	return ret;
}

/* Transfer from the device */
int spi_master_dma::transfer( const void *inbuf, void *outbuf, size_t len )
{
#if CONFIG_ISIXDRV_SPI_ENABLE_DMAIRQ
	stm32::setBitsAll_BB(&m_irq_flags, 1<<irqs_rxmode );
#endif
	dma_rx_config( m_spi, inbuf, outbuf, len );
	int ret;
#if CONFIG_ISIXDRV_SPI_ENABLE_DMAIRQ
	do
	{
		if( (ret=m_notify_sem.wait( spi_device::C_spi_timeout )) )
			break;
		if( getBitsAll_BB(&m_irq_flags)&irqs_err_msk )
		{
			ret = spi_device::err_dma;
			break;
		}
	} while(0);
#else
	ret = dma_rx_busy_wait(m_spi);
#endif /*ISIX_DRV_SPI_DMA_WITH_IRQ*/
	while(spi_i2s_get_flag_status(m_spi, SPI_I2S_FLAG_BSY));
	dma_rx_disable(m_spi);
	return ret;
}

#if CONFIG_ISIXDRV_SPI_ENABLE_DMAIRQ
//Handle DMA TX IRQ
void spi_master_dma::handle_isr( spi_master_dma::irqs_no reason )
{
	using namespace stm32;
	setBit_BB( &m_irq_flags, reason );
	if( (m_irq_flags&irqs_err_msk) ||
		( getBit_BB(&m_irq_flags, irqs_rxmode) && ((getBitsAll_BB(&m_irq_flags)&irqs_rxtc_msk)==irqs_rxtc_msk ) ) ||
		( !getBit_BB(&m_irq_flags, irqs_rxmode) && ((getBitsAll_BB(&m_irq_flags)&irqs_txtc_msk)==irqs_txtc_msk) )
	)
	{
		m_notify_sem.signal_isr();
	}
}
#endif

} /* namespace drv */
} /* namespace stm32 */


