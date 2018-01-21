#pragma once


namespace periph {
namespace dt {

	enum class bus {
		axi,
		ahb,
		apb1,
		apb2,
		cpu
	};

	enum pinid {
		sck,
		miso,
		mosi,
		rxd,
	};

	struct clk {
		bus busid;
		unsigned speed;
	};

	struct pin {
		pinid id;
		unsigned gpio;
	};

	struct dev {
		uintptr_t addr;
		clk* clock;
		pin* pins;
		unsigned mux;
	};


	struct devconf {
		clk* clk;
		devs* devs;
	};


}}


//int devconf_dev_get_pin( void *addr, enum pinid );
//int devconf_dev_get_clock( void* addr );
//int devconf_bus_get_clock( bus );


