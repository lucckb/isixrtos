/*
 * =====================================================================================
 *
 *       Filename:  datadefs.hpp
 *
 *    Description:  Datadefinitions for the GSM library functions
 *
 *        Version:  1.0
 *        Created:  01.03.2015 19:32:48
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
#include <cctype>
#include <algorithm>

namespace gsm_modem {

	struct number_format {
		enum number_format_ {
			unknown = 129,
			international = 145
		};
	};

	// Sim requirement codes
	struct sim_req {
	enum sim_req_ {
		ready=1,	//Sim slot is ready
		pin,		//Waiting for pin1
		puk,		//Waiting for puk
		pin2,		//Waiting for pin2
		puk2		//Waiting for puk2
	};
	};

	//! Operation mode status
	enum class op_modes : short {
		automatic = 0,
		manual = 1,
		deregister = 2,
		manualauto = 4
	};
	//! Operation status
	enum class op_status : short {
		unknown = 0,
		available = 1,
		current = 2,
		forbidden = 3
	};

	//! Set registration notification
	enum class reg_notify : short {
		disabled = 0,
		registration = 1,
		location = 2
	};

	//! Current operator status
	struct oper_info {
		char desc_short[11] {};
		char desc_long[17] {};
		op_modes mode {};
		op_status status {};
		unsigned numeric_name{};	//Network numeric code
	};

	//! Get IMEI
	struct imsi_number {
		char value[16] {};
	};

	//! IMEI
	struct imei_number {
		char value[17] {};
	};

	//! Registration status code
	enum class reg_status {
		not_registered_not_active = 0,
		registered_home = 1,
		not_registered_searching = 2,
		denied = 3,
		unknown = 4,
		registered_roaming = 5
    };
	struct sms_text_params {
		unsigned char first_octet;
		unsigned char validity_period;
		unsigned char protocol_identifier;
		unsigned char data_coding_scheme;
		bool operator!=( const sms_text_params& that ) const {
			return std::memcmp( this, &that, sizeof(sms_text_params) );
		}
	};
	//! NOTE: non shared version is not thread safe!
	class sms;
	using sms_type_ptr_t = sms*;
	namespace	//Unnamed namespace
	{
		/** Check if the number is international format
		 * @param[in] phoneno Phone number
		 * @return true if number has in international format*/
		inline bool is_international_number(const char phoneno[] )
		{
			const auto len = std::strlen(phoneno);
			return len>1 && phoneno[0]=='+' && std::all_of(phoneno+1,phoneno+len,::isdigit);
		}
	}
}
