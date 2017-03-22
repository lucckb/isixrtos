#!/usr/bin/env python
# encoding: utf-8

from waflib.Configure import conf
from waflib import Logs,Context
import os
import json

# Configure toolchain
def configure(conf):
    _set_default_options(conf)
    conf.env.CROSS = conf.options.cross
    conf.env.CC = conf.env.CROSS +  'gcc'
    conf.env.CXX = conf.env.CROSS + 'g++'
    conf.env.CPP = conf.env.CROSS + 'cpp'
    conf.env.AS = conf.env.CROSS +  'gcc'
    conf.env.AR = conf.env.CROSS +  'ar'
    conf.env.OBJDUMP = conf.env.CROSS + 'objdump'
    conf.env.OBJCOPY = conf.env.CROSS +  'objcopy'
    conf.env.OBJSIZE = conf.env.CROSS + 'size'
    conf.load( 'compiler_cxx' )
    conf.load( 'compiler_c' )
    conf.load( 'gcc gas' )
    conf.load( 'objcopy' )
    conf.load( 'isix_link' )
    conf.load( 'build_summary' )
    conf.load( 'isix_cpudb' )
    conf.find_program( 'git', var='GIT', mandatory=True )



# Get Git repository version
@conf
def git_repo_version(conf):
    cmd = conf.env.GIT + [ 'describe', '--tags', '--dirty', '--long' ]
    out = conf.cmd_and_log( cmd, cwd=conf.path, quiet=Context.BOTH )
    return out.strip()

# On options
def options(conf):
    conf.add_option('--cross', default='arm-none-eabi-',
            help='Cross compiler prefix, e.g. arm-none-eabi-')
    conf.add_option('--disable-defconfig',
            help='Disable reading configuration from config.json',
            action='store_true', default=False )
    conf.load( 'compiler_cxx' )
    conf.load( 'compiler_c' )
    conf.load( 'isix_cpudb' )


# Read default configuration
@conf
def read_default_configuration(conf):
    if conf.top_dir: fname = conf.top_dir
    else: fname = os.getcwd()
    fname = os.path.join( fname, 'config.json' )
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
    if 'configure' in cfg:
        cfg = cfg['configure']
        for key,value in cfg.items():
            if hasattr(conf.options,key):
                conf.msg('Configuration','%s=%s'%(key,value))
                setattr(conf.options,key,value)


