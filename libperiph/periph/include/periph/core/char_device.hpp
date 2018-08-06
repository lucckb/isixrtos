/*
 * =====================================================================================
 *
 *       Filename:  char_device.hpp
 *
 *    Description:  Character device base specs
 *
 *        Version:  1.0
 *        Created:  29.07.2018 21:36:40
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#include <periph/device.hpp>

namespace periph {
	class char_device : public device {
	public:
		enum class seek {
			set, cur, end
		};
		enum class fio : bool {
			read, write
		};
		//! Constructor
		char_device();
		//! Destructor
		virtual ~char_device() {
		}
		//! Read
		virtual int read(pointer data, size len) = 0;
		//! Write
		virtual int write(cpointer data, size len) = 0;
		//! seek
		virtual int llseek(seek pos, long offs) = 0;
		//! How much can read or write from device without block
		virtual int fnio(fio fio) = 0;
	};
}
