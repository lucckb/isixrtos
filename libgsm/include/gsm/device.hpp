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
#include <gsm/event.hpp>
#include <gsm/hw_control.hpp>
#include <foundation/serial_port.hpp>
#include <gsm/datadefs.hpp>
#include <gsm/phonebook.hpp>
#include <gsm/containers.hpp>
#include <gsm/sms_message.hpp>
#include <gsm/phonebook.hpp>
#include <gsm/sms_store.hpp>
#include <functional>

namespace gsm_modem {

	//! Class which representing the GSM device
	class device {
	public:
		//! Capability flags 
		class cap {
			unsigned bits;
		public:
			cap& operator=(cap&) = delete;
			cap( cap& ) = delete;
			cap( unsigned bits_ )
				: bits(bits_) {}
			enum cap_ {
				hw_flow 	= 1<<0,	//! Hardware flow control
				sms_pdu 	= 1<<1,	//! SMS in pdu mode not suported yet
				hw_datamode = 1<<2,	//! Hardware data mode DTR/DSR
			}; 
			bool has_hw_flow() const {
				return bits & hw_flow;
			}
			bool has_sms_pdu() const {
				return bits & sms_pdu;
			}
			bool has_hw_data() const {
				return bits & hw_datamode;
			}
		};
		//Noncopyable
		device& operator=(device&) = delete;
		device( device& ) = delete;

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

		/** Reset the modem  */
		void reset() {
			m_hwctl.reset();
		}

		/**  Set character set to the modem
		 * @param[in] charset Charset notification number
		 * @return Error code
		 */
		int set_charset( const char* charset );

		/** Register to GSM network
		 * @param[in] pin Optional input pin
		 * @param[in] notify registration notify type
		 * @return Error code
		 */
		int register_to_network( const char *pin = nullptr, reg_notify notify=reg_notify::disabled );

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
		 * @param[in] gprs Gprs registration status if true CGREG instead of CREG
		 * @return registration code or failed if fatal
		 */
		int get_registration_status( bool gprs = false );


		/** Get phone IMEI
		 * @param[in] inp Input IMEI structure 
		 * @return Error code or 0 */
		int get_imei( imei_number& inp );

		/** Get phone imsi 
		 * @param[in] inp Input structure with imsi
		 * @return Error code or success */
		int get_imsi( imsi_number& inp );

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
		 * @param[in] sms Input message for sumbit
		 * @return Error code if negative or message ref if pos
		 */
		int send_sms( const sms_submit& sms );

		/** Get sms store 
		 * @return reference to sms store class */
		sms_store& get_sms_store() {
			return m_sms_store;
		}
		
		/** Set sms routing to TA
		 * sets routing of SMS to TA to true for all supported SMSMessageTypes
		 if onlyReceptionIndication is set to true
		 only GsmEvent::SMSReceptionIndication is called
		 this has two reasons: GSM 07.05 section 3.4.1 does not recommend
		 direct routing of new SMS to the TA
		 CNMI=
		 */
		int set_sms_routing_to_ta( bool en_sms, bool en_cbs, bool en_stat_report, 
				bool only_reception_indication=true );

		//Wait for event
		int wait_event( int timeout ) {
			return m_at.wait( timeout );
		}

		//! Set event handler
		void set_event_handler( event* ev ) {
			m_at.set_event_handler( ev );
		}

		//! Switch to command mode if DSR/DTR not set ignore
		int command_mode( bool hang = false );
		
		//! Switch to data mode if DSR/DTR not set ignore
		int data_mode();

		/** Activate GPRS session and switch to data mode
		 * @param[in] query_apn function callback for query APN
		 * @return Error code
		 * @note query_apn callback should return const string
		 * to the callback name
		 */
		int connect_gprs( std::function<const char*()> apn_callback );


		/** Deactivate GPRS session and return to command mode 
		 * @return Error code
		 */
		int disconnect_gprs();
		
		/** Check if modem is currently in data mode */
		bool in_data_mode() const {
			return m_at.in_data_mode();
		}

	private:
		/** Private function for set and get text mode parameters
		 *  used only for text mode sms
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
		int do_disable() {
			m_hwctl.power_control( false );
			return 0;
		}
	private:
		at_parser m_at;			    //AT parser class
		hw_control& m_hwctl;		//Hardware ctl class
		phonebook m_phonebook;		//Phonebook object
		sms_store m_sms_store;		//Current sms store
		cap m_capabilities;			//Capability flags
	};
}
