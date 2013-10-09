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
	event( const object *_sender, const input::event_info &event)
		: event_info( event), sender(_sender)
	{}
	virtual ~event() {}
	const object *sender;
};
/* ------------------------------------------------------------------ */
//Basic event signal
using event_signal = std::function<void(const event&)>;
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
	void emit( const event& ev )
	{
		for( const auto handler : m_events )
		{
			(*handler)( ev );
		}
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
