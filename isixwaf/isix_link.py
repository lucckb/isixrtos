#!/usr/bin/env python
# encoding: utf-8

import waflib
from waflib.Tools import ccroot
from waflib import Tools,Errors
from waflib.TaskGen import after, feature, after_method

@after('apply_link')
@feature('cprogram', 'cxxprogram' )
def process_ldscript(self):
        #if not getattr(self, 'ldscript', None):
            #or self.env.CC_NAME != 'gcc':
        #    return
        #node = self.path.find_resource(self.ldscript)
        #if not node:
            #raise Errors.WafError('could not find %r' % self.ldscript)
        node = self.env.CRT0_LINKER_SCRIPT
        if not node:
            raise Errors.WafError('could not find %r' % 'x')

        self.link_task.env.append_value('LINKFLAGS', ['-Wl,-T%s'% node.abspath()] )
        self.link_task.dep_nodes.append(node)
        self.link_task.env.append_value('LDFLAGS', [ '-Wl,--start-group', '-lstdc++', '-lc', '-lm', '-lg', '-lgcc', '-Wl,--end-group' ] )



@feature('cprogram', 'cxxprogram' )
@after_method('apply_link')
def force_static_linking(self):
	env = self.link_task.env
	env.STLIB += env.LIB
	env.LIB = []
	env.SHLIB_MARKER = ''
	env.STLIB_MARKER = ''


