#!/bin/bash

if [ "$#" -ne 3 ]; then
  echo "Usage: noiserate-update.sh -r <run_number> noiserate_file.json"
  exit
fi

GCDSERVER_BUILD=$HOME/gcd-update/gcdserver-release
$GCDSERVER_BUILD/env-shell.sh python $GCDSERVER_BUILD/lib/icecube/gcdserver/NoiseRateImport.py "$@"