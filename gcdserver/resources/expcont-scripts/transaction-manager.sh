#!/bin/bash

GCDSERVER_BUILD=$HOME/gcd-update/gcdserver-release
$GCDSERVER_BUILD/env-shell.sh $GCDSERVER_BUILD/gcdserver/resources/TransactionManager.py "$@"