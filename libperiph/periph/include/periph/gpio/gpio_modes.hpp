#pragma once

namespace periph {
namespace gpio {

	enum class pulltype {
		floating,
		down,
		up
	};

	enum class outtype {
		pushpull,		//! Push pullmode
		opendrain_pu,	//! Open drain pullup
		opendrain_pd,	//! Open drain pulldown
	};
	enum class speed {
		low,
		medium,
		high
	};
	namespace mode {
		struct in {
			pulltype pu;
		};
		struct an {
			pulltype pu;
		};
		struct out {
			outtype  out;
			speed spd;
		};
		struct alt {
			outtype out;
			int altno;	//Alternate function identifier
			speed spd; // Port speed
		};
	}

}}
