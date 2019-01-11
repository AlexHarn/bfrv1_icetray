#!/usr/bin/python

import sys
import os
import re
import glob
import datetime
from optparse import OptionParser

# Define arguments
usage = "usage: %prog [options]"
parser = OptionParser(usage)
parser.add_option("-b", "--minID", default="0",
                  dest="minID", help="Minimum run number")
parser.add_option("-e", "--maxID", default="0",
                  dest="maxID", help="Maximum run number")
parser.add_option("-i", "--i3build", default="/home/vemcal/offline-V02-02-04/build_RHEL4-x86_64",
                  dest="i3build", help="IceTray build directory")
parser.add_option("-s", "--srcdir", default="/data/exp/IceCube/2009/filtered/PFFilt",
                  dest="srcdir", help="Source directory")
parser.add_option("-o", "--basedir", default="/home/vemcal/data/2009/RawHist",
                  dest="basedir", help="Output base directory")

# Parse cmd line args
# Bail out if anything is not understood
(options,args) = parser.parse_args()
if len(args) != 0:
    message = "Got undefined options: "
    for a in args:
        message += a
        message += " "
        parser.error(message)

# Get and check the options
if options.minID:
    minID = int(options.minID)
else:
    print "Please specify minimum run number with option \"-a\"!"
    sys.exit(-1)
                                                        
if options.maxID:
    maxID = int(options.maxID)
else:
    print "Please specify maximum run number with option \"-e\"!"
    sys.exit(-1)

if options.i3build:
    i3BuildDir = options.i3build
else:
    print "Please specify the IceTray build directory with option \"-i\"!"
    sys.exit(-1)

if options.srcdir:
    srcDir = options.srcdir
else:
    print "Please specify the source directory with option \"-s\"!"
    sys.exit(-1)

if options.basedir:
    baseDir = options.basedir
else:
    print "Please specify the output base directory with option \"-o\"!"
    sys.exit(-1)

# Get list of files
fileList = glob.glob(srcDir + "/*/PFFilt_PhysicsTrig_PhysicsFiltering_Run*_Subrun00000000_00000000.tar.gz")
fileList.sort()

# Remove last two days to avoid 
# that the files are not complete
lastDir = os.path.dirname(fileList[-1])
while (os.path.dirname(fileList[-1]) == lastDir):
    fileList.pop()
lastDir = os.path.dirname(fileList[-1])
while (os.path.dirname(fileList[-1]) == lastDir):
    fileList.pop()


runPattern=re.compile('_Run(\d{8})_')
for file in fileList:
    # Parse run number
    match = runPattern.search(file)
    runID = int(match.group(1))

    # Change directory to base directory
    os.chdir(baseDir)
    
    if(runID>=minID and runID<=maxID) or (minID<=0 and maxID<=0):
        dirName = os.path.basename(os.path.dirname(file))
        outDir = baseDir + "/" + dirName
        
        # Create output directory if it doesn't exist yet
        if not os.path.exists(outDir):
            os.mkdir(outDir)
        os.chdir(outDir)
        
        # If file doesn't exist yet submit the job
        if(len(glob.glob(("*_Run%08d*.root" % runID)))==0):
            os.system('pwd')
            #pbsCmd = "qsub -N r%d %s/vemcal/resources/scripts/pbs/vemcal_process.py" % (runID, i3BuildDir)
            pbsCmd = "qsub -N r%d -v RUN_ID=%s,OUT_DIR=%s,BUILD_DIR=%s %s/vemcal/resources/scripts/cron/vemcal_process.py" % (runID, runID, outDir, i3BuildDir, i3BuildDir)
            print ""
            print "Submitting Run%08d at %s" % (runID, datetime.datetime.today())
            print pbsCmd
            os.system(pbsCmd)
            

