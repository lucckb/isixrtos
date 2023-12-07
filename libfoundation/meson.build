libfoundation_inc = include_directories('include')

libfoundation = library('foundation',
	include_directories : [ libfoundation_inc, config_inc, libisix_inc ],
	sources : [
		'src/drv/storage/i2c_eeprom.cpp',
		'src/drv/storage/virtual_eeprom.cpp',
		'src/drv/storage/fs_env.cpp',
		'src/drv/audio/i2s_audio.cpp',
		'src/algo/md5sum.cpp',
		'src/algo/utils.c',
		'src/algo/util_ftoa.cpp',
		'src/algo/util_dtoa.cpp',
		'src/newlib/memory.cpp',
		'src/newlib/cppruntime.cpp',
		'src/newlib/locking.c',
		'src/sys/tiny_alloc.c',
		'src/sys/tiny_printf.c',
		'src/dsp/spectrum_pwr.cpp',
	],
)