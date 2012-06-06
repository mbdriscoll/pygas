#!/usr/bin/env python

#from pygas import *
from random import random

import sys

def main(argv):
  try:
    num_samples = int( argv[1] )
  except IndexError:
    print "Usage: ./%s <num_samples>"
    exit(1)

  samples_inside_circle = 0.0
  for sample_id in range(num_samples):
    x,y = random(),random()
    if x**2 + y**2 <= 1:
      samples_inside_circle += 1.0

  sample_ratio = 4*samples_inside_circle / num_samples
  print "Found ratio: %f" % sample_ratio

if __name__ == "__main__":
  main(sys.argv)
