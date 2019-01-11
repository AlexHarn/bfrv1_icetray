#!/usr/bin/env python
from optparse import OptionParser

types=["NuMu", "NuE", "NuTau", "Corsika"]

parser=OptionParser()
parser.add_option("-t", "--type",
    dest="TYPE", help="specity the type of simulation")
parser.add_option("-y", "--year", default=2012,
    dest="YEAR", help="specify detector season/year: 2012, 2013")
parser.add_option("-s", "--dataset", type="int",
    dest="DATASET", help="dataset number which should be processed")
parser.add_option("-d", "--dag", default="",
    dest="DAG", help="specify dagman file")
parser.add_option("--submit", action="store_true",
    dest="SUBMIT", help="automatically submit dag")
parser.add_option("-m", "--maxnum", default=0, type="int",
    dest="MAXNUM", help="maximum number in 1k to process")
parser.add_option("--maxjobs", default=400, type="int",
    dest="MAXJOBS", help="maximum number of jobs submitted to npx4")
parser.add_option("--reprocess", action="store_true",
    dest="REPROCESS", help="reprocess successfully finished files")
parser.add_option("--test", action="store_true",
    dest="TEST", help="only process 100 test files")
(options, args)=parser.parse_args()

type=options.TYPE
if not type in types:
    print "type {0} unknown.  use a type from:".format(type), types
    raise NameError
season=int(options.YEAR)
dataset=options.DATASET
dag=options.DAG
try:
    dag_file=open(dag, "w")
except IOError as e:
    print "I/O error({0}): {1}".format(e.errno, e.strerror)
    raise
submit=options.SUBMIT
maxjobs=options.MAXJOBS
maxnum=options.MAXNUM
reprocess=options.REPROCESS
is_test=options.TEST
if submit:
    import socket
    if socket.gethostname()!="submitter.icecube.wisc.edu":
        print "Need to be on submitter to run the level3!"
        exit(-1)
    import os
    if os.environ["I3_SRC"]==None or not os.path.exists(os.environ["I3_SRC"]+"/level3-filter-muon"):
        print "Need to load the level3 metaproject"
        exit(-1)

if type=="NuMu" or type=="NuE" or type=="NuTau":
    from builddag import build_nugen_dag
    for num in range(0, maxnum+1):
        build_nugen_dag(dag_file, season, dataset, type, num, reprocess, is_test)

if type=="Corsika":
    from builddag import build_corsika_dag
    for num in range(0, maxnum+1):
        build_corsika_dag(dag_file, season, dataset, num, reprocess, is_test)
dag_file.close()
if submit:
    os.system("condor_submit_dag -maxjobs {maxjobs} {dag}".format(**locals()))

