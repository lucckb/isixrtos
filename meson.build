project('isix', 'c', 'cpp',
	version : run_command(
		find_program('git'), 'describe', '--tags', '--dirty', '--long', '--always', check : true).stdout().strip(),
	meson_version: '>=1.2',
	default_options : [
		'default_library=static',
		'b_sanitize=' + (meson.is_cross_build() ? 'none' : 'address,undefined'),
		'b_pch=false',
		'b_staticpic=false',
		'b_pie=false',
		'c_std=gnu17',
		'cpp_std=gnu++23',
		'cpp_rtti=' + (meson.is_cross_build() ? 'false' : 'true'),
		'cpp_eh=' + (meson.is_cross_build() ? 'none' : 'default'),
		'werror=true',
		'warning_level=2',
		'b_lundef=false',
	]
)

add_project_arguments(
	'-DISIX_VERSION="'+meson.version()+'"',
	'-Wno-variadic-macros', '-Wno-long-long',
	'-Wa,-mapcs-32',
	get_option('debug')
		? '-DPDEBUG'
		: ['-fomit-frame-pointer', '-ffunction-sections', '-fdata-sections', '-flto'],
	get_option('disable_isix') ? '-DCONFIG_ISIX_WITHOUT_KERNEL=1' : [],
	language : ['c', 'cpp']
)
add_project_arguments('-Wno-volatile', language : 'cpp')

if get_option('enable_newlib_nano') and get_option('cpp_eh') != 'none'
	error('Unable to use nano version of newlib with exceptions enabled')
endif
add_project_link_arguments(
	'-nostdlib', '-nostartfiles', '--specs=nosys.specs',
	not get_option('debug') ? ['-Wl,--gc-sections', '-flto'] : [],
	get_option('enable_newlib_nano') and get_option('cpp_eh') == 'none' ? '--specs=nano.specs' : [],
	language : ['c', 'cpp']
)

config_inc = include_directories('config/include')
if get_option('disable_isix')
	subdir('libfoundation')
else
	subdir('libisix')
	subdir('libfoundation')
	subdir('libperiph')
	subdir('libgfx')
endif
subdir('libenergymeter')
subdir('libgsm')
subdir('libtcpip')
subdir('libtls')