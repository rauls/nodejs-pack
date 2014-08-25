import Options
from os import unlink, symlink, popen
from os.path import exists 

srcdir = '.'
blddir = 'build'
VERSION = '0.1.0'

def set_options(opt):
  opt.tool_options('compiler_cxx')

def configure(conf):
  conf.check_tool('compiler_cxx')
  conf.check_tool('node_addon')
  conf.env.append_unique('CXXFLAGS', ['-Wall', '-O3'])
  conf.env['LIB_PACK'] = 'hipack'

def build(bld):
  obj = bld.new_task_gen('cxx', 'shlib', 'node_addon')
  obj.cxxflags = ["-I../deps", "-g", "-Wall"]
  obj.target = "hipack"
  obj.source = 'pack.cc'
  