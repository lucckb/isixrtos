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
/*----------------------------------------------------------*/
#if defined(CONFIG_ISIX_DRV_SPI_ENABLE_DMA_MASK) && (CONFIG_ISIX_DRV_SPI_ENABLE_DMA_MASK!=0)

/*----------------------------------------------------------*/
namespace stm32 {
namespace drv {

/*----------------------------------------------------------*/
#if defined(STM32MCU_MAJOR_TYPE_F1)
extern "C" {
#if CONFIG_ISIX_DRV_SPI_ENABLE_DMA_MASK==ISIX_DRV_SPI_DMA_SPI1_ENABLE
	void dma1_channel2_isr_vector(void) __attribute__((__interrupt__));
	void dma1_channel3_isr_vector(void) __attribute__((__interrupt__));
#endif
}
#endif

/*----------------------------------------------------------*/
class spi_master_dma : public spi_master
{
#if defined(STM32MCU_MAJOR_TYPE_F1)
#if CONFIG_ISIX_DRV_SPI_ENABLE_DMA_MASK==ISIX_DRV_SPI_DMA_SPI1_ENABLE
	friend void dma1_channel2_isr_vector(void);
	friend void dma1_channel3_isr_vector(void);
#endif
#endif
	static const int IRQPRIO = 1;
	static const int IRQSUB =  7;
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
	//Handle DMA TX IRQ
	void handle_dma_tx_isr();
	//Handle DMA RX IRQ
	void handle_dma_rx_isr();
private:
	isix::semaphore m_notify_sem;
};
/*----------------------------------------------------------*/
} /* namespace drv */
} /* namespace stm32 */
/*----------------------------------------------------------*/
#endif /*defined(CONFIG_ISIX_DRV_SPI_ENABLE_DMA_MASK) && (CONFIG_ISIX_DRV_SPI_ENABLE_DMA_MASK!=0)*/
#endif /* STM32_SPI_MASTER_DMA_HPP_ */
/*----------------------------------------------------------*/
