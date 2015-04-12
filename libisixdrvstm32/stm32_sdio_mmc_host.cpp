/*
 * stm32_sdio_mmc_host.cpp
 *
 *  Created on: 14-12-2012
 *      Author: lucck
 */
/*----------------------------------------------------------*/
#include "mmc/mmc_command.hpp"
#include "stm32_sdio_mmc_host.hpp"
#include <stm32dma.h>
#include <stm32sdio.h>
#include <stm32gpio.h>
#include <stm32system.h>
#include <stm32bitbang.h>
#include <stm32exti.h>

/*----------------------------------------------------------*/
#ifdef DEBUG_STM32_SDIO_MMC_HOST_CPP
#include <dbglog.h>
#else
#define dbprintf(...) do {} while(0)
#endif
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
		return div;
	}
	//Check for timer elapsed
	inline bool timer_elapsed(ostick_t t1, ostick_t timeout)
	{
		ostick_t t2 = isix_get_jiffies();
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
	//DAT LIne port and dat line PIN
	GPIO_TypeDef * const DATA0_PORT = GPIOC;
	const unsigned DATA0_PIN = 8;
}
/*----------------------------------------------------------*/
#if (defined(STM32MCU_MAJOR_TYPE_F4) || defined(STM32MCU_MAJOR_TYPE_F2))
// Config defs

//USE dma stream number valid no are 3 or 6
#ifndef CONFIG_SDDRV_DMA_STREAM_NO
#define CONFIG_SDDRV_DMA_STREAM_NO 3
#endif
//! Initialize or not dma channel by default yes
#ifndef CONFIG_SDDRV_INIT_DMA_GPIO_CLKS
#define CONFIG_SDDRV_INIT_DMA_GPIO_CLKS 1
#endif 
// Config defs end
#define _SD_SDIO_cat(x, y) x##y
#define _SD_SDIO_cat3(x, y, z) x##y##z
#define _SD_SDIO_STREAM_prv(x, y)  _SD_SDIO_cat(x, y)
#define _SD_SDIO_STREAM_prv3(x, y, z)  _SD_SDIO_cat3(x, y, z)


