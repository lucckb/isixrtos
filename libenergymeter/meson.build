libenergymeter = declare_dependency(
	link_with : library('energymeter',
		'src/energy_meter.cpp',
		'src/energy_phase_bufs.cpp',
		'src/energy_phase_n.cpp',
		cpp_args : '-Wno-psabi',
		include_directories : 'include',
		dependencies : libfoundation,
	),
	include_directories : 'include',
	dependencies : libfoundation,
)