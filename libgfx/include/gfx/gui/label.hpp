/*
 * label.hpp
 *
 *  Created on: 10 paź 2013
 *      Author: lucck
 */
 
#pragma once
 
#include <gfx/gui/widget.hpp>
 
namespace gfx {
namespace gui {
 
class label: public widget {
public:
	struct flags {
		enum : unsigned {
			center = 0x01,		//Center text
			select = 0x02,		// Can select
		};
	};
	//Destructor
	virtual ~label(){ }
	//Constructor
	label( rectangle const& rect,layout const& layout,
			window &win, unsigned flags = 0 );
	//Label no reports any events
	template< typename T>
	void caption( const T caption ) {
		m_caption = caption;
		modified();
	}
	const detail::string& caption() const {
		return m_caption;
	}
	void sel_color( bool use_sel ) {
		m_sel_color = use_sel;
	}
protected:
	//Repaint the label
	virtual void repaint( bool focus );
private:
	static constexpr auto c_default = -1;
	detail::string m_caption;
	coord_t text_px_old = 0;
	bool m_sel_color {};
	unsigned m_flags {};
};
 
} /* namespace gui */
} /* namespace gfx */
 
