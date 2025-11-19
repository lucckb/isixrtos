isix_tests_src = files(
	'01_basic_primitives.cpp',
	'02_mempool.cpp',
	'03_mutex.cpp',
	'04_sched_suspend.cpp',
	'05_semaphores.cpp',
	'06_tasks.cpp',
	'07_vtimer.cpp',
	'08_fifo.cpp',
	'09_events.cpp',
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

executable('isixtests',
	sources: isix_tests_src,
	override_options : _test_option_overrides,
	include_directories : ['include', 'utils'],
	dependencies : [
		libperiph,
		_lest,
		libisix,
	],
	link_with : [ fpu_asm_lib ],
)