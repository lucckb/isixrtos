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
	void items( const item *items=nullptr);
	//Repaint virtual function
	virtual void repaint();
	//* Report input event
	virtual bool report_event( const input::event_info& ev );
	//Get selection
	int selection() const
	{
		return m_style==style::normal?m_curr_item:m_sel_item;
	}
	void selection(int sel)
	{
		if( m_style==style::normal ) m_curr_item = sel;
		else m_sel_item = sel;
	}
private:
	//Calc maximum item value
	int calc_max_items() const;
private:
	//Widget style
	style m_style { style::normal };
	//Menu items
	const item *m_items;
	//Current selected item
	int m_curr_item {};
	//Selected item in choice mode
	int m_sel_item {};
	//Max number of items
	const int m_max_box_items;
	//Num items
	int m_num_items {};
	//Margin for y
	static constexpr coord_t y_margin = 2;
	static constexpr coord_t x_margin_left = 4;
	static constexpr coord_t x_margin_right = 9;
	static constexpr coord_t slider_space = 2;
};

/* ------------------------------------------------------------------ */
} /* namespace gui */
} /* namespace gfx */

/* ------------------------------------------------------------------ */
#endif /* CHOICE_MENU_HPP_ */
