#!/usr/bin/env bash

set -ex

export DATE=`date +%s`
export OPT='-g'
python setup.py build_ext --inplace --debug

gasnetrun_ibv -n 4 nosetests -v
