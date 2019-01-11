#!/bin/bash

if [ "$#" -lt 1 ]; then
  echo "Usage: config-update.sh config_file.xml config_file2.xml ..."
  exit
fi

GCDSERVER_BUILD=$HOME/gcd-update/gcdserver-release
$GCDSERVER_BUILD/env-shell.sh python $GCDSERVER_BUILD/lib/icecube/gcdserver/ConfigImport.py "$@"