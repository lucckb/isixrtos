 
/*
 * object.hpp
 *
 *  Created on: 8 paź 2013
 *      Author: lucck
 */
 
#pragma once
 
#include <foundation/algo/noncopyable.hpp>
#include <gfx/input/event_info.hpp>
#include <functional>
#include <memory>
#include <gfx/gui/detail/defines.hpp>
 
namespace gfx {
namespace gui {
 
class object;
 
//Event for user space
struct event : public input::event_info
{
	using evtype = input::event_info::evtype;

	event( object *_sender, const input::event_info &event)
		: event_info( event), sender(_sender)
	{
		window = nullptr;
	}
	event( object *_sender, evtype _type )
		: sender( _sender )
	{
		type = _type;
		window = nullptr;
	}
	bool match( object& o ) const {
		return sender == &o;
	}
	virtual ~event() {}
	object * const sender;
};
 
//Basic event signal
using event_signal = std::function<bool(const event&)>;
using event_handle = std::weak_ptr<std::pair<event::evtype,event_signal>>;

 
class object
{
public:
	object() {
	}
	virtual ~object() {
	}
	object& operator=(object&) = delete;
	object(object&) = delete;
	event_handle connect( event_signal evt_h, event::evtype evt_t  )
	{
		auto ret = std::make_shared<std::pair<event::evtype,event_signal>>( std::make_pair(evt_t, evt_h) );
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
			if( handler->first == ev.type )
				ret |= (handler->second)( ev );
		}
		return ret;
	}
private:
	//Signal event slot
	detail::container<std::shared_ptr<std::pair<event::evtype,event_signal>>> m_events;
};
 
}
}
 
