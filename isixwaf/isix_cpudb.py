#!/usr/bin/env python
# encoding: utf-8

#Common project cflags
_cflags = ['-Wno-variadic-macros', '-Wno-long-long', '-pipe', '-Wextra', '-Wall' ]


import os
import json
from waflib.Errors import WafError
from waflib.Configure import conf


_cpu_db_ = None
_mcus_db = None
_base_dir = os.path.dirname( os.path.abspath(__file__) )

# Read json database by name
def _read_json_db(filename):
    try:
        if _cpu_db_ == None:
            fname = os.path.join( _base_dir, 'cpu_database', filename )
            with open( fname ) as fh:
                _mcus_db = json.load( fh )
    except  IOError as e:
        raise WafError( 'Unable to read cpudb %s %r'% (fname,e), e)
    except json.decoder.JSONDecodeError  as e:
        raise WafError( 'Syntax error in the cpudb %s %r'% (fname,e), e)
    return _mcus_db

# Get cpu names
def _cpu_names():
    mcu = _read_json_db('mcu.json')
    return sorted(list(mcu['mcus'].keys()))

# Get selected flag mcu,cpu
def _get_flag(mcu, flag, scope='both' ):
    db = _read_json_db('mcu.json');
    if scope !='mcu':
        db.update(  _read_json_db('cpu.json') )
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

# MCU set flag configuration
@conf
def mcu_setflags(cfg):
    if not cfg.options.cpu:
        raise WafError('Missing option CPU type not provided')
    cfg.env.ISIX_CPU_TYPE = cfg.options.cpu
    cflags = _get_flag(cfg.options.cpu, 'cflags')
    cflags += _cflags
    cfg.env.CFLAGS += cflags + ['-std=gnu11' ]
    cfg.env.CXXFLAGS += cflags + [ '-std=gnu++14' ]
    cfg.env.ASFLAGS += cflags + [ '-Wa,-mapcs-32' ]
    cfg.env.DEFINES += _get_flag(cfg.options.cpu,'defs')
    cfg.env.LDFLAGS += [ '-nostdlib', '-nostartfiles' ]

# Get linker script name
@conf
def isix_get_linker_script_name(bld):
    return _get_flag( bld.env.ISIX_CPU_TYPE,'link_script','mcu' ) + ".ld"


# Get isix architecture
@conf
def isix_get_arch(bld):
    return _get_flag( bld.env.ISIX_CPU_TYPE,'isix_arch', 'cpu' );
