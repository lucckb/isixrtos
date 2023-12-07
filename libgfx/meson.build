libgfx_inc = include_directories('include')

libgfx = library('gfx',
	include_directories : [ libgfx_inc, config_inc, libisix_inc, libfoundation_inc, libperiph_inc ],
	sources : [
		'drivers/input/ft6x06_touch.cpp',
		'drivers/disp/dsi_fb.cpp',
		'src/gdi.cpp',
		'src/input.cpp',
		'src/gui/choice_menu.cpp',
		'src/gui/frame.cpp',
		'src/gui/button.cpp',
		'src/gui/window.cpp',
		'src/gui/waterfall.cpp',
		'src/gui/seekbar.cpp',
		'src/gui/timer.cpp',
		'src/gui/multiview.cpp',
		'src/gui/label.cpp',
		'src/gui/selectable_widget.cpp',
		'src/gui/battery_icon.cpp',
		'src/gui/editbox.cpp',
		'src/gui/icon.cpp',
		'src/fonts/small.cpp',
		'src/fonts/default.cpp',
	],
	cpp_args : '-DCONFIG_ISIX_USE_TIMERS=1'
)