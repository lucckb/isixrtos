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
#include <cstddef>

namespace gsm_modem {


	namespace dcs_codes {
		const unsigned char DCS_COMPRESSED = 0x20; // bit 5
		const unsigned char DCS_DEFAULT_ALPHABET = 0 << 2; // bit 2..3 == 0
		const unsigned char DCS_EIGHT_BIT_ALPHABET = 1 << 2; // bit 2..3 == 01
		const unsigned char DCS_SIXTEEN_BIT_ALPHABET = 2 << 2; // bit 2..3 == 10
		const unsigned char DCS_RESERVED_ALPHABET = 3 << 2; // bit 2..3 == 11

		const unsigned char DCS_MESSAGE_WAITING_INDICATION = 0xc0; // bit 7..6 == 11
		const unsigned char DCS_VOICEMAIL_MESSAGE_WAITING = 0;
		const unsigned char DCS_FAX_MESSAGE_WAITING = 1;
		const unsigned char DCS_ELECTRONIC_MAIL_MESSAGE_WAITING = 2;
		const unsigned char DCS_OTHER_MESSAGE_WAITING = 3;
	};
	//! Base classes for all smses
	class sms 
	{
	public:
		enum type {
			t_deliver,
			t_submit,
			t_status_report
		};
		virtual int type() const = 0;
		virtual int encode( char *buf , size_t len ) = 0;
		virtual int decode( const char *buf, size_t len ) = 0;
	};

	// SMS-DELIVER TPDU
	class sms_deliver : public sms 
	{
	public:
		virtual int type() const {
			return sms::t_deliver;
		}
	};

	// SMS-SUBMIT TPDU
	class sms_submit : public sms 
	{
	public:
		virtual int type() const {
			return sms::t_submit;
		}
	};

    // SMS-STATUS-REPORT TPDU
	class sms_status_report : public sms {
	public:
		virtual int type() const {
			return sms::t_status_report;
		}
	};
};
