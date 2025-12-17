unity_sub = subproject('unity',
	default_options: { 'extension_fixture' : true },
)
unity_src = unity_sub.get_variable('unity_src')
unity_inc = unity_sub.get_variable('unity_inc')
unity_args = unity_sub.get_variable('unity_args')
unity_args += '-DUNITY_INCLUDE_CONFIG_H'

isix_tests_src = files(
	'01_basic_primitives.cpp',
	'02_mempool.cpp',
	'03_mutex.cpp',
	'04_sched_suspend.cpp',
	'05_semaphores.cpp',
	'06_tasks.cpp',
	# '07_vtimer.cpp',
	# '08_fifo.cpp',
	# '09_events.cpp',
	'unittests_entry.cpp',
	'utils/board_boot.cpp',
	'utils/dts.cpp',
	'utils/event_groups_demo.c',
	'utils/task_test_helper.c',
	'utils/timer_interrupt.cpp',
)


fpu_asm_lib = static_library(
  'sp_fpu_test_and_set',
  'utils/sp_fpu_test_and_set.S',
  include_directories : ['include', 'utils'],
	c_args: ['-fno-lto'],
	cpp_args: ['-fno-lto'],
)

isix_tests_elf = executable('isixtests',
	sources: [isix_tests_src, unity_src],
	include_directories : ['include', 'utils', unity_inc],
	dependencies : [
		libperiph,
		libisix,
		libfoundation,
	],
	c_args: unity_args,
	link_with : [ fpu_asm_lib ],
)

_objcopy = find_program(meson.get_external_property('objcopy'))
custom_target('isixtests.binary',
	input: isix_tests_elf,
	output: 'isixtests.binary',
	command: [_objcopy, '-O', 'binary', '@INPUT@', '@OUTPUT@'],
	build_by_default: true
)

_size = find_program(meson.get_external_property('size'))
run_target('Print size', command: [_size, isix_tests_elf])
