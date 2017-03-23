#!/usr/bin/env python
# encoding: utf-8
# Thomas Nagy, 2006-2016 (ita)
# Ralf Habacker, 2006 (rh)
# Yinon Ehrlich, 2009

"""
g++/llvm detection.
"""

from waflib.Tools import ccroot, cross_none_ar
from waflib.Configure import conf





def options(conf):
	"""
	Configure option for gxx
	"""
	conf.add_option('--cross', default='arm-none-eabi-',
		help='Cross compiler prefix, e.g. arm-none-eabi-')

def configure(conf):
	"""
	Configuration for g++
	"""


