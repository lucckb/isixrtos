/*
 * spi_sdcard_driver.c
 *
 *  Created on: 27-11-2012
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#include "spi_sdcard_driver.h"
#include "sdcard_defs.h"
#ifdef _HAVE_CONFIG_H
#include "config.h"
#endif
#include <stm32spi.h>
#include <stm32gpio.h>
#include <stm32rcc.h>
#include <isix.h>
#include <dbglog.h>
/* ------------------------------------------------------------------ */
#ifndef SDDRV_SPI_DEVICE
#define SDDRV_SPI_DEVICE 1
#endif

#ifndef SDDRV_DETECT_GPIO_PORT
#define SDDRV_DETECT_GPIO_PORT D
#endif

#ifndef SDDRV_DETECT_PIN
#define SDDRV_DETECT_PIN 0
#endif

//Config section
#ifndef SDDRV_INIT_CLK_DIV
#define SDDRV_INIT_CLK_DIV                SPI_BaudRatePrescaler_256
#endif

//Normal transfer CLK divider
#ifndef SDDRV_TRANSFER_CLK_DIV
#define SDDRV_TRANSFER_CLK_DIV            SPI_BaudRatePrescaler_8
#endif

/* ------------------------------------------------------------------ */
#define _SPI_SD_cat_(x, y) x##y
#define _SPI_SD_cat(x, y)  _SPI_SD_cat_(x, y)

/* ------------------------------------------------------------------ */
#if (SDDRV_SPI_DEVICE==1)
#define SD_SPI_SCK_GPIO_CLK              RCC_APB2Periph_GPIOA
#define SD_SPI_MISO_GPIO_CLK             RCC_APB2Periph_GPIOA
#define SD_SPI_MOSI_GPIO_CLK             RCC_APB2Periph_GPIOA
#define SD_CS_GPIO_CLK                   RCC_APB2Periph_GPIOA
#define SD_SPI_MISO_PIN					 6
#define SD_SPI_MOSI_PIN					 7
#define SD_SPI_SCK_PIN 					 5
#define SD_SPI_CS_PIN 					 4
#define SD_SPI_SCK_PORT					 GPIOA
#define SD_SPI_MISO_PORT				 GPIOA
#define SD_SPI_MOSI_PORT				 GPIOA
#define SD_SPI_SCK_PORT					 GPIOA
#define SD_SPI_CS_PORT 					 GPIOA
#else
#error other SPI not implemented yet
#endif
#define SD_DETECT_GPIO_PORT				 _SPI_SD_cat(GPIO, SDDRV_DETECT_GPIO_PORT )
#define SD_DETECT_GPIO_PIN				 SDDRV_DETECT_PIN
#define SD_SPI 							 _SPI_SD_cat(SPI, SDDRV_SPI_DEVICE)
#define SD_DETECT_GPIO_CLK _SPI_SD_cat(RCC_APB2Periph_GPIO,SDDRV_DETECT_GPIO_PORT)

/* ------------------------------------------------------------------ */
#define SD_DUMMY_BYTE 0xff

typedef enum sd_error
{
  SD_R1_RESPONSE_NO_ERROR      = (0x00),
  SD_R1_IN_IDLE_STATE          = (0x01),
  SD_R1_ERASE_RESET            = (0x02),
  SD_R1_ILLEGAL_COMMAND        = (0x04),
  SD_R1_COM_CRC_ERROR          = (0x08),
  SD_R1_ERASE_SEQUENCE_ERROR   = (0x10),
  SD_R1_ADDRESS_ERROR          = (0x20),
  SD_R1_PARAMETER_ERROR        = (0x40),
} sd_error;


/* ------------------------------------------------------------------ */
static uint32_t card_type =  SDIO_STD_CAPACITY_SD_CARD_V1_1;
/* ------------------------------------------------------------------ */
//Reinit SPI change divider
static inline void sdspi_config(unsigned divider)
{
	spi_init( SD_SPI,  SPI_Direction_2Lines_FullDuplex, SPI_Mode_Master, SPI_DataSize_8b,
			   SPI_CPOL_High, SPI_CPHA_2Edge, SPI_NSS_Soft, divider , SPI_FirstBit_MSB , 7 );
}

