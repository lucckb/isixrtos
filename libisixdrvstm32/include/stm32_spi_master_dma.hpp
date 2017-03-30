/*
 * stm32_spi_master_dma.hpp
 *
 *  Created on: 11-12-2012
 *      Author: lucck
 */
/*----------------------------------------------------------*/
#ifndef STM32_SPI_MASTER_DMA_HPP_
#define STM32_SPI_MASTER_DMA_HPP_
/*----------------------------------------------------------*/
#include "stm32_spi_master.hpp"


/*----------------------------------------------------------*/
namespace stm32 {
namespace drv {

/*----------------------------------------------------------*/

#if defined(STM32MCU_MAJOR_TYPE_F1)
extern "C" {
#if (CONFIG_ISIXDRV_SPI_SUPPORTED_DEVS & ISIX_DRV_SPI_SPI1_ENABLE) && CONFIG_ISIXDRV_SPI_ENABLE_DMAIRQ
	void dma1_channel2_isr_vector(void) __attribute__((__interrupt__));
	void dma1_channel3_isr_vector(void) __attribute__((__interrupt__));
#endif
}
#endif

/*----------------------------------------------------------*/
class spi_master_dma : public spi_master
{

#if defined(STM32MCU_MAJOR_TYPE_F1)
#if (CONFIG_ISIXDRV_SPI_SUPPORTED_DEVS & ISIX_DRV_SPI_SPI1_ENABLE) && CONFIG_ISIXDRV_SPI_ENABLE_DMAIRQ
	friend void dma1_channel2_isr_vector(void);
	friend void dma1_channel3_isr_vector(void);
#endif
#endif
	static const int IRQPRIO = 1;
	static const int IRQSUB =  7;
	enum irqs_no
	{
		irqs_tc_rx,
		irqs_tc_tx,
		irqs_te_rx,
		irqs_te_tx,
		irqs_rxmode
	};
	static const uint32_t irqs_err_msk =  (1<<irqs_te_rx)|(1<<irqs_te_tx);
	static const uint32_t irqs_rxtc_msk = (1<<irqs_tc_rx)|(1<<irqs_tc_tx);
	static const uint32_t irqs_txtc_msk = (1<<irqs_tc_tx);
public:
	//Constructor
	spi_master_dma(SPI_TypeDef *spi, unsigned pclk1, unsigned pclk2);
	/* Destructor */
	virtual ~spi_master_dma();
	/* Write to the device */
	virtual int write( const void *buf, size_t len);
	/* Read from the device */
	virtual int read ( void *buf, size_t len)
	{
		return spi_master_dma::transfer( buf, NULL, len );
	}
	/* Transfer (BIDIR) */
	virtual int transfer( const void *inbuf, void *outbuf, size_t len );
private:
#if CONFIG_ISIXDRV_SPI_ENABLE_DMAIRQ
	//Handle DMA TX IRQ
	void handle_isr( irqs_no reason );
private:
	isix::semaphore m_notify_sem;
	uint32_t m_irq_flags;
#endif
};
/*----------------------------------------------------------*/
} /* namespace drv */
} /* namespace stm32 */
/*----------------------------------------------------------*/
#endif /* STM32_SPI_MASTER_DMA_HPP_ */
/*----------------------------------------------------------*/
