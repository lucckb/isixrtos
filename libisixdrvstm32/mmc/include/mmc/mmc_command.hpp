/*
 * mmc_command.hpp
 *
 *  Created on: 01-12-2012
 *      Author: lucck
 */
/*----------------------------------------------------------*/
#ifndef MMC_COMMAND_HPP_
#define MMC_COMMAND_HPP_
/*----------------------------------------------------------*/
#include "noncopyable.hpp"
#include <stdint.h>
#include "mmc/mmc_error_codes.hpp"
/*----------------------------------------------------------*/
namespace drv {

/*----------------------------------------------------------*/
class mmc_command: public fnd::noncopyable
{
private:	/* Private response codes */
	enum resp_code
	{
		rR1t = 1, rR2t, rR3t, rR4t, rR5t, rR6t, rR7t
	};
	enum r1_bits
	{
		bR1_IN_IDLE_STATE 	= 1<<0,
		bR1_ERASE_RESET   	= 1<<1,
		bR1_ILLEGAL_COMMAND = 1<<2,
		bR1_COM_CRC_ERROR   = 1<<3,
		bR1_ERASE_SEQ_ERROR = 1<<4,
		bR1_ADDRESS_ERROR   = 1<<5,
		bR1_PARAMETER_ERROR = 1<<6
	};
public:
	enum op
	{
			OP_GO_IDLE_STATE                       =  0,
			OP_SEND_OP_COND                        =  1,
			OP_ALL_SEND_CID                        =  2,
			OP_SET_REL_ADDR                        =  3, /*!< SDIO_SEND_REL_ADDR for SD Card */
			OP_SET_DSR                             =  4,
			OP_SDIO_SEN_OP_COND                    =  5,
			OP_HS_SWITCH                           =  6,
			OP_SEL_DESEL_CARD                      =  7,
			OP_SEND_IF_COND						   =  8,
			OP_SEND_CSD                            =  9,
			OP_SEND_CID                            =  10,
			OP_READ_DAT_UNTIL_STOP                 =  11, /*!< SD Card doesn't support it */
			OP_STOP_TRANSMISSION                   =  12,
			OP_SEND_STATUS                         =  13,
			OP_HS_BUSTEST_READ                     =  14,
			OP_GO_INACTIVE_STATE                   =  15,
			OP_SET_BLOCKLEN                        =  16,
			OP_READ_SINGLE_BLOCK                   =  17,
			OP_READ_MULT_BLOCK                     =  18,
			OP_HS_BUSTEST_WRITE                    =  19,
			OP_WRITE_DAT_UNTIL_STOP                =  20, /*!< SD Card doesn't support it */
			OP_SET_BLOCK_COUNT                     =  23, /*!< SD Card doesn't support it */
			OP_WRITE_SINGLE_BLOCK                  =  24,
			OP_WRITE_MULT_BLOCK                    =  25,
			OP_PROG_CID                            =  26, /*!< reserved for manufacturers */
			OP_PROG_CSD                            =  27,
			OP_SET_WRITE_PROT                      =  28,
			OP_CLR_WRITE_PROT                      =  29,
			OP_SEND_WRITE_PROT                     =  30,
			OP_SD_ERASE_GRP_START                  =  32, /*!< To set the address of the first write block to be erased. (For SD card only) */
			OP_SD_ERASE_GRP_END                    =  33, /*!< To set the address of the last write block of the continuous range to be erased. (For SD card only) */
			OP_ERASE_GRP_START                     =  35, /*!< To set the address of the first write block to be erased. (For MMC card only spec 3.31) */
			OP_ERASE_GRP_END                       =  36, /*!< To set the address of the last write block of the continuous range to be erased. (For MMC card only spec 3.31) */
			OP_ERASE                               =  38,
			OP_FAST_IO                             =  39, /*!< SD Card doesn't support it */
			OP_GO_IRQ_STATE                        =  40, /*!< SD Card doesn't support it */
			OP_LOCK_UNLOCK                         =  42,
			OP_APP_CMD                             =  55,
			OP_GEN_CMD                             =  56,
			OP_NO_CMD                              =  64,
			OP_CRC_ON_OFF						   =  59,
			OP_APP_SD_SET_BUSWIDTH                 =  6 , /*!< For SD Card only */
			OP_SD_APP_STAUS                        =  13, /*!< For SD Card only */
			OP_SD_APP_SEND_NUM_WRITE_BLOCKS        =  22, /*!< For SD Card only */
			OP_SD_APP_OP_COND                      =  41, /*!< For SD Card only */
			OP_SD_APP_SET_CLR_CARD_DETECT          =  42, /*!< For SD Card only */
			OP_SD_APP_SEND_SCR                     =  51, /*!< For SD Card only */
			OP_SDIO_RW_DIRECT                      =  52, /*!< For SD I/O Card only */
			OP_SDIO_RW_EXTENDED                    =  53, /*!< For SD I/O Card only */
			OP_SDIO_READ_OCR					   =  58,
			OP_SD_APP_GET_MKB                      =  43, /*!< For SD Card only */
			OP_SD_APP_GET_MID                      =  44, /*!< For SD Card only */
			OP_SD_APP_SET_CER_RN1                  =  45, /*!< For SD Card only */
			OP_SD_APP_GET_CER_RN2                  =  46, /*!< For SD Card only */
			OP_SD_APP_SET_CER_RES2                 =  47, /*!< For SD Card only */
			OP_SD_APP_GET_CER_RES1                 =  48, /*!< For SD Card only */
			OP_SD_APP_SECURE_READ_MULTIPLE_BLOCK   =  18, /*!< For SD Card only */
			OP_SD_APP_SECURE_WRITE_MULTIPLE_BLOCK  =  25, /*!< For SD Card only */
			OP_SD_APP_SECURE_ERASE                 =  38, /*!< For SD Card only */
			OP_SD_APP_CHANGE_SECURE_AREA           =  49, /*!< For SD Card only */
			OP_SD_APP_SECURE_WRITE_MKB             =  48  /*!< For SD Card only */
	};
private:
	void set_flags( op opcode)
	{
		switch( opcode )
		{
			case OP_GO_IDLE_STATE: 		m_flags = resp_R1B|resp_spi_R1B; break;
			case OP_SEND_IF_COND:		m_flags = resp_R1|resp_spi_R1; break;
			default: 					m_flags = resp_none;
		}
	}
public:
	mmc_command( op opcode, uint32_t arg )
	: m_arg(arg), m_opcode(opcode)
	{
		set_flags( opcode );
	}
	void operator()( op opcode, uint32_t arg )
	{
		m_opcode = opcode;
		m_arg = arg;
		set_flags(opcode);
	}
	//Get opcode
	uint8_t get_op() const { return m_opcode; }
	//Get argument
	uint32_t get_arg() const { return m_arg; }
	//Get flags
	unsigned get_flags() const { return m_flags; }