/* ------------------------------------------------------------------ */
//! Initialize the SPI interface for GPIO card access
static void sd_spi_hwinit(void)
{
	rcc_apb2_periph_clock_cmd( SD_SPI_SCK_GPIO_CLK | SD_SPI_MISO_GPIO_CLK |
			SD_SPI_MOSI_GPIO_CLK | SD_CS_GPIO_CLK |  SD_DETECT_GPIO_CLK, true );
#if   (SDDRV_SPI_DEVICE==1)
	rcc_apb2_periph_clock_cmd( RCC_APB2Periph_SPI1, true );
#elif (SDDRV_SPI_DEVICE==2)
	rcc_apb1_periph_clock_cmd( RCC_APB1Periph_SPI2, true );
#elif (SDDRV_SPI_DEVICE==3)
	rcc_apb1_periph_clock_cmd( RCC_APB1Periph_SPI3, true );
#endif
	//Configure GPIO PINS
	gpio_abstract_config( SD_SPI_SCK_PORT,  SD_SPI_SCK_PIN,  AGPIO_MODE_ALTERNATE_PP, AGPIO_SPEED_HALF );
	gpio_abstract_config( SD_SPI_MOSI_PORT, SD_SPI_MOSI_PIN, AGPIO_MODE_ALTERNATE_PP, AGPIO_SPEED_HALF );
	gpio_abstract_config( SD_SPI_MISO_PORT, SD_SPI_MISO_PIN, AGPIO_MODE_INPUT_FLOATING, AGPIO_SPEED_HALF );
	gpio_abstract_config( SD_SPI_CS_PORT,   SD_SPI_CS_PIN,   AGPIO_MODE_OUTPUT_PP, AGPIO_SPEED_HALF );
	gpio_abstract_config( SD_DETECT_GPIO_PORT, SD_DETECT_GPIO_PIN, AGPIO_MODE_INPUT_PULLUP, AGPIO_SPEED_HALF );
	//Configure the SPI
	sdspi_config( SDDRV_INIT_CLK_DIV );
	//Enable the SPI
	spi_cmd( SD_SPI, true );
}

/* ------------------------------------------------------------------ */
/* Set or reset chip select */
static inline void CS(bool en)
{
	if( en )
		gpio_set( SD_SPI_CS_PORT, SD_SPI_CS_PIN );
	else
		gpio_clr( SD_SPI_CS_PORT, SD_SPI_CS_PIN );
}

/* ------------------------------------------------------------------ */
/* Write byte (no dma) */
static uint8_t sd_transfer_byte(uint8_t data)
{
  /*!< Wait until the transmit buffer is empty */
  while(!spi_i2s_get_flag_status(SD_SPI, SPI_I2S_FLAG_TXE) )
  {
  }
  spi_i2s_send_data(SD_SPI, data);
  while(!spi_i2s_get_flag_status(SD_SPI, SPI_I2S_FLAG_RXNE))
  {
  }
  /*!< Return the byte read from the SPI bus */
  return spi_i2s_receive_data(SD_SPI);
}

/* ------------------------------------------------------------------ */
static uint8_t sd_command(uint8_t command, uint32_t address)
{
	unsigned timeout;
	sd_transfer_byte(0xff); 		// Dummy write
	sd_transfer_byte(command|0x40);
	sd_transfer_byte(((address & 0xFF000000)>>24)); // MSB of adress
	sd_transfer_byte(((address & 0x00FF0000)>>16));
	sd_transfer_byte(((address & 0x0000FF00)>>8));
	sd_transfer_byte((address & 0x000000FF));       // LSB of adress
	sd_transfer_byte(0x95); // Checksum for CMD0 GO_IDLE_STATE and dummy checksum for other commands
	timeout=0;
	uint8_t r1= SD_DUMMY_BYTE;
	//wait for response
	while(r1== SD_DUMMY_BYTE)
	{
		r1 = sd_transfer_byte( SD_DUMMY_BYTE );
		timeout++;
		if(timeout>2500) break; // no response
	}
	return r1;
}

