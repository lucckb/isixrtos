disable_isix = get_option('disable_isix')

fnd_includes = include_directories('include')
fnd_deps = disable_isix ? _config : libisix
fnd_flag = disable_isix ? '-DCONFIG_ISIX_WITHOUT_KERNEL=1' : []


fnd_srcs = files( 
    'src/drv/storage/i2c_eeprom.cpp',
    'src/drv/storage/virtual_eeprom.cpp',
    'src/drv/storage/fs_env.cpp',
    'src/algo/md5sum.cpp',
    'src/algo/utils.c',
    'src/algo/util_ftoa.cpp',
    'src/algo/util_dtoa.cpp',
    'src/sys/tiny_alloc.c',
    'src/sys/tiny_printf.c',
    'src/dsp/spectrum_pwr.cpp'
)

fnd_src_exp = files()

if not disable_isix
  fnd_srcs += files(
    'src/drv/audio/i2s_audio.cpp',
  )
  fnd_src_exp = files(
    'src/newlib/memory.cpp',
    'src/newlib/cppruntime.cpp',
    'src/newlib/locking.c',
  )
endif

fnd_syscalls = declare_dependency(
  link_whole: library('foundation_syscalls',
      fnd_src_exp,
      include_directories: fnd_includes,
      dependencies: fnd_deps,
  )
)


libfoundation = declare_dependency(
	link_with : library('foundation',
        fnd_srcs,
		include_directories : fnd_includes,
		dependencies : [fnd_deps, fnd_syscalls],
		c_args : fnd_flag,
		cpp_args : fnd_flag,
	),
	include_directories : fnd_includes,
	dependencies : [fnd_deps, fnd_syscalls],
	compile_args : fnd_flag,
    #link_args: '-Wl,--undefined=malloc',
)