#define SD_SDIO_DMA_FLAG_TCIF         _SD_SDIO_STREAM_prv( DMA_FLAG_TCIF, CONFIG_SDDRV_DMA_STREAM_NO )
#define SD_SDIO_DMA_FLAG_FEIF         _SD_SDIO_STREAM_prv( DMA_FLAG_FEIF, CONFIG_SDDRV_DMA_STREAM_NO )
#define SD_SDIO_DMA_STREAM            _SD_SDIO_STREAM_prv( DMA2_Stream, CONFIG_SDDRV_DMA_STREAM_NO )
#define SD_SDIO_DMA_FLAG_HTIF         _SD_SDIO_STREAM_prv( DMA_FLAG_HTIF, CONFIG_SDDRV_DMA_STREAM_NO )
#define SD_SDIO_DMA_FLAG_DMEIF        _SD_SDIO_STREAM_prv( DMA_FLAG_DMEIF, CONFIG_SDDRV_DMA_STREAM_NO )
#define SD_SDIO_DMA_FLAG_TEIF         _SD_SDIO_STREAM_prv( DMA_FLAG_TEIF, CONFIG_SDDRV_DMA_STREAM_NO )
#define SD_SDIO_DMA_CHANNEL           DMA_Channel_4
#define SD_SDIO_DMA_IRQn              _SD_SDIO_STREAM_prv3(DMA2_Stream,CONFIG_SDDRV_DMA_STREAM_NO,_IRQn)
#define SDIO_FIFO_ADDRESS             ((void*)0x40012C80)
#if (CONFIG_SDDRV_DMA_STREAM_NO<=3)
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
	  dma_cmd(SD_SDIO_DMA_STREAM, false);
	  /* DMA2 Stream3  or Stream6 Config */
	  dma_deinit( SD_SDIO_DMA_STREAM );
	  dma_init( SD_SDIO_DMA_STREAM , DMA_DIR_MemoryToPeripheral | DMA_PeripheralInc_Disable |
				SD_SDIO_DMA_CHANNEL |DMA_MemoryInc_Enable | DMA_PeripheralDataSize_Word | DMA_MemoryDataSize_Word |
				DMA_Mode_Normal | DMA_Priority_VeryHigh | DMA_MemoryBurst_INC4| DMA_PeripheralBurst_INC4,
				DMA_FIFOMode_Enable | DMA_FIFOThreshold_Full, 0, SDIO_FIFO_ADDRESS, (uint32_t*)buffer_src );
	  dma_flow_controller_config(SD_SDIO_DMA_STREAM, DMA_FlowCtrl_Peripheral);
	  /* DMA2 Stream3  or Stream6 enable */
	  dma_cmd(SD_SDIO_DMA_STREAM, true);
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
	  dma_cmd(SD_SDIO_DMA_STREAM, false);
	  /* DMA2 Stream3 or Stream6 Config */
	  dma_deinit(SD_SDIO_DMA_STREAM);
	  dma_init(SD_SDIO_DMA_STREAM, SD_SDIO_DMA_CHANNEL | DMA_DIR_PeripheralToMemory | DMA_PeripheralInc_Disable |
			  DMA_MemoryInc_Enable | DMA_PeripheralDataSize_Word | DMA_MemoryDataSize_Word |
			  DMA_Mode_Normal | DMA_Priority_Medium | DMA_MemoryBurst_INC4 | DMA_PeripheralBurst_INC4 ,
			  DMA_FIFOMode_Enable | DMA_FIFOThreshold_Full, 0, SDIO_FIFO_ADDRESS,  buf_dst );
	  dma_flow_controller_config(SD_SDIO_DMA_STREAM, DMA_FlowCtrl_Peripheral);
	  /* DMA2 Stream3 or Stream6 enable */
	  dma_cmd(SD_SDIO_DMA_STREAM, true);
	}
}	/* Unnamed namespace */
#endif
/*----------------------------------------------------------*/
#if (ISIX_SDDRV_TRANSFER_MODE )
namespace		//Private namespace for IRQ handling
{
	mmc_host_sdio *p_sdio;
}
#endif
/*----------------------------------------------------------*/
#if !(ISIX_SDDRV_TRANSFER_MODE & ISIX_SDDRV_TRANSFER_USE_IRQ)
namespace
{
	//Used in polling mode only
	static bool wait_for_trx_complete( unsigned timeout )
	{
		uint32_t sreg;
		bool ret = false;
		static const uint32_t wait_flags = SDIO_FLAG_DCRCFAIL|SDIO_FLAG_DTIMEOUT|SDIO_FLAG_DATAEND|
				SDIO_FLAG_RXOVERR|SDIO_FLAG_STBITERR;
		ostick_t t_start = isix_get_jiffies();
		timeout = isix_ms2tick( timeout );
		do
		{
			sreg = SDIO->STA;
			if(timer_elapsed( t_start, timeout) )
			{
				ret = true;
				break;
			}
		}
		while( !(sreg & wait_flags) );
		return ret;
	}
}
#endif
/*----------------------------------------------------------*/
//Process irq
void mmc_host_sdio::process_irq_sdio()
{
	 using namespace ::drv::mmc;
	 using namespace stm32;
	 if (sdio_get_it_status(SDIO_IT_DATAEND) != 0)
	 {
	    m_transfer_error = MMC_OK;
	    sdio_clear_it_pending_bit(SDIO_IT_DATAEND);
	 }
	 else if (sdio_get_it_status(SDIO_IT_DCRCFAIL) != 0)
	 {
		sdio_clear_it_pending_bit(SDIO_IT_DCRCFAIL);
	    m_transfer_error = MMC_DATA_CRC_FAIL;
	 }
	 else if (sdio_get_it_status(SDIO_IT_DTIMEOUT) != 0)
	 {
		sdio_clear_it_pending_bit(SDIO_IT_DTIMEOUT);
		m_transfer_error = MMC_DATA_TIMEOUT;
	 }
	 else if (sdio_get_it_status(SDIO_IT_RXOVERR) != 0)
	 {
		sdio_clear_it_pending_bit(SDIO_IT_RXOVERR);
		m_transfer_error = MMC_RX_OVERRUN;
	 }
	 else if (sdio_get_it_status(SDIO_IT_TXUNDERR) != 0)
	 {
		sdio_clear_it_pending_bit(SDIO_IT_TXUNDERR);
		m_transfer_error = MMC_TX_UNDERRUN;
	 }
	 else if (sdio_get_it_status(SDIO_IT_STBITERR) != 0)
	 {
		sdio_clear_it_pending_bit(SDIO_IT_STBITERR);
		m_transfer_error = MMC_START_BIT_ERR;
	 }
#if (ISIX_SDDRV_TRANSFER_MODE & ISIX_SDDRV_TRANSFER_USE_IRQ)
	 sdio_it_config(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND |
	               SDIO_IT_TXFIFOHE | SDIO_IT_RXFIFOHF | SDIO_IT_TXUNDERR |
	               SDIO_IT_RXOVERR | SDIO_IT_STBITERR, false);
	 m_complete.signal_isr();
#endif
}
/*----------------------------------------------------------*/
#if (ISIX_SDDRV_TRANSFER_MODE & ISIX_SDDRV_WAIT_USE_IRQ)
void mmc_host_sdio::process_irq_exti()
{
    m_complete.signal_isr();
}
#endif
/*----------------------------------------------------------*/
#if (ISIX_SDDRV_TRANSFER_MODE & ISIX_SDDRV_TRANSFER_USE_IRQ)
extern "C"
{
	void __attribute__((__interrupt__)) sdio_isr_vector( void )
	{
		if(p_sdio) p_sdio->process_irq_sdio();
	}
}
#endif

