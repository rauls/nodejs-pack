/*
HiPack (C) Raul Sobon
*/


#ifndef PACK_H
#define PACK_H

#include <node.h>
#include <node_version.h>
#include <node_buffer.h>

using namespace v8;

#define NODEJS_MINIT_FUNCTION(_fname_)			void (_fname_) (Handle<Object> target);
#define NODEJS_FUNCTION(_fname_)			Handle<Value> (_fname_) (const Arguments& argv);

#endif /* PACK_H */



/*
WSCRIPT=

srcdir = '.'
blddir = 'build'
VERSION = '0.0.1'

def set_options(opt):
  opt.tool_options('compiler_cxx')

def configure(conf):
  conf.check_tool('compiler_cxx')
  conf.check_tool('node_addon')

def build(bld):
  obj = bld.new_task_gen('cxx', 'shlib', 'node_addon')
  obj.target = 'pack'
  obj.source = 'pack.cc'
  
  */