/* ------------------------------------------------------------------ */
//Initialize SD CARD
static int sd_init_card(void)
{
	unsigned retry = 0;
	CS(1);
	for(int b=0; b<10; b++)
		sd_transfer_byte( SD_DUMMY_BYTE );
	CS(0);
	uint8_t r1;
	do
	{
		// Zerowanie karty
		r1 = sd_command(SD_CMD_GO_IDLE_STATE, 0);
		// Licznik wznowien
		if(++retry>250)
		{
			CS(1);
			return SD_CMD_RSP_TIMEOUT;
		}
	}
	while(r1 != 0x01);
	r1 = sd_command( SDIO_SEND_IF_COND, 0x00000122 );
	if(!(r1 & SD_R1_ILLEGAL_COMMAND) ) // SD card version 2.00
	{
		dbprintf("SD card V2/SHC\n");
		uint8_t iob[4];
		//geht four more bytes of response7
		for(int i=0; i<4; i++)
		{
		   iob[i] = sd_transfer_byte(0xff);
		}
		if(iob[2] != 0x01) // test voltage range failed
		{
		    CS(1);
		    return SD_INVALID_VOLTRANGE;
		}
		if(iob[3] != 0x22) // test check pattern failed
		{
			 CS(1);
		     return SD_ADDR_OUT_OF_RANGE;
		}
		sd_command(SD_CMD_SDIO_READ_OCR , 0); // READ_OCR
		//geht four more bytes of response3
		for(int i=0; i<4; i++)
		{
		   iob[i]= sd_transfer_byte(0xff);
		}
		retry = 0;
		do // repeat ACMD41 until card is ready
		{
			sd_command( SD_CMD_APP_CMD, 0 ); // SEND_APP_CMD
		    r1 =sd_command( SD_CMD_SD_APP_OP_COND, 0x40000000 ); // send with HCS bit set
		    retry++;
		    if(retry == 1024)  // no response from card
		    {
		       CS(1);
		       return SD_CMD_RSP_TIMEOUT;
		    }
		 }
		 while( r1 & SD_R1_IN_IDLE_STATE );

		 // check for high capacity card now
		 r1 = sd_command(SD_CMD_SDIO_READ_OCR, 0 ); // READ OCR

		 //geht four more bytes of response3
		 for(int i=0; i<4; i++)
		 {
		   iob[i] = sd_transfer_byte(0xff);
		 }
		 if(iob[0] & 0x40)
		 {
		    card_type  = SDIO_HIGH_CAPACITY_SD_CARD;
		    dbprintf("SDIO_HIGH_CAPACITY_SD_CARD");
		 }
		 else
		 {
		  	card_type = SDIO_STD_CAPACITY_SD_CARD_V2_0;
		    dbprintf("SDIO_STD_CAPACITY_SD_CARD_V2_0");
		 }
	}
	else	//Standard capacity
	{

			// SEND_APP_CMD
		if((sd_command(SD_CMD_APP_CMD ,0) & SD_R1_ILLEGAL_COMMAND) == 0 ) // SD card V1.xx
		{
			card_type = SDIO_STD_CAPACITY_SD_CARD_V1_1;
			dbprintf("SD card V1\n");
			retry=0;
			do // repeat ACMD41 until card is ready
			{

				r1 = sd_command(SD_CMD_SD_APP_OP_COND,0);
				if( !(r1 & SD_R1_IN_IDLE_STATE) ) break;
				retry++;
				if(retry == 1024)  // no response from card
				{
					CS(1);
					return SD_CMD_RSP_TIMEOUT;
				}
				sd_command( SD_CMD_APP_CMD, 0 ); // Repeat SEND_APP_CMD
			}
			while(1);
		}
		else // MMC card
		{
			card_type = SDIO_MULTIMEDIA_CARD;
			dbprintf("MMC card\n");
			retry=0;
			do
			{
				r1 = sd_command(SD_CMD_SEND_OP_COND, 0 );
				retry++;
				if(retry == 1024)  // no response from card
				{
					CS(1);
					return SD_CMD_RSP_TIMEOUT;
				}
			}
			while(r1!=0);
		}
	}
	// Wylacz sprawdzenie CRC
	r1 = sd_command(SD_CMD_CRC_ON_OFF, 0);
	if( r1 )
	{
		CS(1);
		return SD_ILLEGAL_CMD;
	}
	// Ustaw rozmiar bloku na 512 bajtow
	r1 = sd_command(SD_CMD_SET_BLOCKLEN, 512);
	if(r1)
	{
		CS(1);
		return SD_ILLEGAL_CMD;
	}
	CS(1);
	//Podzielnik
	sdspi_config( SDDRV_TRANSFER_CLK_DIV );
	return SD_OK;
}
/* ------------------------------------------------------------------ */
#define MMC_DE_CHECK_MASK			0xF1
#define MMC_STARTBLOCK_READ			0xFE
#define MMC_DE_ERROR				0x01
#define MMC_STARTBLOCK_WRITE		0xFE
#define MMC_DR_MASK					0x1F
#define MMC_DR_ACCEPT				0x05
/* Odczytuje sektor o numerze sector do bufora buffer
 * z karty MMC zwraca 0 gdy OK
 */
