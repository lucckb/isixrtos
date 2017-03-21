#!/usr/bin/env python


bld_isixlibs = [
    'libisix',
    'libfoundation',
    'libstm32',
    'libisixdrvstm32',
    'libgfx',
    'libfsfat'
]

def options(opt):
    opt.load( 'isix_toolchain')
    opt.load( 'compiler_cxx' )
    opt.load( 'compiler_c' )
    opt.recurse( bld_isixlibs )


def configure(conf):
    conf.load('isix_toolchain')
    conf.recurse( bld_isixlibs )


def build(bld):
    bld.set_build_summary()
    bld.recurse( bld_isixlibs )
