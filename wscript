#!/usr/bin/env python
# encoding: utf-8


bld_isixlibs = [
    'libisix',
    'libfoundation',
    'libstm32',
]

def options(opt):
    opt.load( 'isix_toolchain')
    opt.load( 'isix_jtagprg' )
    opt.recurse( bld_isixlibs )


def configure(conf):
    conf.load('isix_toolchain')
    conf.load( 'isix_jtagprg' )
    conf.recurse( bld_isixlibs )


def build(bld):
    bld.set_build_summary()
    _inc = 'config/include'
    bld( includes = _inc, export_includes=_inc, name='common_conf' )
    bld.recurse( bld_isixlibs )

#Special target program waflib
def program(ctx):
    ctx.load( 'isix_jtagprg' )

#Special rule for debugging
def ocddebug(ctx):
    ctx.load( 'isix_jtagprg' )


from waflib.Build import BuildContext
class jtgprog( BuildContext ):
    cmd = 'program'
    fun = 'program'


from waflib.Build import BuildContext
class jtgdebug( BuildContext ):
    cmd = 'ocddebug'
    fun = 'ocddebug'
