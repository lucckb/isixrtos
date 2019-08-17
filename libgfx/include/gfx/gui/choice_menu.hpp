/*
 * choice_menu.hpp
 *
 *  Created on: 11 paź 2013
 *      Author: lucck
 */
 
#pragma once
 
#include <gfx/gui/widget.hpp>

namespace gfx {
namespace gui {

 
/** Request choice class. This class implements
 *  two choice style types normal and select.
 *  Normal choice is menu like select item 
 *  Select is chocie with circle round
 */
class choice_menu : public widget
{
public:
	//! Menu style
	enum class style : char {
		normal,		//! Normal one item select menu
		select		//! Select item menu
	};
	//! Item used for the array
	using item = std::pair<int, const char*>;

	//! End item indicator
	static constexpr item end { 0, "" };
	/** Choice menu constructor
	 * @param[in] rect Window position relative size
	 * @param[in] layout Window layout colors
	 * @param[in] style Widget style @see style
	 */
	explicit choice_menu( rectangle const& rect,layout const& layout,
			window &win, style style = style::normal );

	//Destructor
	virtual ~choice_menu() {}

	//Set menu items
	void items( const item *items = nullptr );

	// Report input event
	virtual void report_event( const input::event_info& ev );

	/** Get current selection
	 * @return Selected item
	 */
	int selection() const {
		return m_style==style::normal?m_curr_item:m_sel_item;
	}

	int selection_item() const {
		return m_items[selection()].first;
	}

	/** Set selection if selected something is changed */
	void selection(int sel) {
		if( m_style==style::normal ) m_curr_item = sel;
		else m_sel_item = sel;
	}

	void selection_item(int sel) {
		for (int n = 0; n < m_num_items; n++)
		{
			if (m_items[n].first == sel)
			{
				selection(n);
				break;
			}
		}
	}
protected:
	//! Repaint virtual function
	virtual void repaint( bool focus );
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

 
} /* namespace gui */
} /* namespace gfx */

 
