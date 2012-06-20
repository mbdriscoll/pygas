#!/usr/bin/env bash

set -ex

export DATE=`date +%s`
export OPT='-g'
python setup.py build_ext --inplace --debug


mpirun -n 4 -errfile-pattern=test-$DATE-%r.err -outfile-pattern=test-$DATE-%r.out nosetests -v
