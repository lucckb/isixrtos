#!/usr/bin/env python


bld_isixlibs = [
    'libisix',
    'libfoundation',
    'libstm32'
]

def options(opt):
    opt.load( 'isix_cpudb')
    opt.add_option('--cross', default='arm-none-eabi-',
            help='Cross compiler prefix, e.g. arm-none-eabi-')
    opt.add_option('--debug', help='Configure with debug variant',
            action='store_true', default=False )
    opt.add_option('--without-isix', 
            help='Configure standalone application without isix',
            action='store_true', default=False )
    opt.load( 'compiler_cxx' )
    opt.load( 'compiler_c' )
    opt.load( 'gcc gas' )
    opt.recurse( bld_isixlibs )


def configure(conf):
    conf.load('isix_toolchain')
    conf.set_cross_toolchain()
    conf.mcu_setflags()
    conf.recurse( bld_isixlibs )


def build(bld):
    bld.set_build_summary()
    bld.recurse( bld_isixlibs )
