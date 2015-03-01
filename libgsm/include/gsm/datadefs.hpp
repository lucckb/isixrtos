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

namespace gsm_modem {


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
	//! Current operator status
	struct oper_info {
		char desc_short[11] {};
		char desc_long[17] {};
		op_modes mode {};
		op_status status {};
		unsigned numeric_name{};	//Network numeric code
	};

	//! Registration status code
	struct reg_status {
		enum reg_status_ {
			not_registered_not_active = 0,
			registered_home = 1,
			not_registered_searching = 2,
			unknown = 3,
			registered_roaming = 4
		};
	};

}
