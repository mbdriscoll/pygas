#!/bin/bash

set -ex

make html

ORIGDIR=`pwd`
TEMPDIR=`mktemp -d`
cd $TEMPDIR

  git clone https://github.com/mbdriscoll/pygas.git
  cd pygas

    git fetch origin gh-pages
    git checkout gh-pages
    cp -r $ORIGDIR/_build/html/* ./
    git add *
    git commit
    git push

cd $ORIGDIR
rm -rf $TEMPDIR
