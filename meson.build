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
		'b_asneeded=false',
		'b_lto=true',
	]
)

add_project_arguments(
	'-Wno-variadic-macros', '-Wno-long-long', '-pipe',
	'-Wa,-mapcs-32',
	get_option('debug')
		? '-DPDEBUG'
		: ['-fomit-frame-pointer', '-ffunction-sections', '-fdata-sections'],
	get_option('shutdown_api') ? '-DCONFIG_ISIX_SHUTDOWN_API' : [],
	get_option('task_stack_check') ? '-DCONFIG_ISIX_TASK_STACK_CHECK' : [],
	get_option('stack_ascending') ? '-DCONFIG_ISIX_STACK_ASCENDING' : [],
	get_option('fifo_event_notify') ? '-DCONFIG_ISIX_FIFO_EVENT_NOTIFY' : [],
	get_option('cpu_usage_api') ? '-DCONFIG_ISIX_CPU_USAGE_API' : [],
	get_option('mem_protect') != 'none'
		? '-DCONFIG_ISIX_MEMORY_PROTECTION_MODEL=ISIX_MPROT_'+get_option('mem_protect').to_upper() : [],

	'-DCONFIG_ISIX_HZ=' + get_option('tick_hz').to_string(),
	'-DCONFIG_ISIX_NUMBER_OF_PRIORITIES='+get_option('prio_count').to_string(),
	'-DCONFIG_ISIX_NVIC_PRIO_BITS='+get_option('nvic_prio_bits').to_string(),
	'-DISIX_MAX_SYSCALL_INTERRUPT_PRIORITY='+get_option('max_syscall_interrupt_prio').to_string(),
	'-DCONFIG_ISIX_MAX_CXA_ATEXIT_OBJECTS='+get_option('max_cxa_atexit_objects').to_string(),
	'-DCONFIG_ISIX_TIMERS_CMD_QUEUE_SIZE='+get_option('timers_cmd_queue_size').to_string(),

	get_option('log_level') != 'off'
		? '-DCONFIG_ISIX_LOGLEVEL=ISIXLOG_'+get_option('log_level').to_upper() : [],
	get_option('log_level_scheduler') != 'inherit'
		? '-DCONFIG_ISIX_LOGLEVEL_SCHEDULER=ISIXLOG_'+get_option('log_level_scheduler').to_upper() : [],
	get_option('log_level_fifo') != 'inherit'
		? '-DCONFIG_ISIX_LOGLEVEL_FIFO=ISIXLOG_'+get_option('log_level_fifo').to_upper() : [],
	get_option('log_level_task') != 'inherit'
		? '-DCONFIG_ISIX_LOGLEVEL_TASK=ISIXLOG_'+get_option('log_level_task').to_upper() : [],
	get_option('log_level_vtimers') != 'inherit'
		? '-DCONFIG_ISIX_LOGLEVEL_VTIMERS=ISIXLOG_'+get_option('log_level_vtimers').to_upper() : [],
	get_option('log_level_memory') != 'inherit'
		? '-DCONFIG_ISIX_LOGLEVEL_MEMORY=ISIXLOG_'+get_option('log_level_memory').to_upper() : [],
	get_option('log_level_semaphore') != 'inherit'
		? '-DCONFIG_ISIX_LOGLEVEL_SEMAPHORE=ISIXLOG_'+get_option('log_level_semaphore').to_upper() : [],
	get_option('log_level_events') != 'inherit'
		? '-DCONFIG_ISIX_LOGLEVEL_EVENTS=ISIXLOG_'+get_option('log_level_events').to_upper() : [],
	get_option('log_level_condvar') != 'inherit'
		? '-DCONFIG_ISIX_LOGLEVEL_CONDVAR=ISIXLOG_'+get_option('log_level_condvar').to_upper() : [],
	get_option('log_level_mutex') != 'inherit'
		? '-DCONFIG_ISIX_LOGLEVEL_MUTEX=ISIXLOG_'+get_option('log_level_mutex').to_upper() : [],
	get_option('log_level_osstats') != 'inherit'
		? '-DCONFIG_ISIX_LOGLEVEL_OSSTATS=ISIXLOG_'+get_option('log_level_osstats').to_upper() : [],

	get_option('test') ? [  # option overrides for test build
		'-DPDEBUG',
		'-DCONFIG_ISIX_SHUTDOWN_API',
		'-DCONFIG_ISIX_TASK_STACK_CHECK',
		'-DCONFIG_ISIX_FIFO_EVENT_NOTIFY',
		'-DCONFIG_ISIX_CPU_USAGE_API',
		'-DCONFIG_ISIX_MEMORY_PROTECTION_MODEL=ISIX_MPROT_LITE',
		'-DCONFIG_ISIX_LOGLEVEL=ISIXLOG_CRIT',
	] : [],
	language : ['c', 'cpp']
)
add_project_arguments('-Wno-volatile', language : 'cpp')

if get_option('enable_newlib_nano') and get_option('cpp_eh') != 'none'
	error('Unable to use nano version of newlib with exceptions enabled')
endif

meson.get_external_property('ram') # .ini crossfile must have ram
meson.get_external_property('flash') # .ini crossfile must have flash
_memory_defines = []  # MEMORY variables in linker script
foreach mem : ['ram', 'flash', 'ccmram', 'auxram', 'bkpram']
	bounds = meson.get_external_property(mem, ['0', '0K'])
	assert(bounds.length() == 2)
	_memory_defines += '-Wl,--defsym=_'+mem+'_origin='+bounds[0]
	_memory_defines += '-Wl,--defsym=_'+mem+'_length='+bounds[1]
endforeach

add_project_link_arguments(
	'-nostdlib', '-nostartfiles', '--specs=nosys.specs',
	not get_option('debug') ? ['-Wl,--gc-sections'] : [],
	get_option('enable_newlib_nano') and get_option('cpp_eh') == 'none' ? '--specs=nano.specs' : [],
	'-Wl,--defsym=_sys_stack_size='+get_option('stack_size').to_string(),
	_memory_defines,
	'-T' + meson.project_source_root() / 'libperiph/scripts' / host_machine.cpu_family() / meson.get_external_property('subarch') / 'flash.ld',
	'-lstdc++', '-lc', '-lm', '-lg', '-lgcc', '-flto=auto',
	language : ['c', 'cpp']
)

_config = declare_dependency(include_directories : 'config/include')

if get_option('disable_isix')
	subdir('libfoundation')
else
	_test_option_overrides = get_option('test') ? [
		'cpp_eh=default', 'cpp_rtti=true',  # lest needs exceptions
	] : []
	subdir('libisix')
	subdir('libfoundation')
	subdir('libperiph')
	subdir('libgfx')
	subdir('libgsm')
	subdir('libtcpip')
	subdir('libtls')
	if get_option('test')
		_lest = declare_dependency(
			include_directories : 'tests/lest/include',
			compile_args : [
				'-Dlest_FEATURE_COLOURISE=1',
				'-Dlest_FEATURE_WSTRING=0',
			]
		)
		subdir('tests/libisix')
	endif
endif
subdir('libenergymeter')