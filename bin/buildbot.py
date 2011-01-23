#!/usr/bin/env python
# vim: set fileencoding=utf-8 :
# Andre Anjos <andre.anjos@idiap.ch>
# Fri 06 Aug 2010 08:39:26 CEST 

"""This script is used by Buildbot to build full torch releases. It knows how
to handle all construction phases. If you are looking for a way to build torch
locally, look at build.py or aliases "release.sh" and "debug.sh".

This program should be prefixed in a torch-enabled shell environment. You can
create such an environment by calling this program like this:

  <torch-root-installation>/bin/shell.py -x "build.py <command>"
"""

import os
import sys 
import time
import logging

# Imports our admin toolkit
sys.path.append(os.path.realpath(os.path.dirname(sys.argv[0])))
import adm

def parse_args():
  """Parses the command line input."""
  import optparse
  
  #some defaults
  actions = ('cmake', 'make_all', 'documentation', 'make_install', 'ctest', 
      'make_clean')
  build_types = ('release', 'debug') #default is #0
  build_blocks = ('all', 'cxx', 'python') #default is #0
  pwd = os.path.realpath(os.curdir)
  default_install_prefix = os.path.join(pwd, 'install')
  default_doc_prefix = os.path.join('share', 'doc')
  default_build_prefix = os.path.join(pwd, 'build')
  sources = os.path.dirname(os.path.dirname(os.path.realpath(sys.argv[0])))
  sources = os.path.join(sources, 'src')
  default_doxyfile = os.path.join(os.path.dirname(sources), 'doc', 'Doxyfile')

  #our gigantic list of options...
  parser = optparse.OptionParser(description=__doc__)
  parser.add_option("-a", "--action", type="string", action="store", 
      dest="action", default=actions[0], metavar="(%s)" % '|'.join(actions),
      help="what should I build (defaults to %default)",
      )
  parser.add_option("--build-type", type="string", action="store",
      dest="build_type", default=build_types[0],
      metavar="(%s)" % '|'.join(build_types),
      help="defines the build type (defaults to %default)",
      )
  parser.add_option("-B", "--build-block", type="string", action="store",
      dest="build_block", default=build_blocks[0],
      metavar="(%s)" % '|'.join(build_blocks),
      help="defines which libraries to build (defaults to %default)",
      )
  parser.add_option("--build-prefix", type="string", action="store",
      dest="build_prefix", default=default_build_prefix, metavar="DIR",
      help="base build directory (defaults to %default)",
      )
  parser.add_option("--install-prefix", type="string", action="store",
      dest="install_prefix", default=default_install_prefix,
      metavar="DIR", help="base install directory (defaults to %default)",
      )
  parser.add_option("--documentation-prefix", type="string", action="store",
      dest="doc_prefix", default=default_doc_prefix, metavar="DIR",
      help="base install directory for documentation (defaults to %default). note: if a relative directory is given, it is taken w.r.t. the install prefix.",
      )
  parser.add_option("--doxyfile", action="store", dest="doxyfile",
      default=default_doxyfile, metavar="FILE",
      help="path of a doxygen file to be used as configuration basis (defaults to %default)",
      )
  parser.add_option("-j", "--jobs", type="int", action="store", dest="jobs",
      default=1, metavar="INT(>=1)",
      help="where possible, try to launch several jobs at once (defaults to %default)",
      )
  parser.add_option("--static-linkage", action="store_true",
      dest="static_linkage", default=False,
      help="links executables with static libraries when possible",
      )
  parser.add_option("-d", "--debug-build", action="store_true",
      dest="debug_build", default=False,
      help="executes all stages with lots of verbose output",
      )
  parser.add_option("-V", "--version", action="store", dest="version",
      default="?.?", metavar="VERSION",
      help="if it makes sense, choose a version name that will be used to mark the project documentation, otherwise, leave it unassigned"
      )
  
  options, args = parser.parse_args()

  #some error checking
  if options.action not in actions:
    parser.error("option --action has to be one of %s" % ", ".join(actions))
  if options.build_type not in build_types:
    parser.error("option --build-type has to be one of %s" % ", ".join(build_types))
  if options.build_block not in build_blocks:
    parser.error("option --build-block has to be one of %s" % ", ".join(build_blocks))
  if options.jobs < 1:
    parser.error("option --jobs has to be equal or greater than 1")
  if options.debug_build and options.jobs > 1:
    logging.info("option --jobs will be reset to 1")
    options.jobs = 1
  if args:
    parser.error("this program does not accept positional arguments: %s" % args)

  #some fixing
  options.install_prefix = options.install_prefix.strip()
  if options.install_prefix and options.install_prefix[0] != os.path.sep:
    options.install_prefix = os.path.join(pwd, options.install_prefix) 
  options.build_prefix = options.build_prefix.strip()
  if options.build_prefix and options.build_prefix[0] != os.path.sep:
    options.build_prefix = os.path.join(pwd, options.build_prefix) 
  options.doc_prefix = options.doc_prefix.strip()

  options.platform = adm.build.platform(options)
  options.source_dir = sources

  #we suffix the platform on the build and installation directories
  options.build_prefix = os.path.join(options.build_prefix, options.platform)
  options.install_prefix = os.path.join(options.install_prefix, options.platform)

  if options.doc_prefix and options.doc_prefix[0] != os.path.sep:
    options.doc_prefix = os.path.join(options.install_prefix, options.doc_prefix)

  return options, args

if __name__ == '__main__':
  (options, args) = parse_args()

  if options.action == 'cmake': 
    adm.build.cmake(options)
    adm.build.dot(options)

  elif options.action == 'make_all':
    adm.build.make(options, 'all')
    adm.build.write_header(options)
  elif options.action == 'make_install': adm.build.make(options, 'install')
  elif options.action == 'documentation': adm.build.doxygen(options)
  elif options.action == 'ctest': adm.build.ctest(options)
  elif options.action == 'make_clean': adm.build.make(options, 'clean')

  sys.exit(0)