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

#define ISIX_DRV_SPI_DMA_SPI1_ENABLE (1<<0)
#define ISIX_DRV_SPI_DMA_SPI2_ENABLE (1<<1)
#define ISIX_DRV_SPI_DMA_SPI3_ENABLE (1<<2)
#ifndef ISIX_DRV_SPI_DMA_WITH_IRQ
#define ISIX_DRV_SPI_DMA_WITH_IRQ 1
#endif

/*----------------------------------------------------------*/
#if defined(CONFIG_ISIX_DRV_SPI_ENABLE_DMA_MASK) && (CONFIG_ISIX_DRV_SPI_ENABLE_DMA_MASK!=0)

/*----------------------------------------------------------*/
namespace stm32 {
namespace drv {

/*----------------------------------------------------------*/
#if ISIX_DRV_SPI_DMA_WITH_IRQ
#if defined(STM32MCU_MAJOR_TYPE_F1)
extern "C" {
#if CONFIG_ISIX_DRV_SPI_ENABLE_DMA_MASK==ISIX_DRV_SPI_DMA_SPI1_ENABLE
	void dma1_channel2_isr_vector(void) __attribute__((__interrupt__));
	void dma1_channel3_isr_vector(void) __attribute__((__interrupt__));
#endif
}
#endif
#endif
/*----------------------------------------------------------*/
class spi_master_dma : public spi_master
{
#if ISIX_DRV_SPI_DMA_WITH_IRQ
#if defined(STM32MCU_MAJOR_TYPE_F1)
#if CONFIG_ISIX_DRV_SPI_ENABLE_DMA_MASK==ISIX_DRV_SPI_DMA_SPI1_ENABLE
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
#endif
public:
#if(!CONFIG_ISIX_DRV_SPI_SUPPORTED_DEVS)
	spi_master_dma(SPI_TypeDef *spi, unsigned pclk1, unsigned pclk2);
#else
	spi_master_dma();
#endif
	/* Destructor */
	virtual ~spi_master_dma();
	/* Write to the device */
	virtual int write( const void *buf, size_t len);
	/* Read from the device */
	virtual int read ( void *buf, size_t len);
	/* Transfer (BIDIR) */
	virtual int transfer( const void *inbuf, void *outbuf, size_t len  );
private:
#if ISIX_DRV_SPI_DMA_WITH_IRQ
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
#endif /*defined(CONFIG_ISIX_DRV_SPI_ENABLE_DMA_MASK) && (CONFIG_ISIX_DRV_SPI_ENABLE_DMA_MASK!=0)*/
#endif /* STM32_SPI_MASTER_DMA_HPP_ */
/*----------------------------------------------------------*/
