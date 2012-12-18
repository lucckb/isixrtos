/*
 * stm32_sdio_mmc_host.cpp
 *
 *  Created on: 14-12-2012
 *      Author: lucck
 */
/*----------------------------------------------------------*/
#include "mmc/mmc_command.hpp"
#include "stm32_sdio_mmc_host.hpp"
#include <dbglog.h>
#include <stm32dma.h>
#include <stm32sdio.h>
#include <stm32gpio.h>
#include <stm32system.h>
#include <stm32bitbang.h>
/*----------------------------------------------------------*/
namespace stm32 {
namespace drv {
/*----------------------------------------------------------*/
namespace
{
	//Convert KHZ to SDIO divide
	inline uint8_t khz_to_sdio_div( unsigned khz, unsigned pclk2 )
	{
		int div;
		if( khz == 0 ) div = 0xff;
		else div = ((pclk2/2) / (1000*khz) )  - 2;
		if( div > 0xff) div = 0xff;
		else if( div < 0 ) div = 0;
		dbprintf("clkdiv khz: %u div: %u", khz, div);
		return div;
	}
	//Check for timer elapsed
	inline bool timer_elapsed(isix::tick_t t1, isix::tick_t timeout)
	{
		isix::tick_t t2 = isix::isix_get_jiffies();
		if( t2 >= t1) 	//Not overflow
			return t2 - t1 > timeout;
		 else   	       //Overflow
			return t1 - t2 > timeout;
	}
	//LOW clock freq
	const unsigned C_low_clk_khz_host = 400;
	//Static flags wait for cmd
	static const uint32_t SDIO_STATIC_FLAGS = 0x000005FF;
	//C block size
	const size_t C_block_len = 512;

}

/*----------------------------------------------------------*/
#if (defined(STM32MCU_MAJOR_TYPE_F4) || defined(STM32MCU_MAJOR_TYPE_F2))
// Config defs

//USE dma stream number valid no are 3 or 6
#ifndef SDDRV_DMA_STREAM_NO
#define SDDRV_DMA_STREAM_NO 3
#endif

// Config defs end
#define _SD_SDIO_cat(x, y) x##y
#define _SD_SDIO_cat3(x, y, z) x##y##z
#define _SD_SDIO_STREAM_prv(x, y)  _SD_SDIO_cat(x, y)
#define _SD_SDIO_STREAM_prv3(x, y, z)  _SD_SDIO_cat3(x, y, z)
#define _SD_SDIO_ISR_VECT_expand(x, y, z ) _SD_SDIO_cat3(x, y, z)
#define _SD_SDIO_isr_dma_vector _SD_SDIO_ISR_VECT_expand(dma2_stream,SDDRV_DMA_STREAM_NO,_isr_vector)

#define SD_SDIO_DMA_FLAG_TCIF         _SD_SDIO_STREAM_prv( DMA_FLAG_TCIF, SDDRV_DMA_STREAM_NO )
#define SD_SDIO_DMA_FLAG_FEIF         _SD_SDIO_STREAM_prv( DMA_FLAG_FEIF, SDDRV_DMA_STREAM_NO )
#define SD_SDIO_DMA_STREAM            _SD_SDIO_STREAM_prv( DMA2_Stream, SDDRV_DMA_STREAM_NO )
#define SD_SDIO_DMA_FLAG_HTIF         _SD_SDIO_STREAM_prv( DMA_FLAG_HTIF, SDDRV_DMA_STREAM_NO )
#define SD_SDIO_DMA_FLAG_DMEIF        _SD_SDIO_STREAM_prv( DMA_FLAG_DMEIF, SDDRV_DMA_STREAM_NO )
#define SD_SDIO_DMA_FLAG_TEIF         _SD_SDIO_STREAM_prv( DMA_FLAG_TEIF, SDDRV_DMA_STREAM_NO )
#define SD_SDIO_DMA_CHANNEL           DMA_Channel_4
#define SD_SDIO_DMA_IRQn              _SD_SDIO_STREAM_prv3(DMA2_Stream,SDDRV_DMA_STREAM_NO,_IRQn)
#define SDIO_FIFO_ADDRESS             ((void*)0x40012C80)
#if (SDDRV_DMA_STREAM_NO<=3)
#define SD_SDIO_DMA_SR DMA2->LISR
#else
#define SD_SDIO_DMA_SR DMA2->HISR
#endif

namespace
{
	/**
	  * @brief  Configures the DMA2 Channel4 for SDIO Tx request.
	  * @param  BufferSRC: pointer to the source buffer
	  * @param  BufferSize: buffer size
	  * @retval None
	  */
	void sd_lowlevel_dma_tx_config(const void *buffer_src, uint32_t buf_size)
	{
	   //DMA transfer len managed by HW
	   (void)buf_size;
	  dma_clear_flag(SD_SDIO_DMA_STREAM, SD_SDIO_DMA_FLAG_FEIF | SD_SDIO_DMA_FLAG_DMEIF |
			  SD_SDIO_DMA_FLAG_TEIF | SD_SDIO_DMA_FLAG_HTIF | SD_SDIO_DMA_FLAG_TCIF);
	  /* DMA2 Stream3  or Stream6 disable */
	  dma_cmd(SD_SDIO_DMA_STREAM, DISABLE);
	  /* DMA2 Stream3  or Stream6 Config */
	  dma_deinit( SD_SDIO_DMA_STREAM );
	  dma_init( SD_SDIO_DMA_STREAM , DMA_DIR_MemoryToPeripheral | DMA_PeripheralInc_Disable |
				SD_SDIO_DMA_CHANNEL |DMA_MemoryInc_Enable | DMA_PeripheralDataSize_Word | DMA_MemoryDataSize_Word |
				DMA_Mode_Normal | DMA_Priority_VeryHigh | DMA_MemoryBurst_INC4| DMA_PeripheralBurst_INC4,
				DMA_FIFOMode_Enable | DMA_FIFOThreshold_Full, 0, SDIO_FIFO_ADDRESS, (uint32_t*)buffer_src );
#if (ISIX_SDDRV_TRANSFER_MODE & ISIX_SDDRV_TRANSFER_USE_IRQ)
	  dma_it_config(SD_SDIO_DMA_STREAM, DMA_IT_TC, ENABLE);
#endif
	  dma_flow_controller_config(SD_SDIO_DMA_STREAM, DMA_FlowCtrl_Peripheral);
	  /* DMA2 Stream3  or Stream6 enable */
	  dma_cmd(SD_SDIO_DMA_STREAM, ENABLE);
	}
	/*--------------------------------------------------------------------*/
	/**
	  * @brief  Configures the DMA2 Channel4 for SDIO Rx request.
	  * @param  BufferDST: pointer to the destination buffer
	  * @param  BufferSize: buffer size
	  * @retval None
	  */
	void sd_lowlevel_dma_rx_config(void *buf_dst, uint32_t buf_size)
	{
	  //DMA transfer len managed by HW
	  (void)buf_size;
	  dma_clear_flag(SD_SDIO_DMA_STREAM, SD_SDIO_DMA_FLAG_FEIF | SD_SDIO_DMA_FLAG_DMEIF | SD_SDIO_DMA_FLAG_TEIF | SD_SDIO_DMA_FLAG_HTIF | SD_SDIO_DMA_FLAG_TCIF);
	  /* DMA2 Stream3  or Stream6 disable */
	  dma_cmd(SD_SDIO_DMA_STREAM, DISABLE);
	  /* DMA2 Stream3 or Stream6 Config */
	  dma_deinit(SD_SDIO_DMA_STREAM);
	  dma_init(SD_SDIO_DMA_STREAM, SD_SDIO_DMA_CHANNEL | DMA_DIR_PeripheralToMemory | DMA_PeripheralInc_Disable |
			  DMA_MemoryInc_Enable | DMA_PeripheralDataSize_Word | DMA_MemoryDataSize_Word |
			  DMA_Mode_Normal | DMA_Priority_Medium | DMA_MemoryBurst_INC4 | DMA_PeripheralBurst_INC4 ,
			  DMA_FIFOMode_Enable | DMA_FIFOThreshold_Full, 0, SDIO_FIFO_ADDRESS,  buf_dst );
#if (ISIX_SDDRV_TRANSFER_MODE & ISIX_SDDRV_TRANSFER_USE_IRQ)
	  dma_it_config(SD_SDIO_DMA_STREAM, DMA_IT_TC, ENABLE);
#endif
	  dma_flow_controller_config(SD_SDIO_DMA_STREAM, DMA_FlowCtrl_Peripheral);
	  /* DMA2 Stream3 or Stream6 enable */
	  dma_cmd(SD_SDIO_DMA_STREAM, ENABLE);
	}
}	/* Unnamed namespace */
#endif
/*----------------------------------------------------------*/
#if (ISIX_SDDRV_TRANSFER_MODE & ISIX_SDDRV_TRANSFER_USE_IRQ)
namespace		//Private namespace for IRQ handling
{
	mmc_host_sdio *p_sdio;
}
#endif

/*----------------------------------------------------------*/
#if (ISIX_SDDRV_TRANSFER_MODE & ISIX_SDDRV_TRANSFER_USE_IRQ)
//Process dma IRQ
void mmc_host_sdio::process_dma_irq()
{
	setBit_BB( &m_flags, bf_dma_complete );
	m_complete.signal_isr();
}
#endif
/*----------------------------------------------------------*/
#if (ISIX_SDDRV_TRANSFER_MODE & ISIX_SDDRV_TRANSFER_USE_IRQ)
//Process irq
void mmc_host_sdio::process_irq()
{
	 using namespace ::drv::mmc;
	 using namespace stm32;
	 if( sdio_get_it_status(SDIO_IT_CEATAEND) != RESET )
	 {
		 setBit_BB( &m_flags, bf_cmd_end );
	 }
	 else if (sdio_get_it_status(SDIO_IT_DATAEND) != RESET)
	 {
	    m_transfer_error = MMC_OK;
	    sdio_clear_it_pending_bit(SDIO_IT_DATAEND);
	    setBit_BB( &m_flags, bf_transfer_end );
	 }
	 else if (sdio_get_it_status(SDIO_IT_DCRCFAIL) != RESET)
	 {
		sdio_clear_it_pending_bit(SDIO_IT_DCRCFAIL);
	    m_transfer_error = MMC_DATA_CRC_FAIL;
	 }
	 else if (sdio_get_it_status(SDIO_IT_DTIMEOUT) != RESET)
	 {
		sdio_clear_it_pending_bit(SDIO_IT_DTIMEOUT);
		m_transfer_error = MMC_DATA_TIMEOUT;
	 }
	 else if (sdio_get_it_status(SDIO_IT_RXOVERR) != RESET)
	 {
		sdio_clear_it_pending_bit(SDIO_IT_RXOVERR);
		m_transfer_error = MMC_RX_OVERRUN;
	 }
	 else if (sdio_get_it_status(SDIO_IT_TXUNDERR) != RESET)
	 {
		sdio_clear_it_pending_bit(SDIO_IT_TXUNDERR);
		m_transfer_error = MMC_TX_UNDERRUN;
	 }
	 else if (sdio_get_it_status(SDIO_IT_STBITERR) != RESET)
	 {
		sdio_clear_it_pending_bit(SDIO_IT_STBITERR);
		m_transfer_error = MMC_START_BIT_ERR;
	 }
	 sdio_it_config(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND |
	               SDIO_IT_TXFIFOHE | SDIO_IT_RXFIFOHF | SDIO_IT_TXUNDERR |
	               SDIO_IT_RXOVERR | SDIO_IT_STBITERR, DISABLE);
	 m_complete.signal_isr();
}
#endif
/*----------------------------------------------------------*/
#if (ISIX_SDDRV_TRANSFER_MODE & ISIX_SDDRV_TRANSFER_USE_IRQ)
extern "C"
{
	void __attribute__((__interrupt__)) _SD_SDIO_isr_dma_vector( void )
	{
		if(SD_SDIO_DMA_SR & SD_SDIO_DMA_FLAG_TCIF)
		{
		    dma_clear_flag(SD_SDIO_DMA_STREAM, SD_SDIO_DMA_FLAG_TCIF|SD_SDIO_DMA_FLAG_FEIF);
		    if(p_sdio) p_sdio->process_dma_irq();
		}
	}
	void __attribute__((__interrupt__)) sdio_isr_vector( void )
	{
		if(p_sdio) p_sdio->process_irq();
	}
}
#endif
/*----------------------------------------------------------*/
//Constructor
mmc_host_sdio::mmc_host_sdio( unsigned pclk2, int spi_speed_limit_khz )
	: m_pclk2(pclk2), m_spi_speed_limit_khz(spi_speed_limit_khz)
#if(ISIX_SDDRV_TRANSFER_MODE & ISIX_SDDRV_TRANSFER_USE_IRQ)
	, m_complete( 0, 1 ), m_flags(0), m_transfer_error(0)
#endif
{
	  using namespace stm32;
	  /* SDIO Peripheral Low Level Init */
	  rcc_ahb1_periph_clock_cmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD, true);
#if defined(STM32MCU_MAJOR_TYPE_F4) || defined(STM32MCU_MAJOR_TYPE_F2)
	  gpio_pin_AF_config(GPIOC, GPIO_PinSource8, GPIO_AF_SDIO);
	  gpio_pin_AF_config(GPIOC, GPIO_PinSource9, GPIO_AF_SDIO);
	  gpio_pin_AF_config(GPIOC, GPIO_PinSource10, GPIO_AF_SDIO);
	  gpio_pin_AF_config(GPIOC, GPIO_PinSource11, GPIO_AF_SDIO);
	  gpio_pin_AF_config(GPIOC, GPIO_PinSource12, GPIO_AF_SDIO);
	  gpio_pin_AF_config(GPIOD, GPIO_PinSource2, GPIO_AF_SDIO);
	  gpio_config_ext( GPIOC, gpioPIN(8)|gpioPIN(9)|gpioPIN(10)|gpioPIN(11),
			  GPIO_MODE_ALTERNATE, GPIO_PUPD_PULLUP, GPIO_SPEED_25MHZ, GPIO_OTYPE_PP );
	  gpio_config( GPIOD, 2, GPIO_MODE_ALTERNATE, GPIO_PUPD_PULLUP, GPIO_SPEED_25MHZ, GPIO_OTYPE_PP );
	  gpio_config( GPIOC, 12, GPIO_MODE_ALTERNATE, GPIO_PUPD_NONE, GPIO_SPEED_25MHZ, GPIO_OTYPE_PP );
	  //Configure DMA
	  rcc_ahb1_periph_clock_cmd( RCC_AHB1Periph_DMA2, true );
#else
#error F1 not implemented yet
#endif
	  sdio_deinit();
	  rcc_apb2_periph_clock_cmd( RCC_APB2Periph_SDIO, true );
#if(ISIX_SDDRV_TRANSFER_MODE & ISIX_SDDRV_TRANSFER_USE_IRQ)
	  p_sdio = this;
	  nvic_set_priority( SDIO_IRQn, IRQ_PRIO, IRQ_SUB );
	  nvic_irq_enable( SDIO_IRQn, true );
	  nvic_set_priority( SD_SDIO_DMA_IRQn , IRQ_PRIO, IRQ_SUB );
	  nvic_irq_enable( SD_SDIO_DMA_IRQn , true );
#endif
	 sdio_dma_cmd(ENABLE);
}
/*----------------------------------------------------------*/
//Destructor
mmc_host_sdio::~mmc_host_sdio()
{
	/*!< Disable SDIO Clock */
	sdio_clock_cmd(DISABLE);
	/*!< Set Power State to OFF */
	sdio_set_power_state(SDIO_PowerState_OFF);
	/*!< DeInitializes the SDIO peripheral */
	sdio_deinit();
	/* Disable the SDIO APB2 Clock */
	rcc_apb2_periph_clock_cmd(RCC_APB2Periph_SDIO, DISABLE);
#if defined(STM32MCU_MAJOR_TYPE_F4) || defined(STM32MCU_MAJOR_TYPE_F2)
	rcc_ahb1_periph_clock_cmd( RCC_AHB1Periph_DMA2, DISABLE );
	/* Deactivate the GPIO ports */
	gpio_pin_AF_config( GPIOC, GPIO_PinSource8,  GPIO_AF_MCO );
	gpio_pin_AF_config( GPIOC, GPIO_PinSource9,  GPIO_AF_MCO );
	gpio_pin_AF_config( GPIOC, GPIO_PinSource10, GPIO_AF_MCO );
	gpio_pin_AF_config( GPIOC, GPIO_PinSource11, GPIO_AF_MCO );
	gpio_pin_AF_config( GPIOC, GPIO_PinSource12, GPIO_AF_MCO );
	gpio_pin_AF_config( GPIOD, GPIO_PinSource2,  GPIO_AF_MCO );
	gpio_config_ext( GPIOC, gpioPIN(8)|gpioPIN(9)|gpioPIN(10)|gpioPIN(11)|gpioPIN(12),
			GPIO_MODE_INPUT, GPIO_PUPD_NONE, 0, 0 );
	gpio_config( GPIOC, 2, GPIO_MODE_INPUT, GPIO_PUPD_NONE, 0, 0 );
#else
#error F1 not implemented yet
#endif
#if(ISIX_SDDRV_TRANSFER_MODE & ISIX_SDDRV_TRANSFER_USE_IRQ)
	 nvic_irq_enable( SDIO_IRQn, false );
	 nvic_irq_enable( SD_SDIO_DMA_IRQn, false );
	 p_sdio = NULL;
#endif
}

/*----------------------------------------------------------*/
//Execute MMC command
int mmc_host_sdio::execute_command( ::drv::mmc::mmc_command &req, unsigned timeout )
{
	//TODO: ADD for support R1B command
	using namespace ::drv::mmc;
	uint32_t resp = SDIO_Response_No;
	if( req.get_flags() & mmc_command::resp_present )
	{
		if( req.get_flags() & mmc_command::resp_136 )
		{
			//dbprintf("SDIO_Response_Long");
			resp = SDIO_Response_Long;
		}
		else
		{
			//dbprintf("SDIO_Response_Short");
			resp = SDIO_Response_Short;
		}
	}
	//Send command
	uint32_t fwait=  SDIO_Wait_No;
	if(req.get_flags() & mmc_command::resp_busy ) fwait = SDIO_Wait_IT;

	stm32::sdio_send_command( req.get_arg(), req.get_op()&0x7f,
			resp, fwait, SDIO_CPSM_Enable );
	uint32_t stat;
	if( req.get_flags() & mmc_command::resp_present )
		stat = SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT|SDIO_FLAG_CCRCFAIL;
	else
		stat = SDIO_FLAG_CMDSENT;
	isix::tick_t t_start = isix::isix_get_jiffies();
	timeout = isix::isix_ms2tick( timeout );
	uint32_t sreg;
	int ret = MMC_OK;
	do
	{
		sreg = SDIO->STA;
		if(timer_elapsed( t_start, timeout) )
			break;
	}
	while( !(sreg & stat) );
	//dbprintf("FLAG ERROR %08lx", SDIO->STA );
	//Check the response
	do {
		//Check the errors
		if( !(sreg & stat) )
		{
			ret =  MMC_CMD_RSP_TIMEOUT;
			dbprintf("timeout #1");
			break;
		}
		if( (sreg & stat) & SDIO_FLAG_CTIMEOUT )
		{
			ret =  MMC_CMD_RSP_TIMEOUT;
			dbprintf("HW timeout cmd %i arg %08x", req.get_op(), req.get_arg());
			break;
		}
		if( (sreg & stat) & SDIO_FLAG_CCRCFAIL )
		{
			if( req.get_flags()&mmc_command::resp_crc )
			{
				dbprintf("CRC fail");
				ret = MMC_CMD_CRC_FAIL;
				break;
			}
		}
		//Check the command response
		if( req.get_flags() & mmc_command::resp_opcode )
		{
			if (stm32::sdio_get_command_response() != ((req.get_op())&0x7f) )
			{
				ret = MMC_ILLEGAL_CMD;
				break;
			}
		}
		if( req.get_flags() & mmc_command::resp_present )
		{
			if( !(req.get_flags()&mmc_command::resp_136) )
				req.set_resp_short( sdio_get_response(SDIO_RESP1) );
			else
			{
				req.set_resp_long( sdio_get_response(SDIO_RESP1), sdio_get_response(SDIO_RESP2),
					sdio_get_response(SDIO_RESP3), sdio_get_response(SDIO_RESP4) );
			}
		}
		else
		{
			req.set_resp_short(0);
		}
	} while(0);
	sdio_clear_flag(SDIO_STATIC_FLAGS & stat );
	//dbprintf("Exec cmd ret: %i op: %i arg: %08x CEA %02x", ret, req.get_op()&(~0x80), req.get_arg(),m_flags);
	return ret;
}
/*----------------------------------------------------------*/
//Execute MMC data transfer
int mmc_host_sdio::send_data( const void *buf, size_t len, unsigned timeout )
{
	using namespace stm32;
	using namespace ::drv::mmc;
	uint32_t block_size;
	if( len % C_block_len == 0 )
	{
		block_size = SDIO_DataBlockSize_512b;
	}
	else
	{
		if( len > C_block_len ||  __builtin_popcount(len) != 1)
		{
			return err_invalid_parameter;
		}
		block_size = (__builtin_ffs(len) - 1)  << 4;
	}
	m_transfer_error = MMC_OK;
	//dbprintf("Timeout %d", timeout);
	timeout = (m_pclk2/2/8/10000) * timeout;
	//dbprintf("DataWR config Tout=%u len=%u bs=0x%02lX", timeout, len, block_size);

#if(ISIX_SDDRV_TRANSFER_MODE & ISIX_SDDRV_TRANSFER_USE_IRQ)
	 sdio_it_config(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR
			 |SDIO_IT_CEATAEND, ENABLE);
#endif
	sdio_data_config( timeout, len, block_size, SDIO_TransferDir_ToCard,
					SDIO_TransferMode_Block, SDIO_DPSM_Enable );
	sd_lowlevel_dma_tx_config(buf, len);
	/* Wait for RCV DATA
	 *
	 */
	int ret = MMC_OK;
		do
		{
			//Wait for complete sem
			if ((ret = m_complete.wait( timeout ))) break;
			while (SDIO->STA & SDIO_FLAG_TXACT)
			{
				//dbprintf("Tutaj chujnia");
				//ret = MMC_DATA_TIMEOUT;
			//	break;
			}
		} while(0);

		sdio_clear_flag( SDIO_STATIC_FLAGS );
		if( !ret )
		{
			ret = m_transfer_error;
		}
		//dbprintf("WRITE data STAT %i", ret);
		return ret;
}
/*----------------------------------------------------------*/
//Prepare for receive data
int mmc_host_sdio::receive_data_prep( void* buf, size_t len, unsigned timeout)
{
	using namespace stm32;
	using namespace ::drv::mmc;
	uint32_t block_size;
	if( len % C_block_len == 0 )
	{
		block_size = SDIO_DataBlockSize_512b;
	}
	else
	{
		if( len > C_block_len ||  __builtin_popcount(len) != 1)
		{
			return err_invalid_parameter;
		}
		block_size = (__builtin_ffs(len) - 1)  << 4;
	}
	m_transfer_error = MMC_OK;
	//dbprintf("Timeout %d", timeout);
	timeout = (m_pclk2/2/8/10000) * timeout;
	//dbprintf("Data config Tout=%u len=%u bs=0x%02lX", timeout, len, block_size);
	sdio_data_config( timeout, len, block_size, SDIO_TransferDir_ToSDIO, SDIO_TransferMode_Block, SDIO_DPSM_Enable );
#if(ISIX_SDDRV_TRANSFER_MODE & ISIX_SDDRV_TRANSFER_USE_IRQ)
	sdio_it_config(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR
			 |SDIO_IT_CEATAEND, ENABLE);
#endif
	sdio_dma_cmd(ENABLE);
	sd_lowlevel_dma_rx_config( buf, len );
	//Wait for data transfer
	return MMC_OK;
}
/*----------------------------------------------------------*/
//Execute MMC data transfer
int mmc_host_sdio::receive_data( void *buf, size_t len, unsigned timeout )
{
	using namespace stm32;
	using namespace ::drv::mmc;
	int ret = MMC_OK;
	do
	{
		//Wait for complete sem
		if ((ret = m_complete.wait( timeout ))) break;
		while (SDIO->STA & SDIO_FLAG_RXACT)
		{
			//ret = MMC_DATA_TIMEOUT;
			//break;
		}
	} while(0);
	sdio_clear_flag( SDIO_STATIC_FLAGS );
	if( !ret )
	{
		ret = m_transfer_error;
	}
	//dbprintf("Receive data STAT %i", ret);
	return ret;
}
/*----------------------------------------------------------*/
//Execute IO config
int mmc_host_sdio::set_ios( mmc_host::ios_cmd cmd, int param )
{
	using namespace stm32;
	int ret = err_OK;
	switch( cmd )
	{
	//PWR OFF
	case mmc_host::ios_pwr_off:
	{
		sdio_set_power_state(SDIO_PowerState_OFF);
		break;
	}
	//PWR ON
	case mmc_host::ios_pwr_on:
	{
		isix::isix_wait_ms( 5 );
		 //Initialize the SDIO ifc
		sdio_init( SDIO_ClockEdge_Rising, SDIO_ClockBypass_Disable, SDIO_ClockPowerSave_Disable,
			SDIO_BusWide_1b, SDIO_HardwareFlowControl_Disable, khz_to_sdio_div( C_low_clk_khz_host, m_pclk2)
		);
		/*!< Set Power State to ON */
		sdio_set_power_state(SDIO_PowerState_ON);
		for(int w=0; w<16; w++ ) nop();
		/*!< Enable SDIO Clock */
		sdio_clock_cmd(ENABLE);
		dbprintf("Power on");
		break;
	}
	//SET SPEED
	case mmc_host::ios_set_speed:
	{
		if( m_spi_speed_limit_khz && param > m_spi_speed_limit_khz )
			param = m_spi_speed_limit_khz;
		uint32_t cr = SDIO->CLKCR;
		cr = (cr & ~0xff) | khz_to_sdio_div(param, m_pclk2);
		SDIO->CLKCR = cr;
		break;
	}
	//SET bus wide
	case mmc_host::ios_set_bus_with:
	{
		uint32_t cr = SDIO->CLKCR;
		cr = (cr & ~0x1800) | (param==mmc_host::bus_width_4b?SDIO_BusWide_4b:SDIO_BusWide_1b);
		SDIO->CLKCR = cr;
		break;
	}
	//UNSUPORTED FEATURE
	default:
		ret = err_not_supported;
		break;
	}
	return ret;
}

/*----------------------------------------------------------*/
}}
/*----------------------------------------------------------*/
