_cpuarch = 'arch' / host_machine.cpu_family()
_subarch = _cpuarch / meson.get_external_property('subarch')
_mach = _subarch / meson.get_external_property('mach_type')

_include = include_directories(
	'include', 'arch/include', _cpuarch/'include', _subarch/'include', _mach/'include')

_sources = {
	'arm' : 'arch/arm/kernel/irq_global.c',
	'cortexm' : [
		'arch/arm/cortexm/kernel/cache.c',
		'arch/arm/cortexm/kernel/hwarch.c',
		'arch/arm/cortexm/kernel/irq_platform.c',
		'arch/arm/cortexm/kernel/memprot.c',
		'arch/arm/cortexm/kernel/mpu.c',
		'arch/arm/cortexm/kernel/mpu.h',
		'arch/arm/cortexm/kernel/scheduler.c',
		'arch/arm/cortexm/kernel/systick.c',
	],
}

libisix = declare_dependency(
	link_with : library('isix',
		'kernel/condvar.c',
		'kernel/error.c',
		'kernel/events.c',
		'kernel/fifo.c',
		'kernel/fifo_event.c',
		'kernel/mempool.c',
		'kernel/mutex.c',
		'kernel/osstats.c',
		'kernel/ostime.c',
		'kernel/scheduler.c',
		'kernel/semaphore.c',
		'kernel/softtimers.c',
		'kernel/task.c',
		get_option('disable_tlsf')
			? [ 'kernel/mm/malloc/seqfit.c' ]
			: [ 'kernel/mm/malloc/tlsf.c',
				'kernel/mm/malloc/tlsf_isix.c' ],
		_sources[host_machine.cpu_family()],
		_sources[meson.get_external_property('subarch')],
		_mach/'scheduler.c',
		c_args : [
			'-D_ISIX_KERNEL_CORE_=1',
			meson.get_external_property('cache') ? '-D_ISIX_CACHE_PRESENT_=1' : [],
		],
		override_options : _test_option_overrides,
		include_directories : _include,
		dependencies : _config,
	),
	include_directories : _include,
	dependencies : _config,
	version : meson.project_version(),
	compile_args : '-DISIX_VERSION="'+meson.project_version()+'"',
)