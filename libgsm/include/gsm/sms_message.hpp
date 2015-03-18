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

		class param_parser;
		//! Base classes for all smses
		class sms 
		{
			data_coding_scheme m_dcs;
		public:
			virtual ~sms() {
			}
			enum mtype {
				t_deliver,		//! Deliver message type
				t_submit,		//! Submit message type
				t_status_report, //! Status report message type
				t_command,		 //! Command type SMS
			};
			virtual int type() const = 0;
			//Setter getters
			data_coding_scheme get_dcs( ) const 
			{
				return m_dcs;
			}
			void set_dcs( data_coding_scheme dcs ) 
			{
				m_dcs = dcs;
			}
		protected:
			bool addr_valid(const char* addr) const 
			{
				return std::strlen(addr)==12 &&
					addr[0]=='+';
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
			/** Constructor for SMS submit message in txt
			 * @param[in] buf Input buffer data or pdu data
			 * @param[in] input buffer length
			 * @param[in] msg Only in text mode message for PDU should be null
			 */
			sms_deliver( param_parser& p, const char* msg = nullptr );

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
			char* get_data() {
				return m_message;
			}
			size_t data_size() const {
				return sizeof(m_message);
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
			char m_dest_addr[14] {};
			char m_message[162] {};
			unsigned char m_pid {};
		public:
			//Constructor type
			sms_submit( const char* dest=nullptr, const char* message=nullptr)
			{
				if(dest) {
					std::strncpy(m_dest_addr, dest, sizeof(m_dest_addr)-1);
				}
				if(message) {
					std::strncpy(m_message, message, sizeof(m_message)-1);
				}
			}
			/** Constructor for SMS submit message in txt
			 * @param[in] buf Input buffer data or pdu data
			 * @param[in] input buffer length
			 * @param[in] msg Only in text mode message for PDU should be null
			 */
			sms_submit( param_parser &p, const char* msg );
			//Destructor
			virtual ~sms_submit() {

			}
			bool valid() const {
				return addr_valid(m_dest_addr);
			}
			const char* dest_address() const {
				return m_dest_addr;
			}
			void dest_address( const char *addr ) {
				std::strncpy( m_dest_addr, addr, sizeof(m_dest_addr)-1 );
			}
			virtual int type() const {
				return sms::t_submit;
			}
			void validity_period( unsigned char val ) {
				m_validity_period = val;
			}
			int validity_period() const {
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
			int pid() const {
				return m_pid;
			}
			void pid( unsigned char pid ) {
				m_pid = pid;
			}
			const char* message() const {
				return m_message;
			}
			void message( const char* msg ) {
				std::strncpy( m_message, msg, sizeof(m_message)-1 );
			}
			char* get_data() {
				return m_message;
			}
			size_t data_size() const {
				return sizeof(m_message);
			}
		};

		// SMS-STATUS-REPORT TPDU
		class sms_status_report : public sms {
		private:
			char m_receipment_address[14] {};
			char m_service_tstamp[24] {};
			char m_discharge_time[24] {};
			unsigned char m_status {};
			unsigned char m_msg_ref {};	//Message reference
		public:
			/** Constructor for SMS status report message in txt
			 * @param[in] buf Input buffer data or pdu data
			 * @param[in] msg Only in text mode message for PDU should be null
			 */
			sms_status_report( param_parser &p );
			
			//! Destructor
			virtual ~sms_status_report() {
			}
			//! Get sms message type
			virtual int type() const {
				return sms::t_status_report;
			}
			//! Get receimpent address
			const char* receimpent_address() const {
				return m_receipment_address;
			}
			//! SCS timestamp
			const char* scs_timestamp() const {
				return m_service_tstamp;
			}
			const char* discharge_time() const {
				return m_discharge_time;
			}
			int status() const {
				return m_status;
			}
			int msg_ref() const {
				return m_msg_ref;
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

