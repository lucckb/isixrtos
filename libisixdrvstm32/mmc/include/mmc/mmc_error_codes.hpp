/*
 * mmc_error_codes.hpp
 *
 *  Created on: 03-12-2012
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#ifndef MMC_ERROR_CODES_HPP_
#define MMC_ERROR_CODES_HPP_
/* ------------------------------------------------------------------ */
namespace drv
{
	namespace mmc
	{
		enum err
		{
			MMC_OK 					=  0,
			MMC_CMD_CRC_FAIL        =  8192, 	/*!< Command response received (but CRC check failed) */
			MMC_DATA_CRC_FAIL,                   /*!< Data bock sent/received (CRC check Failed) */
			MMC_CMD_RSP_TIMEOUT,                /*!< Command response timeout */
			MMC_DATA_TIMEOUT,                   /*!< Data time out */
			MMC_TX_UNDERRUN ,                   /*!< Transmit FIFO under-run */
			MMC_RX_OVERRUN,                     /*!< Receive FIFO over-run */
			MMC_START_BIT_ERR,                  /*!< Start bit not detected on all data signals in widE bus mode */
			MMC_CMD_OUT_OF_RANGE,               /*!< CMD's argument was out of range.*/
			MMC_ADDR_MISALIGNED,                /*!< Misaligned address */
			MMC_BLOCK_LEN_ERR,                  /*!< Transferred block length is not allowed for the card or the number of transferred bytes does not match the block length */
			MMC_ERASE_SEQ_ERR,                  /*!< An error in the sequence of erase command occurs.*/
			MMC_BAD_ERASE_PARAM,                /*!< An Invalid selection for erase groups */
			MMC_WRITE_PROT_VIOLATION,           /*!< Attempt to program a write protect block */
			MMC_LOCK_UNLOCK_FAILED,             /*!< Sequence or password error has been detected in unlock command or if there was an attempt to access a locked card */
			MMC_COM_CRC_FAILED,                 /*!< CRC check of the previous command failed */
			MMC_ILLEGAL_CMD,                    /*!< Command is not legal for the card state */
			MMC_CARD_ECC_FAILED,                /*!< Card internal ECC was applied but failed to correct the data */
			MMC_CC_ERROR,                       /*!< Internal card controller error */
			MMC_GENERAL_UNKNOWN_ERROR,          /*!< General or Unknown error */
			MMC_STREAM_READ_UNDERRUN,          /*!< The card could not sustain data transfer in stream read operation. */
			MMC_STREAM_WRITE_OVERRUN,           /*!< The card could not sustain data programming in stream mode */
			MMC_CID_CSD_OVERWRITE,              /*!< CID/CSD overwrite error */
			MMC_WP_ERASE_SKIP,                  /*!< only partial address space was erased */
			MMC_CARD_ECC_DISABLED,              /*!< Command has been executed without using internal ECC */
			MMC_ERASE_RESET,                    /*!< Erase sequence was cleared before executing because an out of erase sequence command was received */
			MMC_AKE_SEQ_ERROR,                  /*!< Error in sequence of authentication. */
			MMC_INVALID_VOLTRANGE,
			MMC_ADDR_OUT_OF_RANGE,
			MMC_SWITCH_ERROR,
			MMC_SDIO_DISABLED,
			MMC_SDIO_FUNCTION_BUSY,
			MMC_SDIO_FUNCTION_FAILED,
			MMC_SDIO_UNKNOWN_FUNCTION,
			MMC_INTERNAL_ERROR,
			MMC_NOT_CONFIGURED,
			MMC_REQUEST_PENDING,
			MMC_REQUEST_NOT_APPLICABLE,
			MMC_INVALID_PARAMETER,
			MMC_UNSUPPORTED_FEATURE,
			MMC_UNSUPPORTED_HW,
			MMC_ERROR,
			MMC_LIB_NOT_INITIALIZED,
			MMC_LIB_ALREADY_INITIALIZED,
			MMC_CMD_MISMATCH_RESPONSE,
			MMC_CARD_NOT_PRESENT
		};
	}
}
/* ------------------------------------------------------------------ */
#endif /* MMC_ERROR_CODES_HPP_ */
/* ------------------------------------------------------------------ */
