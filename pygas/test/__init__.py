# This module contains tests for pygas.

import subprocess, sys, unittest

def distributed(nodes=1):
  def fxn_wrapper(fn):
    def wrapped(self=None):
      N = "%d" % nodes
      interpreter = "%s" % sys.executable

      output = subprocess.check_output(
        ["mpirun", "-n", N, interpreter, "-c", "\"import dinner\""],
        shell=True
      )

      return lambda arg: exitcode
    return wrapped
  return fxn_wrapper
