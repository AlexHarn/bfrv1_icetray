#!/bin/bash

if [ "$#" -lt 3 ]; then
  echo "Usage: domcal-update.sh -r <run_number> domcal_file1.xml domcal_file2.xml ..."
  exit
fi

GCDSERVER_BUILD=$HOME/gcd-update/gcdserver-release
$GCDSERVER_BUILD/env-shell.sh python $GCDSERVER_BUILD/lib/icecube/gcdserver/DOMCalImport.py "$@"