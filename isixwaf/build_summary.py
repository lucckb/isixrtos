#!/usr/bin/env python
# encoding: utf-8

from waflib.Configure import conf
from waflib import Logs,Context



#Execute OBJSIZE command
def _parse_size( bld, path ):
    cmd = [ bld.env.get_flat('OBJSIZE'), '-A', path ]
    out = bld.cmd_and_log( cmd, cwd=bld.bldnode.abspath(), quiet=Context.BOTH )
    lines = out.splitlines()[2:]
    sects = {};
    for line in lines:
        row = line.strip().split()
        if len(row) > 1:
            name = row[0]
            size = int(row[1])
            sects[name] = size
    return sects





def _build_summary_(bld):
    if bld.targets == '*' or bld.targets == '':
        taskgens = bld.get_all_task_gen()
    else:
        targets = bld.targets.split(',')
        taskgens = [bld.get_tgen_by_name(t) for t in targets]
    for tg in taskgens:
        if hasattr( tg, 'link_task' ) and tg.typ=='program':
            sects = _parse_size( bld, tg.link_task.outputs[0].relpath() )
            flash = (sects['.isr_vector'] + sects['.text']
            + (sects['.ARM.exidx'] if '.ARM.exidx' in sects else 0 )
            + sects['.data'])/1024.0
            ram = (sects['.data'] + sects['.bss'] + sects['.stack'])/1024.0
            if '.auxram' in sects:
                eram = sects['.auxram'] / 1024.0
                eram_str = 'AUXRAM: %1.fkB' % eram
            else:
                eram_str = ''
            heap = sects['.heap'] / 1024.0
            name = tg.link_task.outputs[0].bldpath()
            Logs.info('%sAppsize %s%s FLASH: %.1fkB RAM: %.1fkB HEAP: %.1fkB %s'
                    %( Logs.colors.NORMAL,Logs.colors.YELLOW,name,flash,ram,heap,eram_str ) )


@conf
def set_build_summary(bld):
    bld.add_post_fun( _build_summary_ )


def configure(conf):
    conf.find_program( '%ssize'% conf.options.cross, var='OBJSIZE', mandatory=True )
