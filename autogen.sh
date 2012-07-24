#!/bin/sh -ex

autoreconf --force --install --verbose
test -n "$NOCONFIGURE" || "./configure" "$@"
