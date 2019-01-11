#!/bin/bash

if [ "$#" -ne 3 ]; then
  echo "Usage: snowdepth-update.sh -r <run_number> snowdepth_file.xml"
  exit
fi

GCDSERVER_BUILD=$HOME/gcd-update/gcdserver-release
$GCDSERVER_BUILD/env-shell.sh python $GCDSERVER_BUILD/lib/icecube/gcdserver/SnowDepthImport.py "$@"