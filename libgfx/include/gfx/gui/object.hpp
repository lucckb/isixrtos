/*
 * object.hpp
 *
 *  Created on: 8 paź 2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#ifndef GFX_GUI_OBJECT_HPP_
#define GFX_GUI_OBJECT_HPP_
/* ------------------------------------------------------------------ */
#include <foundation/noncopyable.hpp>
#include <gfx/input/event_info.hpp>
#include <functional>
#include <memory>
#include <gfx/gui/detail/defines.hpp>
/* ------------------------------------------------------------------ */
namespace gfx {
namespace gui {
/* ------------------------------------------------------------------ */
class object;
/* ------------------------------------------------------------------ */
//Event for user space
struct event : public input::event_info
{
	using evtype = input::event_info::evtype;

	event( object *_sender, const input::event_info &event)
		: event_info( event), sender(_sender)
	{}
	event( object *_sender, evtype _type )
		: sender(_sender )
	{
		type = _type;
	}
	virtual ~event() {}
	object * const sender;
};
/* ------------------------------------------------------------------ */
//Basic event signal
using event_signal = std::function<bool(const event&)>;
using event_handle = std::weak_ptr<event_signal>;

/* ------------------------------------------------------------------ */
class object  : private fnd::noncopyable
{
public:
	event_handle connect( event_signal evt )
	{
		auto ret = std::make_shared<event_signal>( evt );
		m_events.push_back( ret );
		return ret;
	}
	void disconnect( event_handle evt )
	{
		m_events.remove( evt.lock() );
	}
protected:
	bool emit( const event& ev )
	{
		bool ret {};
		for( const auto handler : m_events )
		{
			ret |= (*handler)( ev );
		}
		return ret;
	}
private:
	//Signal event slot
	detail::container<std::shared_ptr<event_signal>> m_events;
};
/* ------------------------------------------------------------------ */
}
}
/* ------------------------------------------------------------------ */
#endif /* OBJECT_HPP_ */
/* ------------------------------------------------------------------ */
