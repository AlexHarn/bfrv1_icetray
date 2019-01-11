#!/usr/bin/env python

# Copied from I3Live/util/Daemon.py

import os
import resource
import time
import sys

from icecube.gcdserver.OptionParser import GCDOptionParser
from icecube.gcdserver.MongoDB import getDB
from icecube.gcdserver.I3MS import db_receiver_context


def find_running_proc(proc):
    """
    Check the process table to determine of there is a process with
    string 'proc' in its command line.  If so, return the PID.  Otherwise,
    return None.
    """
    # Skip the PID of current process which may have the same name
    pids = [pid for pid in os.listdir('/proc') if
                              (pid.isdigit() and pid != str(os.getpid()))]
    for pid in pids:
        try:
            cmdline = open(os.path.join('/proc', pid, 'cmdline'), 'rb').read()
            if proc in cmdline:
                return pid
        except:
            pass


class Daemon(object):
    """
    Loosely follows Chad Schroeder's example at
    http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/278731
    """
    def Daemonize(self):
        "Method which actually sets up the calling program as a daemon"
        sys.stdout.flush()
        sys.stderr.flush()
        pid = os.fork()          # Can raise OSError
        if pid != 0:
            os._exit(0)  # Parent does a minimal exit
        os.setsid()      # Become session leader
        pid = os.fork()          # Fork again to avoid zombies
        if pid != 0:
            os._exit(0)
        os.chdir("/")            # Avoid unmount errors
        os.umask(0)

        # Close all fd's, ignoring ones that weren't open
        maxfd = resource.getrlimit(resource.RLIMIT_NOFILE)[1]
        if(maxfd == resource.RLIM_INFINITY):
            maxfd = 1024
        for fd in xrange(0, maxfd):
            try:
                os.close(fd)
            except OSError:
                pass

        # Redirect stdin, stdout, stderr to /dev/null
        os.open("/dev/null", os.O_RDWR)  # stdin
        os.dup2(0, 1)
        os.dup2(0, 2)    # stdout, stderr
        return


def main(dbhost, dbuser, dbpass, i3msHost):
    # Check if DBDaemon is already running
    pid = find_running_proc('DBDaemon')
    if pid:
        print "DBDaemon is already running (PID=%s). " % pid
        return 1
    Daemon().Daemonize()
    with db_receiver_context(getDB(dbhost, dbuser, dbpass), i3msHost):
        while True:
            time.sleep(1)


if __name__ == "__main__":
    parser = GCDOptionParser()
    (options, args) = parser.parse_args()
    if options.i3mshost is None:
        print "I3MS host not specified"
        parser.print_help()
        sys.exit(-1)
    main(options.dbhost, options.dbuser, options.dbpass, options.i3mshost)