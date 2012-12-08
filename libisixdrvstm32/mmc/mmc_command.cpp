/*
 * mmc_command.cpp
 *
 *  Created on: 08-12-2012
 *      Author: lucck
 */
/*----------------------------------------------------------*/
#include "mmc/mmc_command.hpp"
/*----------------------------------------------------------*/
namespace drv {
namespace mmc {
/*----------------------------------------------------------*/
err mmc_command::get_err() const
{
	if( !(m_flags & resp_ans) )
	{
		return MMC_CMD_RSP_TIMEOUT;
	}
	if( is_spi_type() )
	{
		if( (m_flags&resp_spi_s1) && !(m_flags&resp_spi_s2) )
		{

			if( m_resp[0] & bR1_ERASE_RESET )
			{
				return MMC_ERASE_SEQ_ERR;
			}
			if( m_resp[0] & bR1_ILLEGAL_COMMAND )
			{
				return MMC_ILLEGAL_CMD;
			}
			if( m_resp[0] & bR1_COM_CRC_ERROR )
			{
			        return MMC_CMD_CRC_FAIL;
			}
			if( m_resp[0] & bR1_ADDRESS_ERROR )
			{
				return MMC_ADDR_MISALIGNED;
			}
			if( m_resp[0] & bR1_PARAMETER_ERROR )
			{
				return MMC_INVALID_PARAMETER;
			}
			if( m_resp[0] & bR1_ERASE_SEQ_ERROR )
			{
				return MMC_ERASE_SEQ_ERR;
			}
			if( m_resp[0] == 0 || (m_resp[0]&bR1_IN_IDLE_STATE) )
			{
				return MMC_OK;
			}
			else
			{
			    return MMC_CMD_MISMATCH_RESPONSE;
			}
		}
		else
		{
		    return MMC_CMD_MISMATCH_RESPONSE;
		}
	}
	else
	{
	      if( m_flags & resp_present )
	      {
	    	  if( get_type()==rR1t )
	    	  {
				  if( m_resp[0] & sR1E_OUT_OF_RANGE )
				  {
					return MMC_ADDR_OUT_OF_RANGE;
				  }
				  if( m_resp[0] & sR1E_ADDRESS_ERROR )
				  {
					return MMC_ADDR_MISALIGNED;
				  }
				  if( m_resp[0] & sR1E_BLOCK_LEN_ERROR )
				  {
					return MMC_BLOCK_LEN_ERR;
				  }
				  if( m_resp[0] & sR1E_ERASE_SEQ_ERROR )
				  {
					return MMC_ERASE_SEQ_ERR;
				  }
				  if( m_resp[0] & sR1E_ERASE_PARAM )
				  {
					return MMC_BAD_ERASE_PARAM;
				  }
				  if( m_resp[0] & sR1E_WP_VIOLATION )
				  {
					return MMC_WRITE_PROT_VIOLATION;
				  }
				  if( m_resp[0] & sR1E_LOCK_UNLOCK_FAILED )
				  {
					return MMC_LOCK_UNLOCK_FAILED;
				  }
				  if( m_resp[0] & sR1E_COM_CRC_ERROR )
				  {
					return MMC_CMD_CRC_FAIL;
				  }
				  if( m_resp[0] & sR1E_ILLEGAL_COMMAND )
				  {
					return MMC_ILLEGAL_CMD;
				  }
				  if( m_resp[0] & sR1E_CARD_ECC_FAILED )
				  {
					return MMC_CARD_ECC_FAILED;
				  }
				  if( m_resp[0] & sR1E_CC_ERROR )
				  {
					return MMC_CC_ERROR;
				  }
				  if( m_resp[0] & sR1E_ERROR )
				  {
					return  MMC_GENERAL_UNKNOWN_ERROR;
				  }
				  if( m_resp[0] & sR1E_CSD_OVERWRITE )
				  {
					return MMC_CID_CSD_OVERWRITE;
				  }
				  if( m_resp[0] &  sR1E_WP_ERASE_SKIP )
				  {
					return MMC_WP_ERASE_SKIP;
				  }
				  if( m_resp[0] &  sR1E_AKE_SEQ_ERROR )
				  {
					return MMC_AKE_SEQ_ERROR;
				  }
			  }
	    	  else if( get_type()==rR3t )
	    	  {
	    		  return MMC_OK;
	    	  }
	      }
	}
	return MMC_INTERNAL_ERROR;
}
/*----------------------------------------------------------*/
//Get card status
int mmc_command::get_card_state() const
{
    if( !(m_flags & resp_ans) )
    {
    	return MMC_CMD_RSP_TIMEOUT;
    }
    if( is_spi_type() )
    {
		if( (m_flags&resp_spi_s1) && !(m_flags&resp_spi_s2) )
		{
			if( m_resp[0] & bR1_IN_IDLE_STATE)
				return card_state_IDLE;
			else
				return card_state_READY;
		}
		else
		{
			 return MMC_CMD_MISMATCH_RESPONSE;
		}
    }
    else
    {
		if( get_type() == rR1t )
		{
			return ((m_resp[0] & sR1_CURRENT_STATE ) >> 9) & 0x0f;
		}
		else if( get_type() == rR3t )
		{
			return (!(m_resp[0]&OCR_BUSY_MASK))?(card_state_IDLE):(card_state_READY);
		}
		else
		{
			return MMC_CMD_MISMATCH_RESPONSE;
		}
    }
}
/*----------------------------------------------------------*/
}
}
/*----------------------------------------------------------*/
