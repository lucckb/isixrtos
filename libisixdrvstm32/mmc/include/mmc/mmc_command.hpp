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
#include <foundation/algo/noncopyable.hpp>
#include <stdint.h>
#include <cstddef>
#include "mmc/mmc_error_codes.hpp"
#include "mmc/mmc_defs.hpp"
/*----------------------------------------------------------*/
namespace drv {
namespace mmc {
/*----------------------------------------------------------*/
struct cid;
/*----------------------------------------------------------*/
class mmc_command: public fnd::noncopyable
{
private:	/* Private response codes */
	enum resp_code
	{
	    rR1t = 1, rR2t, rR3t, rR4t, rR5t, rR6t, rR7t
	};
	enum r1_spi_bits
	{
		bR1_IN_IDLE_STATE 	= 1<<0,
		bR1_ERASE_RESET   	= 1<<1,
		bR1_ILLEGAL_COMMAND = 1<<2,
		bR1_COM_CRC_ERROR   = 1<<3,
		bR1_ERASE_SEQ_ERROR = 1<<4,
		bR1_ADDRESS_ERROR   = 1<<5,
		bR1_PARAMETER_ERROR = 1<<6
	};
	enum r1_sd_bits
	{
	  sR1E_AKE_SEQ_ERROR	= 	1<<3,
	  sR1_APP_CMD		=       1<<5,
	  sR1_READY_FOR_DATA	=	1<<8,
	  sR1_CURRENT_STATE     = 	0x1E00,
	  sR1_ERASE_RESET 	=	1<<13,
	  sR1_CARD_ECC_DISABLED =	1<<14,
	  sR1E_WP_ERASE_SKIP	= 	1<<15,
	  sR1E_CSD_OVERWRITE	=	1<<16,
	  sR1E_ERROR		=	1<<19,
	  sR1E_CC_ERROR		=	1<<20,
	  sR1E_CARD_ECC_FAILED	=	1<<21,
	  sR1E_ILLEGAL_COMMAND	=	1<<22,
	  sR1E_COM_CRC_ERROR	=	1<<23,
	  sR1E_LOCK_UNLOCK_FAILED =	1<<24,
	  sR1_CARD_IS_LOCKED	=	1<<25,
	  sR1E_WP_VIOLATION	=       1<<26,
	  sR1E_ERASE_PARAM	= 	1<<27,
	  sR1E_ERASE_SEQ_ERROR  =	1<<28,
	  sR1E_BLOCK_LEN_ERROR	=	1<<29,
	  sR1E_ADDRESS_ERROR	=       1<<30,
	  sR1E_OUT_OF_RANGE	= 	1<<31
	};
	enum r2_spi_bits
	{
		bR2_CARD_IS_LOCKED  = (1<<0),
		bR2_WP_ERASE_SKIP   = (1<<1),
		bR2_ERROR		    = (1<<2),
		bR2_CC_ERROR	    = (1<<3),
		bR2_ECC_FAILED      = (1<<4),
		bR2_WP_VIOLATION    = (1<<5),
		bR2_ERASE_PARAM	    = (1<<6),
		bR2_OUT_OF_RANGE    = (1<<7),
		bR2_IN_IDLE_STATE   = (1<<8),
		bR2_ERASE_RESET	    = (1<<9),
		bR2_ILLEGAL_COMMAND = (1<<10),
		bR2_COM_CRC_ERROR	= (1<<11),
		bR2_ERASE_SEQ_ERROR = (1<<12),
		bR2_ADDRESS_ERROR   = (1<<13),
		bR2_PARAMETER_ERROR = (1<<14)
	};
    enum sd_r6_bits
    {
        sdR6_GENERAL_UNKNOWN_ERROR = 0x00002000,
        sdR6_ILLEGAL_CMD           = 0x00004000,
        sdR6_COM_CRC_FAILED        = 0x00008000,
        sdR6_ERROR_MASK = sdR6_GENERAL_UNKNOWN_ERROR|sdR6_ILLEGAL_CMD|sdR6_COM_CRC_FAILED
    };
	static const uint32_t bR1_ERROR_MASK = bR1_ERASE_RESET|bR1_ILLEGAL_COMMAND|bR1_COM_CRC_ERROR|
			bR1_ERASE_SEQ_ERROR|bR1_ADDRESS_ERROR|bR1_PARAMETER_ERROR;
	static const uint32_t bR2_ERROR_MASK = bR2_WP_ERASE_SKIP|bR2_ERROR|
			bR2_ECC_FAILED|bR2_WP_VIOLATION|bR2_ERASE_PARAM|bR2_OUT_OF_RANGE|bR2_ERASE_RESET|
			bR2_ILLEGAL_COMMAND|bR2_COM_CRC_ERROR|bR2_ERASE_SEQ_ERROR|bR2_ADDRESS_ERROR|bR2_PARAMETER_ERROR;
	static const uint32_t sR1_ERROR_MASK = sR1E_AKE_SEQ_ERROR|sR1E_WP_ERASE_SKIP|sR1E_CSD_OVERWRITE|
			sR1E_ERROR|sR1E_CC_ERROR|sR1E_CARD_ECC_FAILED|sR1E_ILLEGAL_COMMAND|sR1E_COM_CRC_ERROR|
			sR1E_LOCK_UNLOCK_FAILED|sR1E_WP_VIOLATION|sR1E_ERASE_PARAM|sR1E_ERASE_SEQ_ERROR|
			sR1E_BLOCK_LEN_ERROR| sR1E_ADDRESS_ERROR|sR1E_OUT_OF_RANGE;
	static const uint32_t OCR_VOLTRANGE_MASK = 0xff8000;
	static const uint32_t OCR_CCS_MASK = 1<<30;
	static const uint32_t OCR_BUSY_MASK = 1<<31;
	static const uint32_t APP_FLAG = 0x80;
public:
  	//Card state
	enum card_state
	{
	  card_state_IDLE,
	  card_state_READY,
	  card_state_IDENT,
	  card_state_STBY,
	  card_state_TRAN,
	  card_state_DATA,
	  card_state_RCV,
	  card_state_PRG,
	  card_state_DIS
	};
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
			OP_SEND_IF_COND			       		   =  8,
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
			OP_APP_SD_SET_BUSWIDTH                 =  6 |APP_FLAG, /*!< For SD Card only */
			OP_SD_APP_STATUS                       =  13|APP_FLAG, /*!< For SD Card only */
			OP_SD_APP_SEND_NUM_WRITE_BLOCKS        =  22|APP_FLAG, /*!< For SD Card only */
			OP_SD_APP_OP_COND                      =  41|APP_FLAG, /*!< For SD Card only */
			OP_SD_APP_SET_CLR_CARD_DETECT          =  42|APP_FLAG, /*!< For SD Card only */
			OP_SD_APP_SEND_SCR                     =  51|APP_FLAG, /*!< For SD Card only */
			OP_SDIO_RW_DIRECT                      =  52, /*!< For SD I/O Card only */
			OP_SDIO_RW_EXTENDED                    =  53, /*!< For SD I/O Card only */
			OP_SDIO_READ_OCR					   =  58,
			OP_SD_APP_GET_MKB                      =  43|APP_FLAG, /*!< For SD Card only */
			OP_SD_APP_GET_MID                      =  44|APP_FLAG, /*!< For SD Card only */
			OP_SD_APP_SET_CER_RN1                  =  45|APP_FLAG, /*!< For SD Card only */
			OP_SD_APP_GET_CER_RN2                  =  46|APP_FLAG, /*!< For SD Card only */
			OP_SD_APP_SET_CER_RES2                 =  47|APP_FLAG, /*!< For SD Card only */
			OP_SD_APP_GET_CER_RES1                 =  48|APP_FLAG, /*!< For SD Card only */
			OP_SD_APP_SECURE_READ_MULTIPLE_BLOCK   =  18|APP_FLAG, /*!< For SD Card only */
			OP_SD_APP_SECURE_WRITE_MULTIPLE_BLOCK  =  25|APP_FLAG, /*!< For SD Card only */
			OP_SD_APP_SECURE_ERASE                 =  38|APP_FLAG, /*!< For SD Card only */
			OP_SD_APP_CHANGE_SECURE_AREA           =  49|APP_FLAG, /*!< For SD Card only */
			OP_SD_APP_SECURE_WRITE_MKB             =  48|APP_FLAG  /*!< For SD Card only */
	};
	static const uint32_t ARG_IFCOND_3V3_SUPPLY = 0x122;
	static const uint32_t ARG_OPCOND_HCS = ( 1<<30 );
	static const uint32_t ARG_OPCOND_VOLT_ALL = 0x008F8000;
private:
	void set_flags( op opcode, uint32_t arg )
	{
		switch( opcode )
		{
			case OP_GO_IDLE_STATE: 		m_flags = resp_spi_R1|resp_spi_cs;   break;
			case OP_SEND_IF_COND:		m_flags = resp_R7|resp_spi_R7|resp_spi_cs;   break;
			case OP_SDIO_READ_OCR:		m_flags = resp_R3|resp_spi_R3|resp_spi_cs;   break;
			case OP_APP_CMD:			m_flags = resp_R1|resp_spi_R1|resp_spi_cs;   break;
			case OP_SD_APP_OP_COND:		m_flags = resp_R3|resp_spi_R1|resp_spi_cs;   break;
			case OP_SEND_OP_COND:		m_flags = resp_R3|resp_spi_R1|resp_spi_cs;   break;
			case OP_CRC_ON_OFF:			m_flags = resp_spi_R1|resp_spi_cs; 		     break;
			case OP_SET_BLOCKLEN:		m_flags = resp_R1|resp_spi_R1|resp_spi_cs;   break;
			case OP_SET_BLOCK_COUNT:	m_flags = resp_R1;				  break;
			case OP_WRITE_MULT_BLOCK:   m_flags = resp_R1|resp_spi_R1;    break;
			case OP_WRITE_SINGLE_BLOCK: m_flags = resp_R1|resp_spi_R1;    break;
			case OP_READ_MULT_BLOCK:	m_flags = resp_R1|resp_spi_R1;    break;
			case OP_READ_SINGLE_BLOCK:  m_flags = resp_R1|resp_spi_R1;    break;
			case OP_STOP_TRANSMISSION:	m_flags = resp_R1B|resp_spi_R1B|resp_spi_cs;  break;
			case OP_SEND_STATUS:		m_flags = resp_R1|resp_spi_R2|resp_spi_cs;    break;
			case OP_SEND_CID:			m_flags = resp_R2|resp_spi_R1D|resp_spi_cs;   break;
			case OP_SEND_CSD:			m_flags = resp_R2|resp_spi_R1D|resp_spi_cs;   break;
			case OP_SD_APP_STATUS:		m_flags = resp_R1|resp_spi_R2;    break;
            case OP_ALL_SEND_CID:       m_flags = resp_R2;                break; 
            case OP_SET_REL_ADDR:       m_flags = resp_R6;                break;
            case OP_SEL_DESEL_CARD:     m_flags = arg?resp_R1B:resp_none; break;
            case OP_SD_APP_SEND_SCR:    m_flags = resp_R1|resp_spi_R1|resp_spi_cs;    break;
            case OP_APP_SD_SET_BUSWIDTH: m_flags = resp_R1; 		      break;
			default: 					m_flags = resp_none;			  break;
		}
	}
public:
	mmc_command( op opcode, uint32_t arg )
	: m_arg(arg), m_opcode(opcode)
	{
		set_flags( opcode, arg );
	}
	mmc_command()
		: m_arg(0), m_flags(0), m_opcode(0)
	{
	}
	void operator()( op opcode, uint32_t arg )
	{
		m_opcode = opcode;
		m_arg = arg;
		set_flags(opcode, arg);
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
		resp_spi_cs   = 1 << 9,		//! Disable CS
		resp_ans_spi  = 1 << 10,	//! SPI answer
		resp_ans      = 1 << 11,	//! answer
		resp_spi_d16b = 1 << 12,	//! Transfer extra 16bytes as data after R1
		/* Standard responses */
		resp_R1   = (resp_present|resp_crc|resp_opcode|(1<<29)),
		resp_R1B  = (resp_R1|resp_busy|(1<<29)),
		resp_R2   = (resp_present|resp_136|resp_crc|(2<<29)),
		resp_R3   =  resp_present|(3<<29),
		resp_R4   =  resp_present|(4<<29),
		resp_R5   =  (resp_present|resp_crc|resp_opcode|(5<<29)),
		resp_R6   =  (resp_present|resp_crc|resp_opcode|(6<<29)),
		resp_R7   =  (resp_present|resp_crc|resp_opcode|resp_spi_busy|(7<<29)),
		resp_spi_R1   =  resp_spi_s1|(1<<24),
		resp_spi_R1B  = (resp_spi_s1|resp_spi_busy|(1<<26)),
		resp_spi_R1D  =  resp_spi_s1|resp_spi_d16b|(1<<26),		//! Extra version transfer 16b data as csd
		resp_spi_R2   = (resp_spi_s1|resp_spi_s2|(2<<26)),
		resp_spi_R3   = (resp_spi_s1|resp_spi_b4|(3<<26)),
		resp_spi_R4   = (resp_spi_s1|resp_spi_b4|(4<<26)),
		resp_spi_R5   = (resp_spi_s1|resp_spi_s2|(5<<26)),
		resp_spi_R7   = (resp_spi_s1|resp_spi_b4|(7<<26))
	};
	int get_type() const
	{
		if( is_spi_type() )
			return (m_flags >> 26)&0x07;
		else
			return (m_flags >> 29)&0x07;
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
	void set_resp_short( uint32_t r1 )
	{
		m_resp[0] = r1;
		m_resp[1] = r1;
		m_flags |= resp_ans;
	}
	void set_resp_long( uint32_t r1, uint32_t r2, uint32_t r3, uint32_t r4 )
	{
		m_resp[0] = r1; m_resp[1] = r2; m_resp[2] = r3; m_resp[3] = r4;
		m_flags |= resp_ans;
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
	//Get card error
	err get_err() const;
	//Get card status
	int get_card_state() const;
	//Check if ready for data
	int get_busy_r1() const;
	//Validate R7
	err validate_r7() const
	{
		if( get_type() != rR7t )
			return MMC_CMD_MISMATCH_RESPONSE;
		if( is_spi_type() )
		{
			if( (m_resp[1] & 0xf00) != (ARG_IFCOND_3V3_SUPPLY & 0xf00) )
				return MMC_INVALID_VOLTRANGE;
			if( (m_resp[1] & 0xff) != (ARG_IFCOND_3V3_SUPPLY & 0xff) )
				return MMC_CMD_MISMATCH_RESPONSE;
		}
		return MMC_OK;
	}
    //Vaildate R3 response
	err validate_r3() const
	{
		if( get_type() != rR3t )
			return MMC_CMD_MISMATCH_RESPONSE;
		if( !(m_resp[1] & OCR_VOLTRANGE_MASK) )
			return MMC_INVALID_VOLTRANGE;
		return MMC_OK;
	}
    //Get R3 CCS
	bool get_r3_ccs() const
	{
		return m_resp[1] & OCR_CCS_MASK;
	}
    //Validate R6 response
    int validate_r6(uint16_t &rca);
    //Get response buffer
	uint32_t* get_resp_buffer(size_t pos = 0 ) { return &m_resp[pos]; }
	//Set response status
	void set_resp_spi_status();
	//Decode card CID
	int decode_cid (cid &c, bool mmc) const;
	//Get sectors count
	int decode_csd_sectors(uint32_t &nsectors, bool mmc) const;
	//Decode csd_erase
	int decode_csd_erase( uint32_t &erase_sects, bool mmc );
	//Decode ststats
	static uint32_t decode_sdstat_erase( uint32_t buf[] );
    //Decode SCR bus width
    int decode_scr( scr &scr_reg );
	//Decode tran speed
	int decode_csd_tran_speed( uint32_t &tran_speed );
private:
	uint32_t m_arg;					//Command argument
	uint32_t m_resp[4];				//Data in response
	unsigned long m_flags;				//Flags
	uint8_t m_opcode;				//Opcode
};

/*----------------------------------------------------------*/
}
} /* namespace drv */
/*----------------------------------------------------------*/
#endif /* MMC_COMMAND_HPP_ */
/*----------------------------------------------------------*/
