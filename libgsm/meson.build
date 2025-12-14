_deps = [libfoundation, libisix]

libgsm =  declare_dependency(
	link_with : library('gsm',
		'src/at_parser.cpp',
		'src/gsm_device.cpp',
		'src/gsm_event.cpp',
		'src/gsm_phonebook.cpp',
		'src/param_parser.cpp',
		'src/sms_message.cpp',
		'src/sms_store.cpp',
		include_directories : 'include',
		dependencies : _deps,
	),
	include_directories : 'include',
	dependencies : _deps,
)