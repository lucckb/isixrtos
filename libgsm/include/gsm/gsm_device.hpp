/*
 * =====================================================================================
 *
 *       Filename:  gsm_device.hpp
 *
 *    Description:  GSM device modem class
 *
 *        Version:  1.0
 *        Created:  28.02.2015 16:09:25
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once

#include <gsm/at_parser.hpp>
#include <gsm/gsm_event.hpp>
#include <gsm/hw_control.hpp>
#include <foundation/serial_port.hpp>
#include <gsm/datadefs.hpp>
#include <gsm/phonebook.hpp>
#include <gsm/containers.hpp>
#include <gsm/sms_message.hpp>

namespace gsm_modem {

	//! Class which representing the GSM device
	class device {
	public:
		//! Capability flags 
		class cap {
			unsigned bits;
		public:
			cap( unsigned bits_ )
				: bits(bits_) {}
			enum cap_ {
				hw_flow,	//! Hardware flow control
				sms_pdu,	//! SMS in pdu mode not suported yet
			}; 
			bool has_hw_flow() const {
				return bits & hw_flow;
			}
			bool has_sms_pdu() const {
				return bits & sms_pdu;
			}
		};
		/** Constructor of the GSM device instance
		 * @param comm Serial class for communication
		 * @param hwctl Class for hardware control modem device
		 */
		device( fnd::serial_port& comm, hw_control& hwctl, unsigned cap = 0 );
		/** Initialize and enable GSM device */
		int enable( bool enable ) {
			if(enable) return do_enable();
			else return do_disable();
		}
		//! Set character set to the modem
		int set_charset( const char* charset );
		//! Register to GSM network 
		int register_to_network( const char *pin = nullptr );

		//! Set pin
		int set_pin( const char* pin );

		/** Get pin status
		 * function return error code or sim req
		 */
		int get_pin_status();
		
		/** Get current operator information 
		 * @param[out] Information structure
		 * @return success or error code if failure
		 */
		int get_current_op_info( oper_info& info );

		/** Print registration status
		 * @return registration code or failed if fatal
		 */
		int get_registration_status();

		/** Get signal strength information
		 * @return signal stength or error code
		 * 0 -113 dBm or less
			1 -111 dBm
			2...30 -109... -53 dBm
			31 -51 dBm or greater
			99 not known or not detectable
		 */
		int get_signal_strength();
		
		/** Get at parser 
		 * @return at parser object for other device setups */
		at_parser& get_at() {
			return m_at;
		}
		
		/** Get phonebook object
		 * @return phonebook reference
		 */
		phonebook& get_phonebook() {
			return m_phonebook;
		}
		
		/** Send sms message and return
		 * error code if it is required
		 */
		int send_sms( const sms_submit& s );

	private:
		/** Private function for set and get text mode parameters
		 *  used only for text mode smses
		 *  @param[in] param Text mode input parameters
		 *  @return Error code if failed
		 */
		int get_text_mode_param_config( sms_text_params& param );
		int set_text_mode_param_config( const sms_text_params& param );

		//Do single command
		int send_command_noresp( const char *cmd, const char* arg );
		//Enable or disable device
		int do_enable();
		//Disable the gsm module
		int do_disable() 
		{
			m_hwctl.power_control( false );
			return 0;
		}
	private:
		at_parser m_at;			    //AT parser class
		hw_control& m_hwctl;		//Hardware ctl class
		event m_event;				//Events class dispatcher
		phonebook m_phonebook;		//Phonebook object
		cap m_capabilities;	//Capability flags
	};
}
