/*
 * =====================================================================================
 *
 *       Filename:  sms_codec.hpp
 *
 *    Description:  SMS codec implementation
 *
 *        Version:  1.0
 *        Created:  03.03.2015 17:30:16
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck
 *   Organization:  
 *
 * =====================================================================================
 */
#pragma once

namespace gsm_modem {


	namespace dcsc {
		constexpr  unsigned char COMPRESSED = 0x20; // bit 5
		constexpr  unsigned char DEFAULT_ALPHABET = 0 << 2; // bit 2..3 == 0
		constexpr  unsigned char EIGHT_BIT_ALPHABET = 1 << 2; // bit 2..3 == 01
		constexpr  unsigned char SIXTEEN_BIT_ALPHABET = 2 << 2; // bit 2..3 == 10
		constexpr  unsigned char RESERVED_ALPHABET = 3 << 2; // bit 2..3 == 11

		constexpr  unsigned char MESSAGE_WAITING_INDICATION = 0xc0; // bit 7..6 == 11
		constexpr  unsigned char VOICEMAIL_MESSAGE_WAITING = 0;
		constexpr  unsigned char FAX_MESSAGE_WAITING = 1;
		constexpr  unsigned char ELECTRONIC_MAIL_MESSAGE_WAITING = 2;
		constexpr  unsigned char OTHER_MESSAGE_WAITING = 3;
		constexpr  unsigned char FLASH_MESSAGE = 0x10;
	};
	namespace foctet {
		const unsigned char REPORT_REQUEST = 1<<5;
	}

	class data_coding_scheme
	{
		private:
			unsigned char _dcs;

		public:
			// initialize with data coding scheme octet
			data_coding_scheme(unsigned char dcs) : _dcs(dcs) {}

			// set to default values (no message waiting, no message class indication,
			// default 7-bit alphabet)
			data_coding_scheme() : _dcs(dcsc::DEFAULT_ALPHABET) {}

			// return type of alphabet used (if messageWaitingIndication == false)
			unsigned char getAlphabet() const
			{
				return _dcs & (3 << 2);
			}
			// return true if message compressed
			// (if messageWaitingIndication == false)
			bool compressed() const
			{
				return (_dcs & dcsc::COMPRESSED) == dcsc::COMPRESSED;
			}

			// return true if message waiting indication
			bool messageWaitingIndication() const
			{
				return (_dcs & dcsc::MESSAGE_WAITING_INDICATION) ==
					dcsc::MESSAGE_WAITING_INDICATION;
			}

			// return type of waiting message (if messageWaitingIndication == true)
			unsigned char getMessageWaitingType() const {return _dcs & 3;}

			operator unsigned char() const {return _dcs;}
	};
	
}