	enum resp_type
	{
		resp_none	  = 0,			//! Dont except the response
		resp_present  = 1 << 0,		//! Response present
		resp_136	  = 1 << 1,		//! Response 136 bit
		resp_crc	  = 1 << 2,		//! Excepted valid CRC
		resp_busy	  = 1 << 3,		//! Card can send busy
		resp_opcode	  = 1 << 4,		//! Resp has opcode
		resp_spi_s1	  = 1 << 5,		//! First byte
		resp_spi_s2	  = 1 << 6,		//! Second byte
		resp_spi_b4	  = 1 << 7,		//! Four data bytes
		resp_spi_busy = 1 << 8,		//! Wait for response
		resp_spi_nocs = 1 << 9,		//! Don't disable CS
		resp_ans_spi  = 1 << 10,	//! SPI answer
		resp_ans  	  = 1 << 11,	//! SPI answer
		/* Standard responses */
		resp_R1   = (resp_present |resp_crc|resp_opcode|(1<<24)),
		resp_R1B  = (resp_R1|resp_busy|(1<<24)),
		resp_R2   = (resp_present|resp_136|resp_crc|(2<<24)),
		resp_R3   =  resp_present|(3<<24),
		resp_R4   =  resp_present|(4<<24),
		resp_R5   =  (resp_present|resp_crc|resp_opcode|(5<<24)),
		resp_R6   =  (resp_present|resp_crc|resp_opcode|(6<<24)),
		resp_R7   =  (resp_present|resp_crc|resp_opcode| resp_spi_busy|(7<<24)),
		resp_spi_R1  =  resp_spi_s1|(1<<24),
		resp_spi_R1B = (resp_spi_s1|resp_spi_busy|(1<<24)),
		resp_spi_R2  = (resp_spi_s1|resp_spi_s2|(2<<24)),
		resp_spi_R3  = (resp_spi_s1|resp_spi_b4|(3<<24)),
		resp_spi_R4  = (resp_spi_s1|resp_spi_b4|(4<<24)),
		resp_spi_R5  = (resp_spi_s1|resp_spi_s2|(5<<24)),
		resp_spi_R7  = (resp_spi_s1|resp_spi_b4|(7<<24))
	};
	int get_type() const
	{
		return (m_flags >> 24)&0xff;
	}
	bool is_spi_type() const
	{
		return m_flags & resp_ans_spi;
	}
	void set_resp_spi_r1( uint8_t r1 )
	{
		m_resp[0] = r1;
		m_flags |=resp_ans_spi|resp_ans;
	}
	void set_resp_spi_r2(uint8_t r1, uint8_t r2 )
	{
		m_resp[0] = (static_cast<uint32_t>(r1) << 8 ) | r2;
		m_flags |=resp_ans_spi|resp_ans;
	}
	void set_resp_spi_b4(uint8_t r1, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4 )
	{
		m_resp[0] = r1;
		m_resp[1] = (static_cast<uint32_t>(b1) << 24 ) |
		(static_cast<uint32_t>(b2) << 16 ) | (static_cast<uint32_t>(b3) << 8 ) | b4;
		m_flags |=resp_ans_spi|resp_ans;
	}
	mmc::err get_err() const
	{
		if( !(m_flags & resp_ans) )
		{
			return mmc::MMC_CMD_RSP_TIMEOUT;
		}
		if( is_spi_type() )
		{
			if( (m_flags&resp_spi_s1) && !(m_flags&resp_spi_s2) )
			{

				if( m_resp[0] & bR1_ERASE_RESET )
				{
					return mmc::MMC_ERASE_SEQ_ERR;
				}
				else if( m_resp[0] & bR1_ILLEGAL_COMMAND )
				{
					return mmc::MMC_ILLEGAL_CMD;
				}
				else if( m_resp[0] & bR1_COM_CRC_ERROR )
				{
					return mmc::MMC_CMD_CRC_FAIL;
				}
				else if( m_resp[0] & bR1_ADDRESS_ERROR )
				{
					return mmc::MMC_ADDR_MISALIGNED;
				}
				else if( m_resp[0] & bR1_PARAMETER_ERROR )
				{
					return mmc::MMC_INVALID_PARAMETER;
				}
				else if( m_resp[0] & bR1_ERASE_SEQ_ERROR )
				{
					return mmc::MMC_ERASE_SEQ_ERR;
				}
				else if(m_resp[0] & bR1_IN_IDLE_STATE)
				{
					return mmc::MMC_IN_IDLE_STATE;
				}
				else if( m_resp == 0 )
				{
					return mmc::MMC_OK;
				}
				else
				{
					return mmc::MMC_INTERNAL_ERROR;
				}
			}
			else
			{
				return mmc::MMC_INTERNAL_ERROR;
			}
		}
	}
private:
	uint32_t m_arg;					//Command argument
	uint32_t m_resp[4];				//Data in response
	unsigned long m_flags;			//Flags
	uint8_t m_opcode;				//Opcode
};

/*----------------------------------------------------------*/

} /* namespace drv */
/*----------------------------------------------------------*/
#endif /* MMC_COMMAND_HPP_ */
/*----------------------------------------------------------*/