static int mmcRead(uint8_t* buffer,unsigned long sector)
{
	uint8_t r1;
	int i;
	// Wlacz CS
	CS(0);
	// Wyslij komende odczytu bloku
	r1 = sd_command(SD_CMD_READ_SINGLE_BLOCK, card_type==SDIO_HIGH_CAPACITY_SD_CARD?sector:sector<<9);
	// Sprawdz odpowiedz
	if(r1 != 0x00)
	{
		CS(1);
		return r1;
	}
	// Czekaj na gotowosc karty
	while(1)
	{
		r1 = sd_transfer_byte(0xFF);
		if( r1 == MMC_STARTBLOCK_READ)
			break;
		if((r1&MMC_DE_CHECK_MASK)==MMC_DE_ERROR)
		{
			CS(1);
			return -(int)r1;
		}
	}

	// Odczytuj dane
	for(i=0; i<0x200; i++)
	{
		*buffer++ = sd_transfer_byte(0xFF);
	}
	// Nie sprawdzaj CRC
	 sd_transfer_byte(0xFF);  sd_transfer_byte(0xFF);
	// Zwolnij CS
	CS(1);
	// Wszystko OK
	return 0;
}

/* Zapisuje dane z bufora buffer do sektora o numerze
 * sector zwraca 0 gdy wszystko w porzadku */
static int mmcWrite(const uint8_t* buffer, unsigned sector)
{
	uint8_t r1;
	int i;

	utick_t xxm1 =  isix_get_ujiffies();
	// Uruchom CS
	CS(0);
	// Wyslij komende odczytu
	r1 = sd_command(SD_CMD_WRITE_SINGLE_BLOCK, card_type==SDIO_HIGH_CAPACITY_SD_CARD?sector:sector<<9);
	// Sprawdz odpowiedz karty
	if(r1 != 0x00)
	{
		CS(1);
		return -(int)r1;
	}
	// Wyslij pusty bajt
	sd_transfer_byte(0xFF);
	// Wyslij kod rozpoczecia transferu
	sd_transfer_byte(MMC_STARTBLOCK_WRITE);
	// Zapisz dane z bufora
	for(i=0; i<0x200; i++)
	{
		sd_transfer_byte(*buffer++);
	}
	// zapisz 16-bitowy CRC - nieistotny
	sd_transfer_byte(0xFF);
	sd_transfer_byte(0xFF);
	// Sprawdz token odpowiedzi
	r1 = sd_transfer_byte(0xFF);
	if( (r1&MMC_DR_MASK) != MMC_DR_ACCEPT)
	{
		CS(1);
		return r1;
	}
	dbprintf("TICK #2 %u", (unsigned)(isix_get_ujiffies()-xxm1));
	xxm1 =  isix_get_ujiffies();
	// Czekaj az karta bedzie wolna
	while(!sd_transfer_byte(0xFF));
	dbprintf("TICK #3 %u", (unsigned)(isix_get_ujiffies()-xxm1));
	// Zwolnij CS
	CS(1);
	// Zwroc OK
	return 0;
}
/* ------------------------------------------------------------------ */
//Initialize the SDIO card driver
int isix_spisd_card_driver_init(void)
{
	//SPI low level init
	sd_spi_hwinit();
	/* Initialize SD card */
	return sd_init_card();
}
/* ------------------------------------------------------------------ */

//Card driver status
sdcard_drvstat isix_spisd_card_driver_status(void)
{
	return SD_TRANSFER_OK;
}

/* ------------------------------------------------------------------ */
//SDIO card driver read
int isix_spisd_card_driver_read( void *buf, unsigned long LBA, size_t count )
{
	uint8_t *_buf = buf;
	for(size_t i=0;i<count;i++)
	{
		if (mmcRead(_buf, LBA+i ) ) return SD_ERROR;
	}
	return SD_OK;
}

/* ------------------------------------------------------------------ */
//SDIO card driver write
int isix_spisd_card_driver_write( const void *buf, unsigned long LBA, size_t count )
{
	const uint8_t *_buf = buf;
	for(size_t i=0;i<count;i++)
	{
		if (mmcWrite(_buf, LBA+i ) ) return SD_ERROR;
	}
	return SD_OK;
}

/* ------------------------------------------------------------------ */
/* Destroy card driver and disable interrupt */
void isix_spisd_card_driver_destroy(void)
{

}

/* ------------------------------------------------------------------ */
//Get SD card info
int isix_spisd_card_driver_get_info( void *cardinfo, scard_info_field req )
{
	return 0;
}

/* ------------------------------------------------------------------ */
//SD CARD initialize again
int isix_spisd_card_driver_reinitialize( void )
{
	return sd_init_card();
}

/* ------------------------------------------------------------------ */
//IS card connected to slot
bool isix_spisd_card_driver_is_card_in_slot( void )
{
	return !gpio_get( SD_DETECT_GPIO_PORT, SD_DETECT_GPIO_PIN );
}

/* ------------------------------------------------------------------ */
