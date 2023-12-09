_include = include_directories('include')
_deps = get_option('disable_isix') ? _config : libisix
_flag = get_option('disable_isix') ? '-DCONFIG_ISIX_WITHOUT_KERNEL=1' : []

libfoundation = declare_dependency(
	link_with : library('foundation',
		'src/drv/storage/i2c_eeprom.cpp',
		'src/drv/storage/virtual_eeprom.cpp',
		'src/drv/storage/fs_env.cpp',
		'src/algo/md5sum.cpp',
		'src/algo/utils.c',
		'src/algo/util_ftoa.cpp',
		'src/algo/util_dtoa.cpp',
		'src/sys/tiny_alloc.c',
		'src/sys/tiny_printf.c',
		'src/dsp/spectrum_pwr.cpp',
		get_option('disable_isix') ? [] : [
			'src/drv/audio/i2s_audio.cpp',
			'src/newlib/memory.cpp',
			'src/newlib/cppruntime.cpp',
			'src/newlib/locking.c',
		],
		include_directories : _include,
		dependencies : _deps,
		c_args : _flag,
		cpp_args : _flag,
	),
	include_directories : _include,
	dependencies : _deps,
	compile_args : _flag,
)