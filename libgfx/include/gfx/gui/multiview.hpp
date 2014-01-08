/*
 * =====================================================================================
 *
 *       Filename:  multiedit.hpp
 *
 *    Description:  Multi Edit Box based only on last line message
 *    				Revert is impossible. It use only graphics memory
 *    				shihts for edit box. It is always RO block
 *    				It is simple console type widget
 *
 *        Version:  1.0
 *        Created:  05.01.2014 19:28:01
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */


#ifndef GFX_GUI_MULTIVIEW_HPP_
#define GFX_GUI_MULTIVIEW_HPP_

/* ------------------------------------------------------------------ */ 
#include <gfx/gui/widget.hpp>

/* ------------------------------------------------------------------ */ 
namespace gfx {
namespace gui {
/* ------------------------------------------------------------------ */ 
//! Multi edit class text
class multiview : public widget
{
	static constexpr coord_t text_margin = 2;
public:
	 /** Multi edit layout constructor
	  * @param[in] rect Rectangle layout
	  * @param[in] layout Input layout
	  * @param[in] win Window input
	  */
	 explicit multiview( rectangle const& rect,layout const& layout ,window &win)
	 	 : widget( rect, layout, win, false )
	 {
	 }
	 
	 //! Destructor
	 virtual ~multiview() {
	 }
	
	 /** Add one line to multiedit 
	  * @param[in] value String value 
	  */
	 template <typename T>
	 void append( const T& value ) {
		m_line += value;
		dirty();
	 }

	 /** Clear the window  */
	 void clear() {
		m_clear_req = true;
		dirty();
	 }
protected:
	 //! On repaint the widget return true when changed
	 virtual void repaint();
private:
	detail::string m_line;
	bool m_clear_req {};
};
/* ------------------------------------------------------------------ */ 
}}
/* ------------------------------------------------------------------ */ 
#endif   /* ----- #ifndef GFX_GUI_MULTIEDIT_HPP_  ----- */
