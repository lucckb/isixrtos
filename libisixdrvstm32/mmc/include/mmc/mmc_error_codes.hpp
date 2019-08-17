/*
 * mmc_error_codes.hpp
 *
 *  Created on: 03-12-2012
 *      Author: lucck
 */
 
#ifndef MMC_ERROR_CODES_HPP_
#define MMC_ERROR_CODES_HPP_
 
namespace drv
{
	namespace mmc
	{
		enum err
		{
			MMC_OK 					=  0,
			MMC_CMD_CRC_FAIL        =  8192, 	/*!< Command response received (but CRC check failed) */
			MMC_DATA_CRC_FAIL = 8193,                   /*!< Data bock sent/received (CRC check Failed) */
			MMC_CMD_RSP_TIMEOUT = 8194,                /*!< Command response timeout */
			MMC_DATA_TIMEOUT = 8195,                   /*!< Data time out */
			MMC_TX_UNDERRUN = 8196,                   /*!< Transmit FIFO under-run */
			MMC_RX_OVERRUN = 8197,                     /*!< Receive FIFO over-run */
			MMC_START_BIT_ERR = 8198,                  /*!< Start bit not detected on all data signals in widE bus mode */
			MMC_CMD_OUT_OF_RANGE = 8199,               /*!< CMD's argument was out of range.*/
			MMC_ADDR_MISALIGNED = 8200,                /*!< Misaligned address */
			MMC_BLOCK_LEN_ERR = 8201,                  /*!< Transferred block length is not allowed for the card or the number of transferred bytes does not match the block length */
			MMC_ERASE_SEQ_ERR = 8202,                  /*!< An error in the sequence of erase command occurs.*/
			MMC_BAD_ERASE_PARAM = 8203,                /*!< An Invalid selection for erase groups */
			MMC_WRITE_PROT_VIOLATION = 8204,           /*!< Attempt to program a write protect block */
			MMC_LOCK_UNLOCK_FAILED = 8205,             /*!< Sequence or password error has been detected in unlock command or if there was an attempt to access a locked card */
			MMC_COM_CRC_FAILED = 8206,                 /*!< CRC check of the previous command failed */
			MMC_ILLEGAL_CMD = 8207,                    /*!< Command is not legal for the card state */
			MMC_CARD_ECC_FAILED = 8208,                /*!< Card internal ECC was applied but failed to correct the data */
			MMC_CC_ERROR = 8209,                       /*!< Internal card controller error */
			MMC_GENERAL_UNKNOWN_ERROR = 8210,          /*!< General or Unknown error */
			MMC_STREAM_READ_UNDERRUN  = 8211,          /*!< The card could not sustain data transfer in stream read operation. */
			MMC_STREAM_WRITE_OVERRUN = 8212,           /*!< The card could not sustain data programming in stream mode */
			MMC_CID_CSD_OVERWRITE = 8213,              /*!< CID/CSD overwrite error */
			MMC_WP_ERASE_SKIP = 8214,                  /*!< only partial address space was erased */
			MMC_CARD_ECC_DISABLED = 8215,              /*!< Command has been executed without using internal ECC */
			MMC_ERASE_RESET  = 8216,                    /*!< Erase sequence was cleared before executing because an out of erase sequence command was received */
			MMC_AKE_SEQ_ERROR  = 8217,                  /*!< Error in sequence of authentication. */
			MMC_INVALID_VOLTRANGE = 8218,
			MMC_ADDR_OUT_OF_RANGE  = 8219,
			MMC_SWITCH_ERROR  = 8220,
			MMC_SDIO_DISABLED  = 8221,
			MMC_SDIO_FUNCTION_BUSY  = 8222,
			MMC_SDIO_FUNCTION_FAILED  = 8223,
			MMC_SDIO_UNKNOWN_FUNCTION  = 8224,
			MMC_INTERNAL_ERROR  = 8225,
			MMC_NOT_CONFIGURED  = 8226,
			MMC_REQUEST_PENDING  = 8227,
			MMC_REQUEST_NOT_APPLICABLE  = 8228,
			MMC_INVALID_PARAMETER  = 8229,
			MMC_UNSUPPORTED_FEATURE  = 8230,
			MMC_UNSUPPORTED_HW  = 8231,
			MMC_ERROR  = 8232,
			MMC_LIB_NOT_INITIALIZED  = 8233,
			MMC_LIB_ALREADY_INITIALIZED  = 8234,
			MMC_CMD_MISMATCH_RESPONSE  = 8235,
			MMC_CARD_NOT_PRESENT  = 8236,
			MMC_UNRECOGNIZED_CSD  = 8237,
			MMC_DATA_NOT_ACCEPTED = 8238,
            MMC_UNRECOGNIZED_SCR  = 8239,
            MMC_DATA_ERROR =		8240,
            MMC_CARD_BUSY  =        8241
		};
}}
 
#endif /* MMC_ERROR_CODES_HPP_ */
 
