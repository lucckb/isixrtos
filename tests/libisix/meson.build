unity_sub = subproject('unity',
	default_options: { 'extension_fixture' : true },
)
unity_src = unity_sub.get_variable('unity_src')
unity_inc = unity_sub.get_variable('unity_inc')
unity_args = [
	unity_sub.get_variable('unity_args'),
	'-DUNITY_FIXTURE_NO_EXTRAS',
	'-include', 'foundation/sys/tiny_printf.h',
	'-DUNITY_OUTPUT_CHAR=tiny_putchar',
	'-DUNITY_OUTPUT_COLOR',
]

isix_tests_src = files(
	'00_exceptions.cpp',
	'01_basic_primitives.cpp',
	'02_mempool.cpp',
	'03_mutex.cpp',
	'04_sched_suspend.cpp',
	'05_semaphores.cpp',
	'06_tasks.cpp',
	'07_vtimer.cpp',
	'08_fifo.cpp',
	'09_events.cpp',
	'10_tickless.cpp',
	'11_coroutines.cpp',
	'12_heap.cpp',
	'13_irq_priority.cpp',
	'14_coroutines_ext.cpp',
	'15_coroutines_bench.cpp',
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
	c_args : unity_args,
	cpp_args : unity_args,
	link_args : [
		'-Wl,-u,isix_pre_sleep_hook',
		'-Wl,-u,isix_post_sleep_hook',
	],
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

_check_pendsv = find_program('../../extras/scripts/check_pendsv.sh')
custom_target('isixtests.pendsv_ok',
	input: isix_tests_elf,
	output: 'isixtests.pendsv_ok',
	command: [_check_pendsv, '@INPUT@', '@OUTPUT@'],
	env: {
		'OBJDUMP': find_program('arm-none-eabi-objdump').full_path(),
		'NM': find_program('arm-none-eabi-nm').full_path(),
	},
	build_by_default: true
)
