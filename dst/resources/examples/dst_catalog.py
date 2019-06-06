#!/usr/bin/env python
#
#
import os,sys
import os.path 
from os.path import expandvars

from optparse import OptionParser


import glob
import json
import urllib
import urllib2
import os,sys


def format_dir(s):
    date = s.split()[0]
    y, m, d = map(int,date.split("-"))
    return "%02u%02u" % (m,d)

def format_date(s):
    date = s.split()[0]
    y, m, d = map(int,date.split("-"))
    return "%04u-%02u-%02u" % (y,m,d)
    
def getgrl(start,end):
    url = 'https://live.icecube.wisc.edu/snapshot-export/'
    
    params = {'user': 'icecube', 
          'pass': 'skua', 
          'start_date': start,
          'end_date': end}
    if type(start) == int:
        params = {'user': 'icecube', 
          'pass': 'skua', 
          'start_run': start,
          'end_run': end}


    # Fetch the JSON data (hits the database)
    req = urllib2.Request(url, urllib.urlencode(params))
    response = urllib2.urlopen(req).read()

    # Parse JSON into a python dict and print some of it
    d = json.loads(response)
    goodruns = []
    for r in d['runs']:
        if r['good_i3']: goodruns.append(r)
    return goodruns


def getdirs(run):
    # Define the view you want to hit (in this case 'run_info')
    url = 'https://live.icecube.wisc.edu/run_info/'

    # Define parameters to pass to the view. Keywords are different for each view,
    # except for 'user' and 'pass' which are mandatory for all views.
    params = {'user': 'icecube', 
          'pass': 'skua', 
          'run_number': run}



    # Fetch the JSON data (hits the database)
    req = urllib2.Request(url, urllib.urlencode(params))
    response = urllib2.urlopen(req).read()

    # Parse JSON into a python dict and print some of it
    d = json.loads(response)
    start = format_dir(d['start'])
    stop = format_dir(d['stop'])
    r = []
    r.append("%s"%start)
    if start != stop:
        r.append("%s"%stop)
    return r,format_date(d['start'])


if __name__ == "__main__":
    usage = "usage: %prog [options]"
    parser = OptionParser(usage)

    parser.add_option("-p", "--procnum",default=0,type=int,
              dest="PROCNUM", help="Job number")
    parser.add_option("-o", "--outdir",default="./",
              dest="OUTDIR", help="Write output to OUTDIR")
    parser.add_option("-f", "--format",default=".hdf5",
              dest="FORMAT", help="Write output format (.root, .hdf5)")
    parser.add_option("-g", "--gcd",default="auto",
              dest="GCDFILE", help="Read geometry from GCDFILE (.i3{.gz} format).")
    parser.add_option("-d", "--dir",default="/data/exp/IceCube/YEAR/filtered/PFFilt",
              dest="DIR", help="Directory")
    parser.add_option("-c", "--chunk-size",default=1,type=int,
              dest="CHUNKSIZE", help="Chunk of data to process")
    parser.add_option("-v", "--dst-version",default="dst16",
              dest="DSTVERSION", help="DST version to read (dst13,dst16)")
    parser.add_option("-s", "--start-run",type=int, default=0,
              dest="START", help="Start run")
    parser.add_option("-e", "--end-run",type=int, default=0,
              dest="END", help="End run")
    parser.add_option("-u", "--url", default="gsiftp://gridftp.icecube.wisc.edu",
              dest="URL", help="GridFTP URL")
    parser.add_option("-j", "--json",default="dst_catalog.json",
              dest="JSON", help="JSON catalog file")

    (options,args) = parser.parse_args()

    # Take the input file from the command line arguments
    runs = getgrl(options.START ,options.END)

    tasks = []
    current_task = -1
    current_subrun = 0
    filecount = 0
    gcdfile = None
    dstfile = options.OUTDIR+"/ic86_simpleDST_%(date)s_run%(run)s_part%(subrun)02u%(format)s" 

    if options.DIR:
        for run in runs:
            current_subrun = 0
            dirs,date = getdirs(run['run'])
            globlist = []
            year,month,day = date.split('-')
            for d in dirs:
                prefix = options.DIR.replace('YEAR',year)+"/"+d
                prefix += "/PFFilt_PhysicsFiltering_Run%08d_Subrun00000000_" % run['run']
                globlist.extend(glob.glob(prefix+"*.tar.bz2"))
            globlist.sort()

            gcdprefix = options.DIR.replace('YEAR',year)+"/*"
            gcdprefix = gcdprefix.replace("PFFilt","level2/OfflinePreChecks/DataFiles/")
            gcdprefix += "/Level2_IC86.*_data_Run%08u_*.i3.zst" % run['run']
            gcdfile = glob.glob(gcdprefix)[0]
            dstdict = {"year":year,"month":month,"day":day,"date":date,"run":run['run'],"subrun":current_subrun,"format":options.FORMAT}
            print ("Run:%(run)s -- date: %(year)s-%(month)s-%(day)s" %dstdict) 
            tasks.append({"files":[],"run":run['run'],"gcd":options.URL+gcdfile,"dstfile":dstfile%dstdict,"dstversion":options.DSTVERSION})
            current_task += 1

            current_file_in_run = 0
            current_file_in_chunk = 0
            for f in globlist:
                tasks[current_task]['files'].append(options.URL+f)
                current_file_in_run += 1
                current_file_in_chunk += 1
                filecount += 1

                if current_file_in_chunk >= options.CHUNKSIZE and current_file_in_run+1 < len(globlist):
                    current_subrun += 1
                    current_task += 1
                    current_file_in_chunk = 0
                    dstdict['subrun']  = current_subrun
                    tasks.append({"files":[],"run":run['run'],"gcd":options.URL+gcdfile,"dstfile":dstfile%dstdict,"dstversion":options.DSTVERSION})

    print ("%u files, %u tasks" % (filecount,len(tasks)) )
    t_count = 0
    print ("Each task has %u files except for the following:" % options.CHUNKSIZE)
    for t in tasks:
       if len(t['files']) != options.CHUNKSIZE:
          print ("\ttask %u for run %u has %u files" % (t_count,t['run'],len(t['files'])) )
       t_count += 1
    
    json.dump({"tasks":tasks},open(options.JSON,'w'))

