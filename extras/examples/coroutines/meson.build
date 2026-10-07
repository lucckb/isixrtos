# Coroutine examples. Board support is shared with the unit tests.
_boot_sources = files(
	'example_support.cpp',
	'../../../tests/libisix/utils/board_boot.cpp',
	'../../../tests/libisix/utils/dts.cpp',
)

_examples = [
	'01_blinky_sleep',
	'02_uart_echo',
	'03_spi_sensor_offload',
	'04_timeouts_and_cancel',
	'05_two_executors',
]

_check_pendsv = find_program('../../scripts/check_pendsv.sh')
_binutils_env = {
	'OBJDUMP': find_program('arm-none-eabi-objdump').full_path(),
	'NM': find_program('arm-none-eabi-nm').full_path(),
}

foreach name : _examples
	_elf = executable('co_' + name,
		name + '.cpp',
		_boot_sources,
		include_directories : include_directories('../../../tests/libisix/include'),
		dependencies : [ libperiph, libisix, libfoundation ],
		override_options : ['b_lto=false'],
		link_args : [
			'-Wl,-u,isix_pre_sleep_hook',
			'-Wl,-u,isix_post_sleep_hook',
		],
	)

	custom_target('co_' + name + '.pendsv_ok',
		input: _elf,
		output: 'co_' + name + '.pendsv_ok',
		command: [_check_pendsv, '@INPUT@', '@OUTPUT@'],
		env: _binutils_env,
		build_by_default: true
	)
endforeach
