/*
 * mmc_host_sdio_stm32.hpp
 *
 *  Created on: 01-12-2012
 *      Author: lucck
 */
/*----------------------------------------------------------*/
#ifndef STM32_SDIO_MMC_HOST_HPP_
#define STM32_SDIO_MMC_HOST_HPP_
/*----------------------------------------------------------*/
#include "mmc/mmc_host.hpp"
#include "spi_device.hpp"
#include <stdint.h>
#include <isix.h>
#ifdef _HAVE_CONFIG_H
#include "config.h"
#endif

/*----------------------------------------------------------*/
#define ISIX_SDDRV_TRANSFER_USE_IRQ (1<<0)

/*----------------------------------------------------------*/
#ifndef ISIX_SDDRV_TRANSFER_MODE
#define ISIX_SDDRV_TRANSFER_MODE ISIX_SDDRV_TRANSFER_USE_IRQ
#endif
/*----------------------------------------------------------*/
namespace stm32 {
namespace drv {
/*----------------------------------------------------------*/

#if(ISIX_SDDRV_TRANSFER_MODE & ISIX_SDDRV_TRANSFER_USE_IRQ)
extern "C" {
	void __attribute__((__interrupt__)) sdio_isr_vector( void );
	void __attribute__((__interrupt__)) dma2_stream3_isr_vector( void );
}
#endif
/*----------------------------------------------------------*/
class mmc_host_sdio : public ::drv::mmc::mmc_host
{
#if(ISIX_SDDRV_TRANSFER_MODE & ISIX_SDDRV_TRANSFER_USE_IRQ)
	friend void sdio_isr_vector(void);
	friend void dma2_stream3_isr_vector(void);
	friend void dma2_stream6_isr_vector(void);
#endif
public:
	//Constructor
	mmc_host_sdio( unsigned pclk2, int spi_speed_limit_khz=0 );
	//Destructor
	virtual ~mmc_host_sdio();
	//Execute MMC command
	virtual int execute_command( ::drv::mmc::mmc_command &req, unsigned timeout );
	//Execute MMC data transfer
	virtual int send_data( const void *buf, size_t len, unsigned timeout );
	//Execute MMC data transfer
	virtual int receive_data( void *buf, size_t len, unsigned timeout );
	//Execute IO config
	virtual int set_ios( ios_cmd cmd, int param );
	//Get capabilities
	virtual unsigned get_capabilities() const
	{
		return mmc_host::cap_1bit|mmc_host::cap_4bit|mmc_host::cap_hs;
	}
	//Prepare for receive data
	virtual int receive_data_prep(void* /*buf*/,  size_t /*len*/, unsigned /*timeout*/ );
private:
	inline void sdio_reinit( mmc_host::bus_width bus_width, int khz );
private:
	static const unsigned IRQ_PRIO = 1;
	static const unsigned IRQ_SUB  = 7;
private:
	const unsigned m_pclk2;
	const unsigned short m_spi_speed_limit_khz;
#if(ISIX_SDDRV_TRANSFER_MODE & ISIX_SDDRV_TRANSFER_USE_IRQ)
	void process_dma_irq();
	void process_irq();
	isix::semaphore m_complete;
	enum
	{
		bf_transfer_end,
		bf_dma_complete
	};
	uint32_t m_flags;
	int m_transfer_error;
#endif
};
/*----------------------------------------------------------*/
} /* namespace drv */
}
/*----------------------------------------------------------*/
#endif /* MMC_HOST_SDIO_STM32_HPP_ */
/*----------------------------------------------------------*/
