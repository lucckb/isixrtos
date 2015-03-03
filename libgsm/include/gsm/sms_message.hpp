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

namespace gsm_modem {

		//http://www.smssolutions.net/tutorials/gsm/sendsmsat/

		//! Base classes for all smses
		class sms 
		{
			data_coding_scheme m_dcs;
		public:
			enum type {
				t_deliver,		//! Deliver message type
				t_submit,		//! Submit message type
				t_status_report, //! Status report message type
				t_command,		 //! Command type SMS
			};
			virtual int type() const = 0;
			virtual int encode( char *buf , size_t len ) = 0;
			virtual int decode( const char *buf, size_t len ) = 0;
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
			char m_message[162] {};		//! Sms message data
		public:
			virtual int type() const {
				return sms::t_deliver;
			}
			const char* service_tstamp() const {
				return m_service_tstamp;
			}
			bool get_report_indication() const {
				return m_status_report_indication;
			}
			const char* origin_address() const {
				return m_service_tstamp;
			}
			const char* message() const {
				return m_message;
			}
			int pid() const {
				return m_pid;
			}
		};

		// SMS-SUBMIT TPDU
		class sms_submit : public sms 
		{
		private:
			unsigned char m_validity_period { 167 };
			bool m_status_report_request {};
			bool m_flash_message {};
			char m_dest_address[14] {};
			char m_message[162] {};
		public:
			virtual int type() const {
				return sms::t_submit;
			}
			void validity_period( unsigned char val ) {
				m_validity_period = val;
			}
			void flash_message( bool flash ) {
				m_flash_message = flash;
			}
			void dest_address( const char* address ) {
				std::strncpy( m_dest_address, address, sizeof m_dest_address - 1 );
			}
			void message( const char* message ) {
				std::strncpy( m_message, message, sizeof m_message - 1 );
			}
		};

		// SMS-STATUS-REPORT TPDU
		class sms_status_report : public sms {
		private:
			char m_receipment_address[14] {};
			char m_service_tstamp[24] {};
			char m_discharge_time[24] {};
			unsigned char m_status {};
		public:
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
		virtual int type() const {
			return sms::t_status_report;
		}
	};
}