/*----------------------------------------------------------*/
#if (ISIX_SDDRV_TRANSFER_MODE & ISIX_SDDRV_WAIT_USE_IRQ)
extern "C" {
    void __attribute__((__interrupt__)) exti8_isr_vector(void)
    {
        stm32::exti_clear_it_pending_bit( EXTI_Line8 );
        exti_init( EXTI_Line8, EXTI_Mode_Interrupt, EXTI_Trigger_Rising, false );
        if(p_sdio) p_sdio->process_irq_exti();
    }
}
#endif
/*----------------------------------------------------------*/
//Constructor
mmc_host_sdio::mmc_host_sdio( unsigned pclk2, int spi_speed_limit_khz )
	: m_pclk2(pclk2), m_spi_speed_limit_khz(spi_speed_limit_khz)
#if(ISIX_SDDRV_TRANSFER_MODE)
	, m_complete( 0, 1 )
#endif
	, m_transfer_error(0)
{
	  using namespace stm32;
#if CONFIG_SDDRV_INIT_DMA_GPIO_CLKS == 1
	  /* SDIO Peripheral Low Level Init */
	  rcc_ahb1_periph_clock_cmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD, true);
#endif
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
#if CONFIG_SDDRV_INIT_DMA_GPIO_CLKS == 1 
	  //Configure DMA
	  rcc_ahb1_periph_clock_cmd( RCC_AHB1Periph_DMA2, true );
#endif /* CONFIG_SDDRV_INIT_DMA_GPIO_CLKS */
#else
#error F1 not implemented yet
#endif
	  sdio_deinit();
	  rcc_apb2_periph_clock_cmd( RCC_APB2Periph_SDIO, true );
#if(ISIX_SDDRV_TRANSFER_MODE)
	  p_sdio = this;
#endif
#if(ISIX_SDDRV_TRANSFER_MODE & ISIX_SDDRV_TRANSFER_USE_IRQ)
	  nvic_set_priority( SDIO_IRQn, IRQ_PRIO, IRQ_SUB );
	  nvic_irq_enable( SDIO_IRQn, true );
#endif
	 sdio_dma_cmd(true);
