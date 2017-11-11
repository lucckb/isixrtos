#!/usr/bin/env python
# encoding: utf-8

import waflib
from waflib.Tools import ccroot
from waflib import Tools,Errors, Task
from waflib.TaskGen import after, feature, after_method, before, extension

_stdlib_lflags = [ '-Wl,--start-group', '-lstdc++', '-lc', '-lm', '-lg', '-lgcc', '-Wl,--end-group' ]

_tmplink_file = 'ldscript.ld'


@extension('.ld')
def ld_hook(self, node):
    pass



@after('apply_link')
@feature('cprogram', 'cxxprogram' )
def process_ldscript(self):
        node = self.path.find_resource(_tmplink_file)
        if not node:
            raise Errors.WafError('could not find %r' % node)
        if node:
            self.link_task.env.append_value('LDFLAGS', ['-Wl,-T%s'% node.abspath()] )
            self.link_task.dep_nodes.append(node)
        self.link_task.env.append_value('LDFLAGS', _stdlib_lflags )


#Only static libraries toolchain
@feature('cprogram', 'cxxprogram' )
@after_method('apply_link')
def force_static_linking(self):
	env = self.link_task.env
	#env.STLIB += env.LIB
	#env.LIB = []
	env.SHLIB_MARKER = ''
	env.STLIB_MARKER = ''



@feature('cprogram', 'cxxprogram' )
@before('apply_link')
def generate_ldcript(self):
    linker_memmap = self.bld.isix_get_link_memmap()
    if self.env.BOOTLOADER_REMAP_SHIFT_KB:
        _bootloader_update_linker_map( linker_memmap, self.env.BOOTLOADER_REMAP_SHIFT_KB )
    tsk = self.create_task('generate_link_for_cpu',
            self.env.CRT0_LINKER_SCRIPT,
            self.path.find_or_declare( _tmplink_file ),
            memmap=linker_memmap
    )
    self.source.extend( tsk.outputs )

# Linker script task generator
class generate_link_for_cpu(Task.Task):
    color = 'GREEN'

    # Task keyword generator
    def keyword(self):
        return 'Createld'


    # Master task runner
    def run(self):
        with open( self.inputs[0].abspath(), 'rt', encoding='utf-8' ) as src:
            with open( self.outputs[0].abspath(), 'wt', encoding='utf-8' ) as dst:
                sect = self.write_header( dst )
                self.append_origin( dst, src, sect )
        return 0

    #Write application
    def write_header( self, hwnd ):
        hwnd.write( 'MEMORY\n' )
        hwnd.write( '{\n' )
        extra_sect = []
        for key, val in self.memmap.items():
            fstr = '\t%s (%s) : ORIGIN = %s, LENGTH = %s\n' % ( key.upper(),
                    val['attrib'], val['origin'], val['length'] )
            hwnd.write( fstr )
            if key !='flash' and key!='ram':
                extra_sect += [ key ]
        hwnd.write( '}\n' )
        return extra_sect

    #Append origin script
    def append_origin( self, dst, src, sect ):
        origl = src.readlines()
        lines = []
        for line in origl:
            if line.find( '<WAFINSERT>' ) != -1:
                lines += [ line ]
                lines += self.append_sections(line,sect)
            else:
                lines += [ line ]
        dst.writelines(lines)

    # Append specific section
    def append_sections( self, line, sects ):
        lines = []
        for sect in sects:
            lines += [ '\t.%s (NOLOAD):\t\n' % sect ]
            lines += [ '\t{\n' ]
            lines += [ '\t\t*(.%s*)\n' % sect ]
            lines += [ '\t\t. = ALIGN(4);\n' ]
            lines += [ '\t} > %s\n' % sect.upper() ]
        return lines


# Update bootloader linker map
def _bootloader_update_linker_map( link_map, remap_kb ):
    fl_origin = int(link_map['flash']['origin'],16)
    fl_size =_si_size_to_int(link_map['flash']['length'])
    remap = remap_kb * 1024
    fl_origin = fl_origin + remap
    fl_size = fl_size - remap
    link_map['flash']['origin'] = '0x%X' % fl_origin
    link_map['flash']['length'] = _int_to_si_size(fl_size)


# Convert SI to INT
def _si_size_to_int( sstr ):
    sstr = sstr.strip().upper()
    mul = 1
    if sstr.endswith('K'):
        mul = 1024
        sstr = sstr[:-1]
    elif sstr.endswith('M'):
        mul = 1024 * 1024
        sstr = sstr[:-1]
    return int(sstr)*mul

# Convert INT to SI
def _int_to_si_size( siz ):
    suffix = ''
    div = 1
    if siz % (1024*1024) == 0:
        suffix = 'M'
        div = 1024*1024
    elif siz % 1024 == 0:
        suffix = 'K'
        div = 1024
    return '%i%s' %( siz/div, suffix )
