#!/usr/bin/env python
# encoding: utf-8

from waflib.Configure import conf
from waflib import Logs,Context

# Configure toolchain
def configure(conf):
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
    cmd = [ 'git', 'describe', '--tags', '--dirty', '--long' ]
    out = conf.cmd_and_log( cmd, cwd=conf.path, quiet=Context.BOTH )
    return out.strip()


# On options
def options(conf):
    conf.load( 'isix_cpudb' )
