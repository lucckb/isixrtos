/*
 * mmc_command.cpp
 *
 *  Created on: 08-12-2012
 *      Author: lucck
 */
/*----------------------------------------------------------*/
#include "mmc/mmc_command.hpp"
#include <cstddef>
#include "mmc/mmc_defs.hpp"
#include <foundation/sys/dbglog.h>
/*----------------------------------------------------------*/
namespace drv {
namespace mmc {

/*----------------------------------------------------------*/
//Unstuff bits for decode card type
namespace {
	//Unstuff bits
	inline uint32_t UNSTUFF_BITS(const uint32_t resp[] , size_t start, const size_t size)
	{
		const uint32_t mask = (size < 32 ? 1 << size : 0) - 1;
		const int off = 3 - ((start) / 32);
		const int shft = (start) & 31;
		uint32_t res = resp[off] >> shft;
		if (size + shft > 32)
			res |= resp[off-1] << ((32 - shft) % 32);
		return res & mask;
	}
	inline void be32_to_cpu(uint32_t val[], size_t len )
	{
	#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		for(size_t p=0; p<len; p++ )
			val[p]=  __builtin_bswap32( val[p] );
	#endif
	}
}
/*----------------------------------------------------------*/
err mmc_command::get_err() const
{
	if( !(m_flags & resp_ans) && (m_flags & resp_present) )
	{
		return MMC_CMD_RSP_TIMEOUT;
	}
	if( is_spi_type() )
	{
		if( (m_flags&resp_spi_s1) && !(m_flags&resp_spi_s2) )
		{
			if( !(m_resp[0] & bR1_ERROR_MASK) )
			{
				return MMC_OK;
			}
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
			return MMC_CMD_MISMATCH_RESPONSE;
		}
		else if(  (m_flags&resp_spi_s1) && (m_flags&resp_spi_s2) )
		{
			if( !(m_resp[0] & bR2_ERROR_MASK) )
			{
				return MMC_OK;
			}
			if( m_resp[0] &	bR2_WP_ERASE_SKIP )
			{
				return MMC_WP_ERASE_SKIP;
			}
			if( m_resp[0] & bR2_ERROR )
			{
				return MMC_GENERAL_UNKNOWN_ERROR;
			}
			if( m_resp[0] & bR2_CC_ERROR )
			{
				return MMC_CC_ERROR;
			}
			if( m_resp[0] & bR2_ECC_FAILED )
			{
				return MMC_CARD_ECC_FAILED;
			}
			if( m_resp[0] & bR2_WP_VIOLATION )
			{
				return MMC_WRITE_PROT_VIOLATION;
			}
			if( m_resp[0] & bR2_ERASE_PARAM )
			{
				return MMC_BAD_ERASE_PARAM;
			}
			if( m_resp[0] & bR2_OUT_OF_RANGE )
			{
				return MMC_CMD_OUT_OF_RANGE;
			}
			if( m_resp[0] &	bR2_ERASE_RESET )
			{
				return MMC_ERASE_RESET;
			}
			if( m_resp[0] &	bR2_ILLEGAL_COMMAND )
			{
				return MMC_ILLEGAL_CMD;
			}
			if( m_resp[0] &	bR2_COM_CRC_ERROR )
			{
				return MMC_COM_CRC_FAILED;
			}
			if( m_resp[0] & bR2_ERASE_SEQ_ERROR )
			{
				return MMC_ERASE_SEQ_ERR;
			}
			if( m_resp[0] &	bR2_ADDRESS_ERROR )
			{
				return MMC_ADDR_OUT_OF_RANGE;
			}
			if( m_resp[0] & bR2_PARAMETER_ERROR )
			{
				return MMC_INVALID_PARAMETER;
			}
			return MMC_CMD_MISMATCH_RESPONSE;
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
	    		  if( !(m_resp[0] & sR1_ERROR_MASK) )
	    		  {
	    			  return MMC_OK;
	    		  }
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
				  return MMC_CMD_MISMATCH_RESPONSE;
			  }
	    	  else if( get_type()==rR3t )
	    	  {
	    		  return MMC_OK;
	    	  }
	      }
	}
	return MMC_OK;
}

/*----------------------------------------------------------*/
int mmc_command::get_busy_r1() const
{
	if( !(m_flags & resp_ans) && (m_flags & resp_present) )
		return MMC_CMD_RSP_TIMEOUT;
	if( is_spi_type() )
		return MMC_INTERNAL_ERROR;
	if( get_type() != rR1t )
	{
		return MMC_CMD_MISMATCH_RESPONSE;
	}
	if( m_resp[0] & sR1_READY_FOR_DATA )
	{
		return MMC_OK;
	}
	else
	{
		return MMC_CARD_BUSY;
	}
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
void mmc_command::set_resp_spi_status()
{
	m_flags |=resp_ans_spi|resp_ans;
	be32_to_cpu( m_resp, sizeof(m_resp)/sizeof(m_resp[0]) );
}
/*----------------------------------------------------------*/
//Decode card CID
int mmc_command::decode_cid( cid &c , bool mmc ) const
{
	if( !(m_flags & resp_ans) )
	{
	    dbprintf("decode MMC_CMD_RSP_TIMEOUT");
		return MMC_CMD_RSP_TIMEOUT;
	}
	if( !(m_flags & resp_spi_d16b) || !(m_flags &resp_136))
	{
		dbprintf("decode MMC_CMD_MISMATCH_RESPONSE");
		return MMC_CMD_MISMATCH_RESPONSE;
	}
	if( !mmc )
	{
		c.manfid        = UNSTUFF_BITS(m_resp, 120, 8);
		c.oemid         = UNSTUFF_BITS(m_resp, 104, 16);
		c.prod_name[0]  = UNSTUFF_BITS(m_resp, 96, 8);
		c.prod_name[1]  = UNSTUFF_BITS(m_resp, 88, 8);
		c.prod_name[2]  = UNSTUFF_BITS(m_resp, 80, 8);
		c.prod_name[3]  = UNSTUFF_BITS(m_resp, 72, 8);
		c.prod_name[4]  = UNSTUFF_BITS(m_resp, 64, 8);
		c.prod_name[5]  = 0;
		c.hwrev         = UNSTUFF_BITS(m_resp, 60, 4);
		c.fwrev         = UNSTUFF_BITS(m_resp, 56, 4);
		c.serial        = UNSTUFF_BITS(m_resp, 24, 32);
		c.year          = UNSTUFF_BITS(m_resp, 12, 8);
		c.month         = UNSTUFF_BITS(m_resp, 8, 4);
		c.year += 2000; /* SD cards year offset */
	}
	else
	{
		 c.manfid        = UNSTUFF_BITS(m_resp, 120, 8);
		 c.oemid         = UNSTUFF_BITS(m_resp, 104, 16);
		 c.prod_name[0]  = UNSTUFF_BITS(m_resp, 96, 8);
		 c.prod_name[1]  = UNSTUFF_BITS(m_resp, 88, 8);
		 c.prod_name[2]  = UNSTUFF_BITS(m_resp, 80, 8);
		 c.prod_name[3]  = UNSTUFF_BITS(m_resp, 72, 8);
		 c.prod_name[4]  = UNSTUFF_BITS(m_resp, 64, 8);
		 c.prod_name[5]  = 0;
		 c.hwrev     = UNSTUFF_BITS(m_resp, 44, 4);
		 c.fwrev     = UNSTUFF_BITS(m_resp, 40, 4);
		 c.serial    = UNSTUFF_BITS(m_resp, 16, 24);
		 c.month     = UNSTUFF_BITS(m_resp, 12, 4);
		 c.year      = UNSTUFF_BITS(m_resp, 8, 4) + 1997;
	}
	return MMC_OK;
}
/*----------------------------------------------------------*/
//Get sectors count from CSD struct
int mmc_command::decode_csd_sectors(uint32_t &nsectors, bool mmc ) const
{
	if( !(m_flags & resp_ans) )
	{
	    dbprintf("decode MMC_CMD_RSP_TIMEOUT");
		return MMC_CMD_RSP_TIMEOUT;
	}
	if( !(m_flags & resp_spi_d16b) || !(m_flags & resp_136))
	{
		dbprintf("decode MMC_CMD_MISMATCH_RESPONSE");
		return MMC_CMD_MISMATCH_RESPONSE;
	}
	unsigned csd_struct = UNSTUFF_BITS(m_resp, 126, 2);
	if( csd_struct == 0 || mmc)
	{
	     unsigned e = UNSTUFF_BITS(m_resp, 47, 3);
	     unsigned m = UNSTUFF_BITS(m_resp, 62, 12);
	     unsigned bl =  1<< UNSTUFF_BITS(m_resp, 80, 4);
	     nsectors     = ((1 + m) << (e + 2)) * (bl / 512);
	}
	else if( csd_struct == 1 )
	{
		 uint32_t m = UNSTUFF_BITS(m_resp, 48, 22);
		 nsectors = (1 + m) << 10;
	}
	else
	{
		dbprintf("CSD_VER=%i", csd_struct);
		return MMC_UNRECOGNIZED_CSD;
	}
	return MMC_OK;
}
/*----------------------------------------------------------*/
//Decode tran speed
int mmc_command::decode_csd_tran_speed( uint32_t &tran_speed )
{
	static const unsigned int tran_exp[] = {
	    10000,      100000,     1000000,    10000000,
	    0,      0,      0,      0
	};

	static const unsigned char tran_mant[] = {
	    0,  10, 12, 13, 15, 20, 25, 30,
	    35, 40, 45, 50, 55, 60, 70, 80,
	};
	if( !(m_flags & resp_ans) )
	{
		return MMC_CMD_RSP_TIMEOUT;
	}
	if( !(m_flags & resp_spi_d16b) || !(m_flags & resp_136))
	{
		return MMC_CMD_MISMATCH_RESPONSE;
	}
	unsigned m = UNSTUFF_BITS(m_resp, 99, 4);
	unsigned e = UNSTUFF_BITS(m_resp, 96, 3);
	tran_speed = tran_exp[e] * tran_mant[m];
	return MMC_OK;
}
/*----------------------------------------------------------*/
//Decode csd_erase
int mmc_command::decode_csd_erase( uint32_t &erase_sects, bool mmc )
{
	if( !(m_flags & resp_ans) )
	{
		return MMC_CMD_RSP_TIMEOUT;
	}
	if( !(m_flags & resp_spi_d16b) || !(m_flags & resp_136))
	{
		return MMC_CMD_MISMATCH_RESPONSE;
	}
	if( !mmc )
	{
		uint32_t write_blkbits = UNSTUFF_BITS(m_resp, 22, 4);
		if( UNSTUFF_BITS( m_resp, 46, 1) )
		{
			erase_sects = 1;
		}
		else if( write_blkbits >=9)
		{
			erase_sects = UNSTUFF_BITS(m_resp, 39, 7) + 1;
			erase_sects <<= write_blkbits - 9;
		}
	}
	else
	{
		uint32_t write_blkbits = UNSTUFF_BITS(m_resp, 22, 4);
		if (write_blkbits >= 9)
		{
			uint32_t a = UNSTUFF_BITS(m_resp, 42, 5);
			uint32_t b = UNSTUFF_BITS(m_resp, 37, 5);
			erase_sects = (a + 1) * (b + 1);
			erase_sects <<= write_blkbits - 9;
		}
	}
	return MMC_OK;
}

/*----------------------------------------------------------*/
//Decode ststats erase
uint32_t mmc_command::decode_sdstat_erase( uint32_t buf[] )
{
	be32_to_cpu( buf, 16 );
	uint32_t au = UNSTUFF_BITS(buf, 428 - 384, 4);
	if( au > 0 )
		au = (16384<<(au-1)) / 512;
	return au;
}
/*----------------------------------------------------------*/
//Validate R6 response
int mmc_command::validate_r6(uint16_t &rca)
{
    if( !(m_flags & resp_ans) )
	{
		return MMC_CMD_RSP_TIMEOUT;
	}
    if( is_spi_type() )
    {
        return MMC_CMD_MISMATCH_RESPONSE;
    }
    if( !(m_resp[0]&sdR6_ERROR_MASK) )
    {
        rca = m_resp[0] >> 16;
        return MMC_OK;
    }
    if( m_resp[0] & sdR6_GENERAL_UNKNOWN_ERROR )
    {
        return MMC_GENERAL_UNKNOWN_ERROR;
    }
    if( m_resp[0] & sdR6_ILLEGAL_CMD )
    {
        return MMC_ILLEGAL_CMD;
    }
    if( m_resp[0] & sdR6_COM_CRC_FAILED )
    {
        return MMC_COM_CRC_FAILED;
    }
    return MMC_OK;
}
/*----------------------------------------------------------*/
//Decode SCR bus width
int mmc_command::decode_scr( scr &scr_reg )
{
	if( !(m_flags & resp_ans) )
	{
		return MMC_CMD_RSP_TIMEOUT;
	}
	be32_to_cpu( &m_resp[2], 2 );
	if( UNSTUFF_BITS(m_resp, 60, 4) != 0)
    {
    	return MMC_UNRECOGNIZED_SCR;
    }
    const uint32_t sd_spec = UNSTUFF_BITS(m_resp, 59, 4 );
    const uint32_t sd_spec3 = UNSTUFF_BITS(m_resp, 47, 1);
    if( sd_spec > 2 )
    {
    	return MMC_UNRECOGNIZED_SCR;
    }
    if(sd_spec3)
    	scr_reg.spec = 0x30;
    else
    {
    	switch(sd_spec)
    	{
    	case 0: scr_reg.spec = 0x10; break;
    	case 1: scr_reg.spec = 0x11; break;
    	case 2: scr_reg.spec = 0x20; break;
    	}
    }
    const uint32_t bus_width = UNSTUFF_BITS(m_resp, 48, 4 );
    scr_reg.bus_width_1b = bus_width&0x01;
    scr_reg.bus_width_4b = (bus_width>>2)&0x01;
    const uint32_t cmd_class = UNSTUFF_BITS(m_resp, 32, 2 );
    scr_reg.is_set_block_count = (cmd_class >> 1)&1;
    scr_reg.is_speed_class = cmd_class & 1;
    return MMC_OK;
}
/*----------------------------------------------------------*/
}
}
/*----------------------------------------------------------*/
