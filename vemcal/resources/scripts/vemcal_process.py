#!/usr/bin/python

import os
import re
import glob
import datetime
from optparse import OptionParser
from I3Tray import *


def runIceTray(workDir):
    load("libdataio")
    load("libvemcal")

    fileList = glob.glob("%s/*.i3" % workDir)
    fileList.sort()
    
    tray = I3Tray()

    tray.AddModule("I3Reader","reader")(
        ("FileNameList", fileList),
        )

    tray.AddModule("I3VEMCalHistWriter","write")(
        ("WorkDir", workDir),
        )

    tray.AddModule("TrashCan","trash")
    tray.Execute()
    tray.Finish()



#---------------------------------------------
#              Main script
#---------------------------------------------


# Define arguments and default values
usage = "usage: %prog [options]"
parser = OptionParser(usage)
parser.add_option("-r", "--runID", default="0",
                  dest="runID", help="Run number")
parser.add_option("-i", "--inBaseDir", default="/data/exp/IceCube",
                  dest="inBaseDir", help="Input base directory")
parser.add_option("-o", "--outDir", default=".",
                  dest="outDir", help="Output directory")
parser.add_option("-t", "--tmpDir", default=".",
                  dest="tmpDir", help="Scratch directory")

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
if options.runID:
    runID = int(options.runID)
else:
    print "Please specify run number with option \"-r\"!"
    sys.exit()

if options.inBaseDir:
    inBaseDir = options.inBaseDir
else:
    print "Please specify input base directory with option \"-i\"!"
    sys.exit()

if options.outDir:
    outDir = options.outDir
else:
    print "Please specify output directory with option \"-o\"!"
    sys.exit()

if options.tmpDir:
    tmpDir = options.tmpDir
else:
    print "Please specify temporary directory with option \"-t\"!"
    sys.exit()

    
# Read temporary directory from _CONDOR_SCRATCH_DIR
if tmpDir=="." and os.environ.has_key('_CONDOR_SCRATCH_DIR'):
    tmpDir=os.environ['_CONDOR_SCRATCH_DIR']
    tmpDir = os.path.abspath(tmpDir)
    

# Check runID
if runID <= 0:
    print "ERROR: Invalid run number (runID=%d)!" % runID
    sys.exit()

# Check directories
if not os.path.exists(inBaseDir):
    print "ERROR: Input directory \"%s\" doesn't exist!" % inBaseDir
    sys.exit()

if not os.path.exists(outDir):
    print "ERROR: Output directory \"%s\" doesn't exist!" % outDir
    sys.exit()

if not os.path.exists(tmpDir):
    print "ERROR: Temporary directory \"%s\" doesn't exist!" % tmpDir
    sys.exit()


print "--------------------------------------------"
print "Starting job - %s" % datetime.datetime.today()
print "--------------------------------------------"
print "Run Number          : %d" % runID
print "Input base directory: %s" % inBaseDir
print "Output directory    : %s" % outDir
print "Temporary directory : %s" % tmpDir

# Copy and unpack all files into the TMPDIR directory
fileList = glob.glob("%s/*/*/*/*/PFFilt_PhysicsFiltering_Run%08d_Subrun00000000_*.tar.bz2" % (inBaseDir, runID))
fileList.sort()

maxFiles=25;
fileCount=0;
partNumber=0;

#Clean tmp dir just to be sure
os.system("rm -rf %s/*" % tmpDir)

print ""
print "Untaring files:"
for file in fileList:
    fileCount = fileCount + 1
    fileName  = os.path.basename(file)
    print "%s ..." % fileName
    os.system("tar -jxf %s --directory=%s" % (file, tmpDir))
    xmlFile = fileName.replace(".tar.gz", ".meta.xml")
    os.system("rm -rf %s/%s" % (tmpDir, xmlFile))
    
    if(fileCount%maxFiles==0 or fileCount==len(fileList)):
        print ""
        print "Starting IceTray (%d) - %s" %  (partNumber, datetime.datetime.today())
        print ""

        # Perform the IceTray processing
        runIceTray(tmpDir)

        print ""
        print "Finished IceTray (%d) - %s" %  (partNumber, datetime.datetime.today())
        print ""
        
        # Rename rootfile and move it to outDir
        rootFiles = glob.glob("%s/*.root" % tmpDir)
        if(len(rootFiles)==1):
            newName=rootFiles[0].replace(".root","_part%04d.root" % partNumber)
            os.rename(rootFiles[0], newName)
            os.system("mv %s %s" % (newName, outDir))
        else:
            print "ERROR: found no or more than one rootfiles:"
            print rootFiles
            print ""
        
        # Increment part number
        partNumber = partNumber + 1
        
        # Cleanup tmpDir
        os.system("rm -rf %s/*" % tmpDir)
        
print ""
print "--------------------------------------------"
print "Finished job - %s" % datetime.datetime.today()
print "--------------------------------------------"
