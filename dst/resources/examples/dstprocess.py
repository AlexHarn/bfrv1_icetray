#!/usr/bin/env python
#
#
import os,sys
import json
import urllib
import urllib2
from optparse import OptionParser

from I3Tray import *
import os.path 
from os.path import expandvars
import icecube.icetray
from icecube import icetray, dataclasses, dataio, dst
import gridftp



if __name__ == "__main__":
	usage = "usage: %prog [options] inputfile"
	parser = OptionParser(usage)

	parser.add_option("-p", "--procnum",default=0,type=int,
			  dest="PROCNUM", help="Job number")
	parser.add_option("-x", "--extract-to-frame",action="store_true", default=False,
			  dest="X2FRAME", help="Extract DST to standar I3Reco objects in frame")
	parser.add_option("-c", "--cut",action="store_true", default=False,
			  dest="CUT", help="Apply cuts")
	parser.add_option("-j", "--json",default="dst_catalog.json",
			  dest="JSON", help="JSON catalog file")

	(options,args) = parser.parse_args()
	
	jsonfile = json.load(open(options.JSON,'r'))
	taskinfo = jsonfile['tasks'][options.PROCNUM]

	dstfile = os.path.basename(taskinfo['dstfile']).encode("utf8")
	dstdir = os.path.dirname(taskinfo['dstfile'])

	ftp = gridftp.GridFTP()
	filelist = map(lambda x:x.encode("utf8"),[taskinfo['gcd']]+taskinfo['files'])
	local_path = os.getcwd()
	local_filelist = map(lambda x:local_path+"/"+os.path.basename(x),filelist)
	for source,dest in zip(filelist,local_filelist):
	    print("downloading file %s" % source)
	    ftp.get(source,filename=dest)


	tray = I3Tray()


	tray.AddModule("I3Reader","i3reader")(
	    ("FileNameList", local_filelist)
	    )


	# You can write the dst to .root files
	tray.AddModule("QConverter","qify-dst")(
	    ("QKeys",["I3DST13","I3DST13Header","I3EventHeader"]),
	    ("WritePFrame", False)
	    )

	if taskinfo['dstversion'] == 'dst13':
	   from icecube.dst import ExtractDST13
	   tray.AddSegment(ExtractDST13,"extractdst",
		    dst_output_filename = dstfile,
		    extract_to_frame = options.X2FRAME,
		    dstname = "I3DST13")
	else:
	   from icecube.dst import ExtractDST
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

	print("uploading dstfile %(dstfile)s" % taskinfo)
	ftp.put(taskinfo['dstfile'],filename=local_path+"/"+dstfile)
