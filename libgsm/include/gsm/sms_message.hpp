/*
 * =====================================================================================
 *
 *       Filename:  sms_message.hpp
 *
 *    Description:  SMS messages class definition
 *
 *        Version:  1.0
 *        Created:  02.03.2015 22:06:27
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once
#include <cstring>
#include <gsm/sms_codec.hpp>
#include <gsm/errors.hpp>

namespace gsm_modem {

		//http://www.smssolutions.net/tutorials/gsm/sendsmsat/
		class at_parser;

		//! Base classes for all smses
		class sms 
		{
			data_coding_scheme m_dcs;
		public:
			virtual ~sms() {
			}
			enum type {
				t_deliver,		//! Deliver message type
				t_submit,		//! Submit message type
				t_status_report, //! Status report message type
				t_command,		 //! Command type SMS
			};
			virtual int type() const = 0;
			virtual int encode( at_parser& /*p*/, bool /*pdu*/) { 
				return error::invalid_argument;
			};
			virtual int decode( at_parser& /*p*/, bool /*pdu*/) {
				return error::invalid_argument;
			}
			//Setter getters
			data_coding_scheme get_dcs( ) const {
				return m_dcs;
			}
			void set_dcs( data_coding_scheme dcs ) {
				m_dcs = dcs;
			}
		};

		// SMS-DELIVER TPDU
		class sms_deliver : public sms 
		{
		private:
			bool m_status_report_indication {};	//! Report indication
			char m_service_tstamp[24] {};		//! Service centre timestamp
			char m_origin_address[14] {};		//! Origin address
			unsigned char m_pid { 17 };			//!protocol identifier;
			char m_message[162] {};				//! Sms message data
		public:
			virtual ~sms_deliver() {
			}
			virtual int type() const {
				return sms::t_deliver;
			}
			int pid() const {
				return m_pid;
			}
			void pid( unsigned char pid ) {
				m_pid = pid;
			}
			const char* service_tstamp() const {
				return m_service_tstamp;
			}
			void service_tstamp( const char* tstamp ) {
				std::strncpy( m_service_tstamp, tstamp, sizeof(m_service_tstamp)-1);
			}
			bool report_indication() const {
				return m_status_report_indication;
			}
			void report_indication( bool ri ) {
				m_status_report_indication = ri;
			}
			const char* origin_address() const {
				return m_origin_address;
			}
			void origin_address( const char* addr ) {
				std::strncpy( m_origin_address, addr, sizeof(m_origin_address)-1 );
			}
			const char* message() const {
				return m_message;
			}
			void message( const char* msg ) {
				std::strncpy( m_message, msg, sizeof(m_message)-1 );
			}
		};

		// SMS-SUBMIT TPDU
		class sms_submit : public sms 
		{
		private:
			static constexpr auto c_sms_len = 160;
			unsigned char m_validity_period { 167 };
			bool m_status_report_request {};
			bool m_flash_message {};
			const char* m_dest_addr {};
			const char *m_message;
		public:
			//Constructor type
			sms_submit( const char* dest, const char* message )
				: m_dest_addr(dest), m_message( message )
			{}
			//Destructor
			virtual ~sms_submit() {

			}
			virtual int type() const {
				return sms::t_submit;
			}
			void validity_period( unsigned char val ) {
				m_validity_period = val;
			}
			int vailidity_period() const {
				return m_validity_period;
			}
			void flash_message( bool flash ) {
				m_flash_message = flash;
			}
			bool flash_message() const {
				return m_flash_message;
			}
			void report_request( bool report ) {
				m_status_report_request = report;
			}
			bool report_request() const {
				return m_status_report_request;
			}
			virtual int encode( at_parser& at, bool pdu ) const ;
			virtual int decode( at_parser& at, bool pdu );
		};

		// SMS-STATUS-REPORT TPDU
		class sms_status_report : public sms {
		private:
			char m_receipment_address[14] {};
			char m_service_tstamp[24] {};
			char m_discharge_time[24] {};
			unsigned char m_status {};
		public:
			virtual ~sms_status_report() {
			}
			virtual int type() const {
				return sms::t_status_report;
			}
		};

	// SMS-COMMAND PDU (see GSM 03.40 section 9.2.2.4)
	class sms_command : public sms 
	{
	private:
		unsigned char m_mref {};		//! Message reference
		unsigned char m_pid {};			//! Protocol identifier
		unsigned char m_cmd_type {};	//! Command type
		unsigned char m_dest_address[14] {}; //! Destination address
		unsigned char m_command_data_len {};
	public:
		virtual ~sms_command() {

		}
		virtual int type() const {
			return sms::t_status_report;
		}
	};
	namespace detail {
		constexpr size_t max( size_t a, size_t b ) {
			return a < b ? b : a;
		}
	}
	//! SMS message placement length 
	//! NOTE: C++11 doesn't have std::max as constexpr
	constexpr size_t sms_placement_size = 
		 detail::max( sizeof(sms_command),
			   detail::max( sizeof(sms_status_report),
			   detail::max( sizeof(sms_submit),
			   sizeof(sms_deliver) ) ) );
}

