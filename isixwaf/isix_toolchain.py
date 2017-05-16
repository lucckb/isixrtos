#!/usr/bin/env python
# encoding: utf-8

from waflib.Configure import conf
from waflib import Logs,Context
import os
import json




# Configure toolchain
def configure(conf):
    _set_default_options(conf)
    conf.find_program( 'git', var='GIT', mandatory=True )
    conf.load( 'cross_none_gnu' )
    conf.load( 'cross_none_objcopy' )
    conf.load( 'isix_link' )
    conf.load( 'build_summary' )
    conf.load( 'isix_cpudb' )


#Define options
def options():
     conf.load( 'cross_none_gnu' )


# Get Git repository version
@conf
def git_repo_version(conf):
    cmd = conf.env.GIT + [ 'describe', '--tags', '--dirty', '--long', '--always' ]
    out = conf.cmd_and_log( cmd, cwd=conf.path, quiet=Context.BOTH )
    return out.strip()

# On options
def options(conf):
    conf.add_option('--cross', default='arm-none-eabi-',
            help='Cross compiler prefix, e.g. arm-none-eabi-')
    conf.add_option('--disable-defconfig',
            help='Disable reading configuration from config.json',
            action='store_true', default=False )
    conf.load( 'isix_cpudb' )


# Read default configuration
@conf
def read_default_configuration(conf):
    fname = os.path.join( os.getcwd(), 'config.json' )
    if not os.path.isfile(fname) and conf.top_dir:
        fname = os.path.join( conf.top_dir, 'config.json' )
    try:
        with open(fname) as fh:
            cfg = json.load(fh)
    except json.decoder.JSONDecodeError as err:
        raise WafError('Syntax error in app config %s %r'%(fname,err), err )
    except FileNotFoundError:
        cfg = None
    return cfg


# Setup default configuration
def _set_default_options(conf):
    if conf.options.disable_defconfig:
        return
    cfg = read_default_configuration(conf)
    if cfg and 'configure' in cfg:
        cfg = cfg['configure']
        for key,value in cfg.items():
            if hasattr(conf.options,key):
                conf.msg('Configuration','%s=%s'%(key,value))
                setattr(conf.options,key,value)
