libgsm_inc = include_directories('include')

libgsm = library('gsm',
	include_directories : [ libgsm_inc, config_inc, libisix_inc, libfoundation_inc ],
	sources : [
		'src/at_parser.cpp',
		'src/gsm_device.cpp',
		'src/gsm_event.cpp',
		'src/gsm_phonebook.cpp',
		'src/param_parser.cpp',
		'src/sms_message.cpp',
		'src/sms_store.cpp',
	],
)