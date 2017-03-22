#!/usr/bin/env python
# encoding: utf-8

#Common project cflags
_cflags = ['-Wno-variadic-macros', '-Wno-long-long', '-pipe', '-Wextra', '-Wall' ]
_cflags_ndebug = [ '-fomit-frame-pointer', '-ffunction-sections', '-fdata-sections', '-flto' ]
_ldflags_ndebug = [ '-Wl,--gc-sections', '-flto' ]
_cflags_debug = [ '-g' ]

import os
import json
from waflib.Errors import WafError
from waflib.Configure import conf


_cpu_db = None
_mcus_db = None
_base_dir = os.path.dirname( os.path.abspath(__file__) )

# Read json database by name
def _read_cpu_db():
    global _cpu_db
    try:
        if _cpu_db == None:
            fname = os.path.join( _base_dir, 'cpu_database', 'cpu.json' )
            with open( fname ) as fh:
                _cpu_db = json.load( fh )
    except  IOError as e:
        raise WafError( 'Unable to read cpudb %s %r'% (fname,e), e)
    except json.decoder.JSONDecodeError  as e:
        raise WafError( 'Syntax error in the cpudb %s %r'% (fname,e), e)
    return _cpu_db


# Read json database by name
def _read_mcu_db():
    global _mcus_db
    try:
        if _mcus_db == None:
            fname = os.path.join( _base_dir, 'cpu_database', 'mcu.json' )
            with open( fname ) as fh:
                _mcus_db = json.load( fh )
    except  IOError as e:
        raise WafError( 'Unable to read cpudb %s %r'% (fname,e), e)
    except json.decoder.JSONDecodeError  as e:
        raise WafError( 'Syntax error in the cpudb %s %r'% (fname,e), e)
    return _mcus_db

# Get cpu names
def _cpu_names():
    mcu = _read_mcu_db()
    return sorted(list(mcu['mcus'].keys()))

# Get selected flag mcu,cpu
def _get_flag(mcu, flag, scope='both' ):
    db = _read_mcu_db()
    if scope !='mcu':
        db.update(  _read_cpu_db() )
        cpu = db['mcus'][mcu]['cpu']
    if scope == 'both':
        return db['mcus'][mcu][flag] + db['cores'][cpu][flag]
    elif scope == 'cpu':
        return db['cores'][cpu][flag]
    elif scope == 'mcu':
        return db['mcus'][mcu][flag]

# Setup options
def options(opt):
    cpu_names = _cpu_names()
    opt.add_option('--cpu', action='store', choices=cpu_names,
            help='Selected cpu for example stm32f107vbt6' )
    opt.add_option('--debug', help='Configure with debug variant',
            action='store_true', default=False )
    opt.add_option('--disable-isix',
            help='Configure standalone application without isix',
            action='store_true', default=False )
    opt.add_option('--disable-exceptions',
            help='Disable exceptions handling in the toolchain',
            action='store_true', default=False )
    opt.add_option('--optimize', default='s', action='store',
            choices=['0','1','2','3','s'],
            help='Compiler optimization flag. [default: s]'
            )



# MCU set flag configuration
def configure(cfg):
    if not cfg.options.cpu:
        raise WafError('Missing option CPU type not provided')
    cfg.env.ISIX_CPU_TYPE = cfg.options.cpu
    cflags = _get_flag(cfg.options.cpu, 'cflags')
    if cfg.options.debug == True:
        cfg.env.ASFLAGS += [ '-gstabs' ]
        cflags += _cflags_debug
        cfg.env.DEFINES += [ 'PDEBUG' ]
    else:
        cflags += _cflags_ndebug
        cfg.env.LDFLAGS = _ldflags_ndebug
    optflag = [ '-O%s' % cfg.options.optimize ]
    cfg.env.CFLAGS += cflags + ['-std=gnu11' ] + optflag
    cfg.env.CXXFLAGS += cflags + [ '-std=gnu++14' ] + optflag
    cfg.env.ASFLAGS += cflags + [ '-Wa,-mapcs-32' ] + optflag
    cfg.env.DEFINES += _get_flag(cfg.options.cpu,'defs')
    cfg.env.LDFLAGS += [ '-nostdlib', '-nostartfiles' ] + cflags + optflag
    if cfg.options.disable_isix == False:
        cfg.env.DEFINES += [ 'COMPILED_UNDER_ISIX', 'ISIX_CONFIG_USE_PREEMPTION' ]
    if cfg.options.disable_exceptions == True:
        cfg.env.CXXFLAGS += [ '-fno-exceptions', '-fno-rtti' ]
    # Always main function return and always C++ code
    cfg.env.DEFINES += [ 'CPP_STARTUP_CODE', 'FUNCTION_MAIN_RETURN' ]
    # Optionaly force DSO handle
    #cfg.env.prepend_value('LINKFLAGS', '-Wl,--undefined=__dso_handle')


# Get linker script name
@conf
def isix_get_linker_script_name(bld):
    return _get_flag( bld.env.ISIX_CPU_TYPE,'link_script','mcu' ) + ".ld"


# Get isix architecture
@conf
def isix_get_arch(bld):
    return _get_flag( bld.env.ISIX_CPU_TYPE,'isix_arch', 'cpu' );

@conf
def isix_get_cpu_family(bld):
    return _get_flag( bld.env.ISIX_CPU_TYPE,'family','mcu' )

