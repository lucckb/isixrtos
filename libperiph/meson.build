assert(get_option('crystal_hz') > 0, 'Missing -Dcrystal_hz=... in meson setup for libperiph')

_arch_sub = host_machine.cpu_family() / meson.get_external_property('subarch')
_vendor = meson.get_external_property('vendor')
_series = meson.get_external_property('series')

_include = include_directories(
	'periph/include',
	'periph/include/arch'/_arch_sub/_vendor,
	'periph/include/arch'/_arch_sub/_vendor/_series,
	'vendor'/_arch_sub/_vendor/_series/'include',
	_arch_sub == 'arm/cortexm' ? [
		'vendor'/_arch_sub/'cmsis/include',
		'vendor'/_arch_sub/'cmsis/device'/_vendor/_series/'include'
	] : [],
)

_sources = {
	'arm/cortexm' : [
		'periph/src/boot/arch/arm/cortexm/crashinfo.c',
	],
	'stm32' : [
		'periph/src/boot/arch/arm/cortexm/stm32/isr_bootvect.c',
		'periph/drivers/arm/cortexm/stm32/all/i2c/i2c_interrupt_handlers.cpp',
		'periph/drivers/arm/cortexm/stm32/all/spi/spi_master.cpp',
		'periph/drivers/arm/cortexm/stm32/all/spi/spi_interrupt_handlers.cpp',
		'periph/drivers/arm/cortexm/stm32/all/dma/dma_common.cpp',
		'periph/drivers/arm/cortexm/stm32/all/serial/uart_early.cpp',
	],
	'f3' : [
		'periph/drivers/arm/cortexm/stm32/f3/dma/dma_interrupt_handlers.cpp',
		'periph/drivers/arm/cortexm/stm32/f3/dma/stm32_dma_v1.cpp',
		'vendor/arm/cortexm/stm32/f3/src/stm32f3xx_ll_adc.c',
		'vendor/arm/cortexm/stm32/f3/src/stm32f3xx_ll_comp.c',
		'vendor/arm/cortexm/stm32/f3/src/stm32f3xx_ll_crc.c',
		'vendor/arm/cortexm/stm32/f3/src/stm32f3xx_ll_dac.c',
		'vendor/arm/cortexm/stm32/f3/src/stm32f3xx_ll_dma.c',
		'vendor/arm/cortexm/stm32/f3/src/stm32f3xx_ll_exti.c',
		'vendor/arm/cortexm/stm32/f3/src/stm32f3xx_ll_fmc.c',
		'vendor/arm/cortexm/stm32/f3/src/stm32f3xx_ll_gpio.c',
		'vendor/arm/cortexm/stm32/f3/src/stm32f3xx_ll_hrtim.c',
		'vendor/arm/cortexm/stm32/f3/src/stm32f3xx_ll_i2c.c',
		'vendor/arm/cortexm/stm32/f3/src/stm32f3xx_ll_opamp.c',
		'vendor/arm/cortexm/stm32/f3/src/stm32f3xx_ll_pwr.c',
		'vendor/arm/cortexm/stm32/f3/src/stm32f3xx_ll_rcc.c',
		'vendor/arm/cortexm/stm32/f3/src/stm32f3xx_ll_rtc.c',
		'vendor/arm/cortexm/stm32/f3/src/stm32f3xx_ll_spi.c',
		'vendor/arm/cortexm/stm32/f3/src/stm32f3xx_ll_tim.c',
		'vendor/arm/cortexm/stm32/f3/src/stm32f3xx_ll_usart.c',
	],
	'f4' : [
		'periph/drivers/arm/cortexm/stm32/f4/memory/stm32_sdram.cpp',
		'periph/drivers/arm/cortexm/stm32/f4/dma/dma_interrupt_handlers.cpp',
		'periph/drivers/arm/cortexm/stm32/f4/dma/stm32_dma_v2.cpp',
		'periph/drivers/arm/cortexm/stm32/f4/display/bus/dsi.cpp',
		'periph/drivers/arm/cortexm/stm32/f4/display/rgb/fbdev.cpp',
		'periph/drivers/arm/cortexm/stm32/f4/block/stm32_i2c_v1.cpp',
		'vendor/arm/cortexm/stm32/f4/src/stm32f4xx_ll_adc.c',
		'vendor/arm/cortexm/stm32/f4/src/stm32f4xx_ll_crc.c',
		'vendor/arm/cortexm/stm32/f4/src/stm32f4xx_ll_dac.c',
		'vendor/arm/cortexm/stm32/f4/src/stm32f4xx_ll_dma.c',
		'vendor/arm/cortexm/stm32/f4/src/stm32f4xx_ll_dma2d.c',
		'vendor/arm/cortexm/stm32/f4/src/stm32f4xx_ll_exti.c',
		'vendor/arm/cortexm/stm32/f4/src/stm32f4xx_ll_fmc.c',
		'vendor/arm/cortexm/stm32/f4/src/stm32f4xx_ll_fsmc.c',
		'vendor/arm/cortexm/stm32/f4/src/stm32f4xx_ll_gpio.c',
		'vendor/arm/cortexm/stm32/f4/src/stm32f4xx_ll_i2c.c',
		'vendor/arm/cortexm/stm32/f4/src/stm32f4xx_ll_lptim.c',
		'vendor/arm/cortexm/stm32/f4/src/stm32f4xx_ll_pwr.c',
		'vendor/arm/cortexm/stm32/f4/src/stm32f4xx_ll_rcc.c',
		'vendor/arm/cortexm/stm32/f4/src/stm32f4xx_ll_rng.c',
		'vendor/arm/cortexm/stm32/f4/src/stm32f4xx_ll_rtc.c',
		'vendor/arm/cortexm/stm32/f4/src/stm32f4xx_ll_sdmmc.c',
		'vendor/arm/cortexm/stm32/f4/src/stm32f4xx_ll_spi.c',
		'vendor/arm/cortexm/stm32/f4/src/stm32f4xx_ll_tim.c',
		'vendor/arm/cortexm/stm32/f4/src/stm32f4xx_ll_usart.c',
		'vendor/arm/cortexm/stm32/f4/src/stm32f4xx_ll_usb.c',
	],
}

_deps = [libisix, libfoundation]

_defines = [
	'-DUSE_FULL_LL_DRIVER=1',
	'-DHSE_VALUE=' + get_option('crystal_hz').to_string(),
]

libperiph = declare_dependency(
	link_with : library('periph',
		'periph/src/core/peripheral_manager.cpp',
		'periph/src/boot/newlib/syscalls.c',
		'periph/src/boot/crt/crt0.c',
		'periph/src/dt/dts_database.cpp',
		'periph/src/dma/controller.cpp',
		'periph/src/display/rgb/idisplay.cpp',
		'periph/src/display/rgb/otm8009a.cpp',
		'periph/src/display/mono/ssd1306.cpp',
		'periph/src/display/mono/uc1601_display.cpp',
		'periph/src/display/mono/hd44xx_display.cpp',
		_sources[_arch_sub],
		_sources[_vendor],
		_sources[_series],
		c_args : _defines,
		cpp_args : _defines,
		include_directories : _include,
		dependencies : _deps,
	),
	compile_args : _defines,
	include_directories : _include,
	dependencies : _deps,
)