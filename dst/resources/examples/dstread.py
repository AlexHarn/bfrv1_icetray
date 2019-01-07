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
    y, m, d = date.split("-")
    return "%s%s" % (m,d)
    
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
    print (d)
    start = format_dir(d['start'])
    stop = format_dir(d['stop'])
    r = []
    r.append(start)
    if start != stop:
        r.append(stop)

    return r


usage = "usage: %prog [options] inputfile"
parser = OptionParser(usage)

parser.add_option("-o", "--outfile",default="dst.root",
                  dest="OUTFILE", help="Write output to OUTFILE (.root, .hdf5 format)")
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

(options,args) = parser.parse_args()

if options.DSTVERSION not in ("dst13", "dst16"):
    raise RuntimeError("unsupported DST version %s. dst13 or dst16" % options.DSTVERSION)
if options.DSTVERSION == 'dst13':
   from icecube.dst import ExtractDST13
else:
   from icecube.dst import ExtractDST

# Take the input file from the command line arguments
if options.RUN and options.DIR:
    import glob
    #globlist = glob.glob(options.DIR+"/PFFilt_PhysicsTrig_PhysicsFiltering_Run%08u*.tar.bz2" % options.RUN)
    prefix = options.DIR+"/PFFilt_PhysicsFiltering_Run%08u_Subrun00000000_" % options.RUN
    globlist = glob.glob(prefix+"*.tar.bz2")
    globlist.sort()
    
    chunksize = len(globlist)/options.NCHUNK

    start = chunksize*options.CHUNK
    end = chunksize*(options.CHUNK+1)

    if end < len(globlist) and end > len(globlist)+chunksize: 
        end = len(globlist)

    print "processing files" , start, "to", end
    i3filelist = globlist[start:end]
    print i3filelist

    FileList = [options.GCDFILE]+i3filelist
elif options.RUN:
	dirs = getdirs(options.RUN)
	startday = dirs[0]
	endday = dirs[0]
	if len(dirs) > 1:
		endday = dirs[1] 
	days = []
	day = int(startday)
	print (int(endday))
	print (day < int(endday))
	while day % 100 <= 31 and day <= int(endday):
		days.append("%04d" % day)
		day +=1 
		if day % 100 > 31:
			day = 100*(day/100 +1) +1
		print day 
	print (days)
	   
	os._exit(0)
else:
    FileList = [options.GCDFILE]+options.INFILELIST
print(FileList)
dstfile = options.OUTFILE

tray = I3Tray()


tray.AddModule("I3Reader","i3reader")(
    ("FilenameList", FileList),
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
