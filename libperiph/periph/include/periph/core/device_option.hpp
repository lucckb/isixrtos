/*
 * =====================================================================================
 *
 *       Filename:  device_option.hpp
 *
 *    Description:  Device option class
 *
 *        Version:  1.0
 *        Created:  09.08.2018 19:30:46
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once

namespace periph::option {
	namespace ord {
		enum ord {
			speed,
			phase,
			polarity,
			dwidth,
		};
	}
	//Base device option
	struct device_option {
	public:
		explicit device_option(int option)
			: ord(option) {
		}
		const int ord;
	};

	//! Speed configuration
	struct speed : public device_option {
		explicit speed(unsigned _hz)
			: device_option(ord::speed), hz(_hz) {
			}
		const unsigned hz;
	};

	//! Phase configuration
	struct phase : public device_option {
		enum _phase : bool {
			_1_edge,
			_2_edge
		};
		phase(_phase _ph)
			: device_option(ord::phase), ph(_ph) {
		}
		const _phase ph;
	};

	//! Phase configuration
	struct polarity : public device_option {
		enum class _polarity : bool {
			low,
			high
		};
		polarity(_polarity _pol)
			: device_option(ord::polarity), pol(_pol) {
		}
		const _polarity pol;
	};

	//! Data with option
	struct dwidth: public device_option {
	public:
		using dwidth_t = unsigned char;
		explicit dwidth(dwidth_t _dwidth)
			: device_option(ord::dwidth) , dw(_dwidth)
		{
		}
		dwidth_t dw;
	};
}

