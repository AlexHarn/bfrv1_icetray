#!/bin/bash

if [ "$#" -ne 3 ]; then
  echo "Usage: vemcal-update.sh -r <run_number> vemcal_file.xml"
  exit
fi

GCDSERVER_BUILD=$HOME/gcd-update/gcdserver-release
$GCDSERVER_BUILD/env-shell.sh python $GCDSERVER_BUILD/lib/icecube/gcdserver/VEMCalImport.py "$@"