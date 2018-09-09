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
			bitorder,
		};
	}
	//Base device option
	class device_option {
	public:
		explicit device_option(int option)
			: _ord(option) {
		}
		auto ord() const {
			return _ord;
		}
	private:
		const int _ord;
	};

	//! Speed configuration
	class speed : public device_option {
	public:
		explicit speed(unsigned _hz)
			: device_option(ord::speed), hz_(_hz) {
			}
		auto hz() const {
			return hz_;
		}
	private:
		const unsigned hz_;
	};

	//Byte order
	class bitorder : public device_option {
	public:
		enum order : bool { lsb, msb };
		explicit bitorder( order _ord)
			: device_option(ord::bitorder), ord_(_ord) {
			}
		auto order() const {
			return ord_;
		}
	private:
		const enum order ord_;
	};

	//! Phase configuration
	class phase : public device_option {
	public:
		enum _phase : bool {
			_1_edge,
			_2_edge
		};
		phase(_phase ph_)
			: device_option(ord::phase), _ph(ph_) {
		}
		auto ph() const {
			return _ph;
		}
	private:
		const _phase _ph;
	};

	//! Phase configuration
	class polarity : public device_option {
	public:
		enum _polarity : bool {
			low,
			high
		};
		polarity(_polarity _pol)
			: device_option(ord::polarity), pol_(_pol) {
		}
		auto pol() const {
			return pol_;
		}
	private:
		const _polarity pol_;
	};

	//! Data with option
	class dwidth: public device_option {
	public:
		using dwidth_t = unsigned char;
		explicit dwidth(dwidth_t _dwidth)
			: device_option(ord::dwidth) , _dw(_dwidth)
		{
		}
		auto dw() const {
			return _dw;
		}
	private:
		dwidth_t _dw;
	};
}

