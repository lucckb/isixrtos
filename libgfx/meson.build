# Allow CONFIG_GFX_PIXEL_FORMAT to be passed from parent project
# This is needed when libgfx is used as subproject and parent project
# defines CONFIG_GFX_PIXEL_FORMAT in its isix_config.h
_gfx_pixel_format = meson.get_external_property('gfx_pixel_format', '')
_gfx_cpp_args = []
if _gfx_pixel_format != ''
	_gfx_cpp_args += ['-DCONFIG_GFX_PIXEL_FORMAT=' + _gfx_pixel_format]
endif

libgfx = declare_dependency(
	link_with : library('gfx',
		'drivers/input/ft6x06_touch.cpp',
		'drivers/disp/dsi_fb.cpp',
		'drivers/disp/ili9341.cpp',
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
		include_directories : 'include',
		dependencies : libperiph,
		cpp_args : _gfx_cpp_args,
	),
	include_directories : 'include',
	dependencies : libperiph,
)
