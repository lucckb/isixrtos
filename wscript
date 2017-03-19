#!/usr/bin/env python


bld_isixlibs = [
    'libisix',
    'libfoundation',
    'libstm32'
]

def options(opt):
    opt.add_option('--cross', default='arm-none-eabi-', help='Cross compiler prefix, e.g. arm-none-eabi-')
    opt.add_option('--cpu', help='selected cpu for example stm32f407zet6' )
    opt.load( 'compiler_cxx' )
    opt.load( 'compiler_c' )
    opt.recurse( bld_isixlibs )


def configure(conf):
    conf.load('isix_toolchain')
    conf.set_cross_toolchain()
    conf.recurse( bld_isixlibs )


def build(bld):
    bld.recurse( bld_isixlibs )
