#!/usr/bin/env python
# encoding: utf-8
# Thomas Nagy, 2006-2016 (ita)
# Ralf Habacker, 2006 (rh)
# Yinon Ehrlich, 2009
# Cross compiler support for embedded by LBryndza

"""
gcc/llvm detection.
"""

from waflib.Tools import ccroot
from waflib.Configure import conf


@conf
def find_gcc(conf):
	"""
	Find the program gcc, and if present, try to detect its version number
	"""
	cross_gcc = '%sgcc' % conf.options.cross
	cross_cc = '%scc'  % conf.options.cross
	cc = conf.find_program([cross_gcc, cross_cc], var='CC')
	conf.get_cc_version(cc, gcc=True)
	conf.env.CC_NAME = 'gcc'

@conf
def gcc_common_flags(conf):
	"""
	Common flags for gcc on nearly all platforms
	"""
	v = conf.env

	v.CC_SRC_F            = []
	v.CC_TGT_F            = ['-c', '-o']

	if not v.LINK_CC:
		v.LINK_CC = v.CC

	v.CCLNK_SRC_F         = []
	v.CCLNK_TGT_F         = ['-o']
	v.CPPPATH_ST          = '-I%s'
	v.DEFINES_ST          = '-D%s'

	v.LIB_ST              = '-l%s' # template for adding libs
	v.LIBPATH_ST          = '-L%s' # template for adding libpaths
	v.STLIB_ST            = '-l%s'
	v.STLIBPATH_ST        = '-L%s'
	v.RPATH_ST            = ''

	v.SONAME_ST           = ''
	v.SHLIB_MARKER        = ''
	v.STLIB_MARKER        = ''

	v.cprogram_PATTERN    = '%s'

	v.CFLAGS_cshlib       = []
	v.LINKFLAGS_cshlib    = []
	v.cshlib_PATTERN      = 'lib%s.so'

	v.LINKFLAGS_cstlib    = ['-Wl,-Bstatic']
	v.cstlib_PATTERN      = 'lib%s.a'

	v.LINKFLAGS_MACBUNDLE = []
	v.CFLAGS_MACBUNDLE    = []
	v.macbundle_PATTERN   = '%s.bundle'



@conf
def find_gxx(conf):
	"""
	Finds the program g++, and if present, try to detect its version number
	"""
	cross_gxx = '%sg++' % conf.options.cross
	cross_cxx = '%sc++'  % conf.options.cross
	cxx = conf.find_program([cross_gxx, cross_cxx], var='CXX')
	conf.get_cc_version(cxx, gcc=True)
	conf.env.CXX_NAME = 'gcc'

@conf
def gxx_common_flags(conf):
	"""
	Common flags for g++ on nearly all platforms
	"""
	v = conf.env

	v.CXX_SRC_F           = []
	v.CXX_TGT_F           = ['-c', '-o']

	if not v.LINK_CXX:
		v.LINK_CXX = v.CXX

	v.CXXLNK_SRC_F        = []
	v.CXXLNK_TGT_F        = ['-o']
	v.CPPPATH_ST          = '-I%s'
	v.DEFINES_ST          = '-D%s'

	v.LIB_ST              = '-l%s' # template for adding libs
	v.LIBPATH_ST          = '-L%s' # template for adding libpaths
	v.STLIB_ST            = '-l%s'
	v.STLIBPATH_ST        = '-L%s'
	v.RPATH_ST            = ''

	v.SONAME_ST           = ''
	v.SHLIB_MARKER        = ''
	v.STLIB_MARKER        = ''

	v.cxxprogram_PATTERN  = '%s'

	v.CXXFLAGS_cxxshlib   = []
	v.LINKFLAGS_cxxshlib  = []
	v.cxxshlib_PATTERN    = 'lib%s.so'

	v.LINKFLAGS_cxxstlib  = []
	v.cxxstlib_PATTERN    = 'lib%s.a'

	v.LINKFLAGS_MACBUNDLE = []
	v.CXXFLAGS_MACBUNDLE  = []
	v.macbundle_PATTERN   = '%s.bundle'


@conf
def find_ar(conf):
	"""Configuration helper used by C/C++ tools to enable the support for static libraries"""
	"""Finds the ar program and sets the default flags in ``conf.env.ARFLAGS``"""
	conf.find_program('%sgcc-ar'% conf.options.cross, var='AR')
	conf.add_os_flags('ARFLAGS')
	if not conf.env.ARFLAGS:
		conf.env.ARFLAGS = ['rcs']


@conf
def find_gas(conf):
        """
        Find the programs gas/as/gcc and set the variable *AS*
        """
        cross_gcc = '%sgcc' % conf.options.cross
        cross_gas = '%sgas' % conf.options.cross
        conf.find_program([cross_gcc, cross_gas], var='AS')
        conf.env.AS_TGT_F = ['-c', '-o']
        conf.env.ASLNK_TGT_F = ['-o']
        conf.find_ar()
        conf.load('asm')


def options(conf):
	"""
	Configure option for gxx
	"""
	conf.add_option('--cross', default='arm-none-eabi-',
		help='Cross compiler prefix, e.g. arm-none-eabi-')


def configure(conf):
	"""
	Configuration for gcc
	"""
	conf.find_gcc()
	conf.find_ar()
	conf.gcc_common_flags()
	conf.cc_load_tools()
	conf.cc_add_flags()
	#conf.link_add_flags()

	conf.find_gxx()
	#conf.find_ar()
	conf.gxx_common_flags()
	conf.cxx_load_tools()
	conf.cxx_add_flags()
	conf.link_add_flags()
	conf.find_gas()

