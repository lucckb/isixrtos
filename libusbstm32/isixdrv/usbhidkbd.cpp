/*
 * usbhost.c
 *
 *  Created on: 20-06-2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#include <foundation/dbglog.h>
#include <isix.h>
#include <usbh_hid_core.h>
#include <usbhidkbd.hpp>

/* ------------------------------------------------------------------ */
namespace stm32 {
namespace dev {

/* ------------------------------------------------------------------ */
int hid_keyboard::get_repeat_settings( int& /*delay */, int& /*period*/ ) const
{

}
/* ------------------------------------------------------------------ */
int hid_keyboard::set_repeat_settings( int /*delay */, int /*period*/ )
{

}
/* ------------------------------------------------------------------ */
int hid_keyboard::hardware_led_enable(bool /*yes*/)
{
}
/* ------------------------------------------------------------------ */
int hid_keyboard::get_led( led_ctl /*led_id */, bool& /*value*/ ) const
{
}
/* ------------------------------------------------------------------ */
int hid_keyboard::set_led( led_ctl /*led_id */, bool /*value*/ )
{
}
/* ------------------------------------------------------------------ */
void hid_keyboard::generate_event( const uint8_t *req, std::size_t len )
{
	/*
	static uint8_t rep_out = 2;
	for( std::size_t a=0;a<len;a++)
	dbprintf("REQ %i --- %02X",a ,req[a]);
	if( req[2] == 0x07) rep_out = 2;
	else if( req[2] == 0x09 ) rep_out = 0;
	if( req[2] == 0x07 || req[2]== 0x09 )
	{
		const auto R = USBH_Set_Report( &gethost().usb_otg_dev, &gethost().stm32_host, 0x02, 0, 1, &rep_out);
		dbprintf("S %02X SEND R %i", rep_out, R);
		input_report_key( gfx::inp::input::KEY_PRESS, 12 );
	}
	*/
	for(size_t ix = 2; ix < len; ix++)
	{
		if( req[ix]==0x01 || req[ix]==0x02 || req[ix]==0x03 )
		{
			dbprintf("Error in report");
			return;
		}
	}
	auto nbr_keys     = 0;
	auto nbr_keys_new = 0;
	std::array<uint8_t, KEY_REPORT_LEN>  keys;
	std::array<uint8_t, KEY_REPORT_LEN>  keys_new;
	for (size_t ix = 2; ix < 2 + KEY_REPORT_LEN; ix++)
	{
	    if (req[ix] != 0)
	    {
	      keys[nbr_keys] = req[ix];
	      nbr_keys++;
	      size_t jx;
	      for (jx = 0; jx < m_nbr_keys_last; jx++)
	      {
	        if (req[ix] == m_keys_last[jx])
	          break;
	      }

	      if (jx == m_nbr_keys_last)
	      {
	        keys_new[nbr_keys_new] = req[ix];
	        nbr_keys_new++;
	      }
	    }
	  }

	  if (nbr_keys_new == 1)
	  {
		  auto key_newest = keys_new[0];
		  int status = input_report_key(  gfx::input::detail::keyboard_tag::status::DOWN, key_newest, req[0] );
		  dbprintf("New key %02X report %i", key_newest, status );
	  }

	  m_nbr_keys_last  = nbr_keys;
	  for (size_t ix = 0; ix < KEY_REPORT_LEN; ix++)
	  {
	    m_keys_last[ix] = keys[ix];
	  }
}
/* ------------------------------------------------------------------ */
}}