#if ISIX_SDDRV_TRANSFER_MODE & ISIX_SDDRV_WAIT_USE_IRQ
     rcc_apb2_periph_clock_cmd(RCC_APB2Periph_SYSCFG, true );
     gpio_exti_line_config( GPIO_PortSourceGPIOC, GPIO_PinSource8 );
     nvic_set_priority( EXTI9_5_IRQn, IRQ_PRIO, IRQ_SUB );
     nvic_irq_enable( EXTI9_5_IRQn, true );
#endif
}
/*----------------------------------------------------------*/
//Destructor
mmc_host_sdio::~mmc_host_sdio()
{
	/*!< Disable SDIO Clock */
	sdio_clock_cmd(false);
	/*!< Set Power State to OFF */
	sdio_set_power_state(SDIO_PowerState_OFF);
	/*!< DeInitializes the SDIO peripheral */
	sdio_deinit();
	/* Disable the SDIO APB2 Clock */
	rcc_apb2_periph_clock_cmd(RCC_APB2Periph_SDIO, false);
#if defined(STM32MCU_MAJOR_TYPE_F4) || defined(STM32MCU_MAJOR_TYPE_F2)
	rcc_ahb1_periph_clock_cmd( RCC_AHB1Periph_DMA2, false );
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
			resp = SDIO_Response_Long;
		else
			resp = SDIO_Response_Short;
	}
	//Send command
	uint32_t fwait = SDIO_Wait_No;
	if(req.get_flags() & mmc_command::resp_busy ) fwait = SDIO_Wait_IT;

	stm32::sdio_send_command( req.get_arg(), req.get_op()&0x7f,
			resp, fwait, SDIO_CPSM_Enable );
	uint32_t stat;
	if( req.get_flags() & mmc_command::resp_present )
		stat = SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT|SDIO_FLAG_CCRCFAIL;
	else
		stat = SDIO_FLAG_CMDSENT;
	ostick_t t_start = isix_get_jiffies();
	timeout = isix_ms2tick( timeout );
	uint32_t sreg;
	int ret = MMC_OK;
	do
	{
		sreg = SDIO->STA;
		if(timer_elapsed( t_start, timeout) )
			break;
	}
	while( !(sreg & stat) );
	//Check the response
	do {
		//Check the errors
		if( !(sreg & stat) )
		{
			ret =  MMC_CMD_RSP_TIMEOUT;
			break;
		}
		if( (sreg & stat) & SDIO_FLAG_CTIMEOUT )
		{
			ret =  MMC_CMD_RSP_TIMEOUT;
			dbprintf("HW timeout cmd %i arg %08lx", req.get_op(), req.get_arg());
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
	timeout = (m_pclk2/2/8/1000) * timeout;
#if(ISIX_SDDRV_TRANSFER_MODE & ISIX_SDDRV_TRANSFER_USE_IRQ)
	 sdio_it_config( SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND
		 | SDIO_IT_RXOVERR | SDIO_IT_STBITERR , true );
#endif
	sdio_data_config( timeout, len, block_size, SDIO_TransferDir_ToCard,
					SDIO_TransferMode_Block, SDIO_DPSM_Enable );
	sd_lowlevel_dma_tx_config(buf, len);
	int ret;
	//Wait for complete sem
#if (ISIX_SDDRV_TRANSFER_MODE & ISIX_SDDRV_TRANSFER_USE_IRQ)
	ret = m_complete.wait( timeout );
#else
	if( wait_for_trx_complete(timeout) )
	{
		ret = MMC_DATA_TIMEOUT;
	}
	else
	{
		process_irq_sdio();
		ret = MMC_OK;
	}
#endif
	sdio_clear_flag( SDIO_STATIC_FLAGS );
	if( !ret )
		ret = m_transfer_error;
	/* Check the status of the DMA transfer */
	if ( !ret && (!dma_get_flag_status(SD_SDIO_DMA_STREAM, SD_SDIO_DMA_FLAG_TCIF) ||
		  dma_get_flag_status(SD_SDIO_DMA_STREAM, SD_SDIO_DMA_FLAG_TEIF)) )
	{
		ret = mmc_host::err_hwdma_fail;
	}
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
	timeout = (m_pclk2/2/8/1000) * timeout;
	sdio_data_config( timeout, len, block_size, SDIO_TransferDir_ToSDIO, SDIO_TransferMode_Block, SDIO_DPSM_Enable );
#if(ISIX_SDDRV_TRANSFER_MODE & ISIX_SDDRV_TRANSFER_USE_IRQ)
	sdio_it_config(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR, true);
#endif
	sdio_dma_cmd(true);
	sd_lowlevel_dma_rx_config( buf, len );
	//Wait for data transfer
	return MMC_OK;
}
/*----------------------------------------------------------*/
//Execute MMC data transfer
int mmc_host_sdio::receive_data( void* /*buf */, size_t /*len*/, unsigned timeout )
{
	using namespace stm32;
	using namespace ::drv::mmc;
	int ret;
	//Wait for complete sem
#if (ISIX_SDDRV_TRANSFER_MODE & ISIX_SDDRV_TRANSFER_USE_IRQ)
	ret = m_complete.wait( timeout );
#else
	if( wait_for_trx_complete(timeout) )
	{
		ret = MMC_DATA_TIMEOUT;
	}
	else
	{
		process_irq_sdio();
		ret = MMC_OK;
	}
#endif
	sdio_clear_flag( SDIO_STATIC_FLAGS );
	if( !ret )
		ret = m_transfer_error;
	/* Check the status of the DMA transfer */
	if ( !ret && (!dma_get_flag_status(SD_SDIO_DMA_STREAM, SD_SDIO_DMA_FLAG_TCIF) ||
		  dma_get_flag_status(SD_SDIO_DMA_STREAM, SD_SDIO_DMA_FLAG_TEIF)) )
	{
		ret = mmc_host::err_hwdma_fail;
	}
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
		dbprintf("Command power OFF");
		break;
	}
	//PWR ON
	case mmc_host::ios_pwr_on:
	{
		isix_wait_ms( 5 );
		 //Initialize the SDIO ifc
		sdio_init( SDIO_ClockEdge_Rising, SDIO_ClockBypass_Disable, SDIO_ClockPowerSave_Disable,
			SDIO_BusWide_1b, SDIO_HardwareFlowControl_Disable, khz_to_sdio_div( C_low_clk_khz_host, m_pclk2)
		);
		/*!< Set Power State to ON */
		sdio_set_power_state(SDIO_PowerState_ON);
		for(int w=0; w<16; w++ ) nop();
		/*!< Enable SDIO Clock */
		sdio_clock_cmd(true);
		dbprintf("Command power ON");
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
		dbprintf("Command setSpeed to %u", param );
		break;
	}
	//SET bus wide
	case mmc_host::ios_set_bus_with:
	{
		uint32_t cr = SDIO->CLKCR;
		cr = (cr & ~0x1800) | (param==mmc_host::bus_width_4b?SDIO_BusWide_4b:SDIO_BusWide_1b);
		SDIO->CLKCR = cr;
		dbprintf("Command setBusWidth to %u", param );
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
//Wait for data will be ready
int mmc_host_sdio::wait_data_ready( unsigned timeout )
{
	using namespace ::drv::mmc;
	int ret = MMC_OK;
#if ISIX_SDDRV_TRANSFER_MODE & ISIX_SDDRV_WAIT_USE_IRQ
	if( !gpio_get(DATA0_PORT,DATA0_PIN) )
	{
		exti_clear_it_pending_bit( EXTI_Line8 );
		exti_init( EXTI_Line8, EXTI_Mode_Interrupt, EXTI_Trigger_Rising, true );
		ret = m_complete.wait( timeout );
	}
	return ret;
#else
	ostick_t t_start = isix_get_jiffies();
	timeout = isix_ms2tick( timeout );
	while( !gpio_get(DATA0_PORT,DATA0_PIN) )
	{
		if( timer_elapsed(t_start,timeout) )
		{
			ret = MMC_DATA_TIMEOUT;
			break;
		}
	}
	return ret;
#endif
}
/*----------------------------------------------------------*/
}}
/*----------------------------------------------------------*/
