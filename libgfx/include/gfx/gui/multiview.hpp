/*
 * =====================================================================================
 *
 *       Filename:  multiedit.hpp
 *
 *    Description:  Multi Edit Box based only on last line message
 *    				Revert is impossible. It use only graphics memory
 *    				shihts for edit box. It is always RO block
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
public:
	 /** Multi edit layout constructor
	  * @param[in] rect Rectangle layout
	  * @param[in] layout Input layout
	  * @param[in] win Window input
	  */
	 explicit multiview( rectangle const& rect,layout const& layout ,window &win)
	 	 : widget( rect, layout, win )
	 {
	 }
	 
	 //! Destructor
	 virtual ~multiview() 
	 {
	 }
	
	 /** Report key or mouse event widget 
	  * @param[in] ev Input event type 
	  */
	 virtual void report_event( const input::event_info& ev );

	 /** Add one line to multiedit 
	  * @param[in] value String value 
	  */
	 void append_value( const detail::string& value )
	 {
		m_last_line = value;
	 }

	 /** Clear the window  */
	 void clear() {
		m_clear_req = true;
	 }
protected:
	 //! On repaint the widget return true when changed
	 virtual void repaint();
private:
	detail::string m_last_line;
	bool m_clear_req {};
};
/* ------------------------------------------------------------------ */ 
}}
/* ------------------------------------------------------------------ */ 
#endif   /* ----- #ifndef GFX_GUI_MULTIEDIT_HPP_  ----- */
