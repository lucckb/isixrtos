libenergymeter_inc = include_directories('include')

libenergymeter = library('energymeter',
	include_directories : [ libenergymeter_inc, libfoundation_inc ],
	sources : [
		'src/energy_meter.cpp',
		'src/energy_phase_bufs.cpp',
		'src/energy_phase_n.cpp',
	],
	cpp_args : '-Wno-psabi'
)