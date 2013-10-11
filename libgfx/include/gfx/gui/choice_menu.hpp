/*
 * choice_menu.hpp
 *
 *  Created on: 11 paź 2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */

#ifndef GFX_GUI_CHOICE_MENU_HPP_
#define GFX_GUI_CHOICE_MENU_HPP_
/* ------------------------------------------------------------------ */
#include <gfx/gui/widget.hpp>

namespace gfx {
namespace gui {

/* ------------------------------------------------------------------ */
//Request choice GUI menu
class choice_menu : public widget
{
public:
	enum class style : char
	{
		normal,		//Normal choice mode
		select		//Active selection mode
	};
	//Item used for the array
	using item = std::pair<int, const char*>;
	static constexpr item end { 0, "" };
	//Constructor
	explicit choice_menu( rectangle const& rect,layout const& layout,
			window &win, style style = style::normal );
	//Destructor
	virtual ~choice_menu() {}
	//Set menu items
	void items( const item *items=nullptr) { m_items = items; }
	//Repaint virtual function
	virtual void repaint();
private:
	//Widget style
	style m_style { style::normal };
	//Menu items
	const item *m_items;
	//Current selected item
	int m_sel_item {};
	//Max number of items
	const int m_max_items {};
};

/* ------------------------------------------------------------------ */
} /* namespace gui */
} /* namespace gfx */

/* ------------------------------------------------------------------ */
#endif /* CHOICE_MENU_HPP_ */
