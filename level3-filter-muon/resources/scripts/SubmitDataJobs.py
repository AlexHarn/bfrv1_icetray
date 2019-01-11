#!/usr/bin/env python
import os
import time
import urllib
import numpy as np
from datetime import datetime
from builddag import build_data_dag
from optparse import OptionParser
test_runs={}
test_runs[2012]=[120540, 120264, 120925, 121840, 121503]
test_runs[2013]=[122504, 122818, 123238, 123620, 124151]

modes=["test", "burn", "full"]
parser=OptionParser()
parser.add_option("-m", "--mode", default="test",
    dest="MODE", help="specify mode: test, burn, full")
parser.add_option("-y", "--year", default=2012,
    dest="YEAR", help="specify detector season/year: 2012, 2013")
parser.add_option("-d", "--dag", default="",
    dest="DAG", help="specify dagman file")
parser.add_option("-s", "--submit", action="store_true",
    dest="SUBMIT", help="automatically submit dag")
parser.add_option("--maxjobs", default=400, type="int",
    dest="MAXJOBS", help="maximum number of jobs submitted to npx4")
parser.add_option("-u", "--update", action="store_true",
    dest="UPDATE", help="download newest version of goodrunlist")
parser.add_option("--reprocess", action="store_true",
    dest="REPROCESS", help="reprocess successfully finished files")
(options, args)=parser.parse_args()

mode=options.MODE
if not mode in modes:
    print "mode {0} unknown.  use a mode from:".format(mode), modes
    raise NameError
dag=options.DAG
try:
    dag_file=open(dag, "w")
except IOError as e:
    print "I/O error({0}): {1}".format(e.errno, e.strerror)
    raise
season=int(options.YEAR)
reprocess=options.REPROCESS
submit=options.SUBMIT
maxjobs=options.MAXJOBS
if submit:
    import socket
    if socket.gethostname()!="npx4.icecube.wisc.edu":
        print "Need to be on npx4 to run the level3!"
        exit(-1)
    import os
    if os.environ["I3_SRC"]==None or not os.path.exists(os.environ["I3_SRC"]+"/level3-filter-muon"):
        print "Need to load the level3 metaproject"
        exit(-1)

grl="IC86_{s}_GoodRunInfo.txt".format(s=season)
if options.UPDATE or not os.path.isfile(grl):
    urllib.urlretrieve("http://convey.icecube.wisc.edu/data/exp/IceCube/{s}/filtered/level2/IC86_{s}_GoodRunInfo.txt".format(s=season), grl)
f_grl=open(grl)

runs=np.genfromtxt(f_grl, dtype=[("run", int), ("good_i3", int), ("good_it", int), ("livetime", int), ("active_strings", int), ("active_doms", int), ("dir", "|S44")], skip_header=2, usecols=(0, 1, 2, 3, 4, 5, 6))

total_livetime=0.
for run in runs:
    is_good=(run["good_i3"] or run["good_it"])
    livetime=run["livetime"]
    run_id=run["run"]
    year=str(run["dir"][18:22])
    date=str(run["dir"][-5:-1])

    if mode=="burn" and run_id[-1]!="0":
        continue

    if mode=="test" and not run_id in test_runs[season]:
        continue

    total_livetime+=livetime

    if is_good:
        build_data_dag(dag_file, season, year, date, run_id, reprocess)

dag_file.close()
if submit:
    os.system("condor_submit_dag -maxjobs {maxjobs} {dag}".format(**locals()))

print "Livetime for {mode} is {total_livetime}s".format(**locals())
