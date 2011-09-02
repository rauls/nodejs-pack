/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2011 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Rasmus Lerdorf <rasmus@lerdorf.on.ca>                        |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PACK_H
#define PACK_H

#include <node.h>
#include <node_version.h>
#include <node_buffer.h>

using namespace v8;

#define NODEJS_MINIT_FUNCTION(_fname_)			void (_fname_) (Handle<Object> target);
#define NODEJS_FUNCTION(_fname_)			Handle<Value> (_fname_) (const Arguments& argv);

extern "C" NODEJS_MINIT_FUNCTION(initpack)
extern "C" NODEJS_FUNCTION(pack);
extern "C" NODEJS_FUNCTION(unpack);
extern "C" NODEJS_FUNCTION(setdebug);

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