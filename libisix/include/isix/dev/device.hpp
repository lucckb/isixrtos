/*
 * device.hpp
 *
 *  Created on: 26-06-2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#ifndef ISIX_DEV_DEVICE_HPP_
#define ISIX_DEV_DEVICE_HPP_

/* ------------------------------------------------------------------ */
#include <isix/error.h>
#include <isix/types.h>
#include <cstddef>
/* ------------------------------------------------------------------ */
namespace isix {
namespace dev {
/* ------------------------------------------------------------------ */
/* Abstract device class */
class device {
private:
#if __cplusplus > 199711L
	device( const device& ) = delete;
	const device& operator=( const device& ) = delete;
#else
	device( const device& );
	const device& operator=( const device& );
#endif
public:
	enum class_id
	{
		cid_input_usb_kbd
	};
	device( class_id dclass )
		: m_class( dclass )
	{}
	virtual ~device() {}
	class_id get_class() const
	{
		return class_id(m_class);
	}
	virtual int open( int /*flags*/ )
	{
		return ISIX_ENOTSUP;
	}
	virtual int close()
	{
		return ISIX_ENOTSUP;
	}
	virtual int read( void* /*buf*/, std::size_t /*len*/, int /*timeout*/ )
	{
		return ISIX_ENOTSUP;
	}
	virtual int write( const void* /*buf*/, std::size_t /*len*/, int /*timeout*/ )
	{
		return ISIX_ENOTSUP;
	}
private:
	const short m_class;
};
/* ------------------------------------------------------------------ */

}}
/* ------------------------------------------------------------------ */
#endif /* DEVICE_HPP_ */
