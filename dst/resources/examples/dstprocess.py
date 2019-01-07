#!/usr/bin/env python
#
#
import os,sys
from I3Tray import *
import os.path 
from os.path import expandvars
import icecube.icetray

from icecube import icetray, dataclasses, dataio, dst
from optparse import OptionParser


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
	usage = "usage: %prog [options] inputfile"
	parser = OptionParser(usage)

	parser.add_option("-p", "--procnum",default=0,type=int,
			  dest="PROCNUM", help="Job number")
	parser.add_option("-o", "--outdir",default="./",
			  dest="OUTDIR", help="Write output to OUTDIR")
	parser.add_option("-f", "--format",default=".root",
			  dest="FORMAT", help="Write output format (.root, .hdf5)")
	parser.add_option("-i", "--infile", action="append",
			  dest="INFILELIST", help="Read (.i3{.gz})")
	parser.add_option("-g", "--gcd",default="auto",
			  dest="GCDFILE", help="Read geometry from GCDFILE (.i3{.gz} format).")
	parser.add_option("-r", "--run",default=0,type=int,
			  dest="RUN", help="Run")
	parser.add_option("-d", "--dir",default="",
			  dest="DIR", help="Directory")
	parser.add_option("-u", "--chunk",default=0,type=int,
			  dest="CHUNK", help="Chunk of data to process")
	parser.add_option("-n", "--nchunks",default=1,type=int,
			  dest="NCHUNK", help="Number of chunks")
	parser.add_option("-v", "--dst-version",default="dst16",
			  dest="DSTVERSION", help="DST version to read (dst13,dst16)")
	parser.add_option("-x", "--extract-to-frame",action="store_true", default=False,
			  dest="X2FRAME", help="Extract DST to standar I3Reco objects in frame")
	parser.add_option("-c", "--cut",action="store_true", default=False,
			  dest="CUT", help="Apply cuts")
	parser.add_option("-s", "--start-run",type=int, default=0,
			  dest="START", help="Start run")
	parser.add_option("-e", "--end-run",type=int, default=0,
			  dest="END", help="End run")

	(options,args) = parser.parse_args()

	if options.DSTVERSION not in ("dst13", "dst16"):
	    raise RuntimeError("unsupported DST version %s. dst13 or dst16" % options.DSTVERSION)
	if options.DSTVERSION == 'dst13':
	   from icecube.dst import ExtractDST13
	else:
	   from icecube.dst import ExtractDST

	# Take the input file from the command line arguments
	if options.START > 0 and options.RUN == 0 and options.PROCNUM > -1:
	    if options.END < options.START: 
	        options.RUN = options.START
	    else:
	        runs = getgrl(options.START ,options.END)
	        NPROC = len(runs)*options.NCHUNK
	        run_index = options.PROCNUM/options.NCHUNK
	        options.RUN = runs[run_index]['run']
	    options.CHUNK = options.PROCNUM%options.NCHUNK

	if options.RUN and options.DIR:
	    import glob
	    #globlist = glob.glob(options.DIR+"/PFFilt_PhysicsTrig_PhysicsFiltering_Run%08u*.tar.bz2" % options.RUN)
	    dirs,date = getdirs(options.RUN)
	    globlist = []
	    year = date.split('-')[0]
	    for d in dirs:
	    	prefix = options.DIR.replace('YEAR',year)+"/"+d
	    	prefix += "/PFFilt_PhysicsFiltering_Run%08d_Subrun00000000_" % options.RUN
	    	globlist.extend(glob.glob(prefix+"*.tar.bz2"))
	    globlist.sort()

	    if options.GCDFILE == 'auto':
	    	gcdprefix = options.DIR.replace('YEAR',year)+"/*"
	    	gcdprefix = gcdprefix.replace("PFFilt","level2/OfflinePreChecks/DataFiles/")
	    	gcdprefix += "/Level2_IC86.*_data_Run%08u_*.i3.zst" % options.RUN
	    	options.GCDFILE = glob.glob(gcdprefix)[0]
            
	    
	    chunksize = len(globlist)/options.NCHUNK
	    print chunksize, len(globlist),options.NCHUNK, options.RUN

	    start = chunksize*options.CHUNK
	    end = chunksize*(options.CHUNK+1)

	    if end < len(globlist) and end > len(globlist)+chunksize: 
		end = len(globlist)

	    print "processing files" , start, "to", end
	    i3filelist = globlist[start:end]

	    FileList = [options.GCDFILE]+i3filelist
	else:
	    FileList = [options.GCDFILE]+options.INFILELIST
	dstfile = options.OUTDIR+"/ic86_simpleDST_%s_part%02u%s" %(date,options.NCHUNK,options.FORMAT)
	dstfile = "test.hdf5"

	tray = I3Tray()


	tray.AddModule("I3Reader","i3reader")(
	    ("FileNameList", FileList),
	    )


	# You can write the dst to .root files
	tray.AddModule("QConverter","qify-dst")(
	    ("QKeys",["I3DST13","I3DST13Header","I3EventHeader"]),
	    ("WritePFrame", False)
	    )


	if options.DSTVERSION == 'dst13':
	   tray.AddSegment(ExtractDST13,"extractdst",
		    dst_output_filename = dstfile,
		    extract_to_frame = options.X2FRAME,
		    dstname = "I3DST13")
	else:
	   tray.AddSegment(ExtractDST,"extractdst",
		    dst_output_filename = dstfile,
		    extract_to_frame = options.X2FRAME,
		    cut_data = options.CUT,
		    dstname = "I3DST16")


	if options.X2FRAME:
	   tray.AddModule("I3Writer","write",
	       filename=dstfile.replace('.root','.i3.bz2').replace('.hdf5','.i3.bz2'),
	       streams=[icecube.icetray.I3Frame.DAQ,icecube.icetray.I3Frame.Physics]
	   )

	# The usual trashcan-at-the-end idiom.
	#

	tray.Execute()
