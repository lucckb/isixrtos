#!/usr/bin/env python
# encoding: utf-8


import os
import json
from waflib.Errors import WafError
from waflib.Configure import conf

_cpu_db_ = None;
_mcus_db = None;
_base_dir = os.path.dirname( os.path.abspath(__file__) )

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

def _cpu_names():
    mcu = _read_json_db('mcus.json')
    return sorted(list(mcu['mcus'].keys()))

def _get_flag(mcu, flag):
    db = _read_json_db('mcus.json');
    db1 = _read_json_db('cores.json')
    db.update(db1)
    #if flag in mcu: return mcu[flag]
    #else: return []
    cpu = db['mcu']['cpu']
    return db['mcus'][mcu][flag] + db['cores'][cpu][flag]

def options(opt):
    cpu_names = _cpu_names()
    opt.add_option('--cpu', action='store', choices=cpu_names,
            help='Selected cpu for example stm32f107vbt6' )

@conf
def mcu_setflags(cfg):
    cfg.env.CFLAGS += _get_flag(cfg.options.cpu, 'cflags');
    cfg.env.CXXFLAGS += _get_flag(cfg.options.cpu, 'cflags');
