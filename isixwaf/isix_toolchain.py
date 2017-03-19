#!/usr/bin/env python
# encoding: utf-8

from waflib.Configure import conf

@conf
def set_cross_toolchain(conf):
    conf.env.CROSS = conf.options.cross
    conf.env.CC = conf.env.CROSS +  'gcc'
    conf.env.CXX = conf.env.CROSS + 'g++'
    conf.env.CPP = conf.env.CROSS + 'cpp'
    conf.env.AS = conf.env.CROSS +  'as'
    conf.env.AR = conf.env.CROSS +  'ar'
    conf.env.OBJDUMP = conf.env.CROSS + 'objdump'
    conf.env.OBJCOPY = conf.env.CROSS +  'objcopy'
    conf.load( 'compiler_cxx' )
    conf.load( 'compiler_c' )
    conf.load( 'objcopy' )
    conf.load( 'isix_link' )




