#!/usr/bin/env python

from pygas import *

import random
import sys
import operator

def main(argv):
  try:
    num_samples = int( argv[1] )
  except IndexError:
    print "Usage: ./%s <num_samples>"
    exit(1)

  my_num_samples = num_samples / THREADS # assume num_samples % THREADS = 0
  my_samples_in_circle = 0

  for sample_id in range(my_num_samples):
    x,y = random.random(),random.random()
    if x**2 + y**2 <= 1:
      my_samples_in_circle += 1

  samples_in_circle = coll.reduce(my_samples_in_circle, operator.add, to=0)

  if MYTHREAD == 0:
    pi_approx = 4.0 * samples_in_circle / num_samples
    print "Found pi: %f" % pi_approx

if __name__ == "__main__":
  main(sys.argv)
