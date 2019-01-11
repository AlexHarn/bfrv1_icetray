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
parser.add_option("-i", "--inBaseDir", default="/data/exp/IceCube",
                  dest="IBD", help="Input base directory")
parser.add_option("-o", "--outBaseDir", default="/home/vemcal/data",
                  dest="OBD", help="Output base directory")
parser.add_option("-b", "--i3BuildDir", default="/home/vemcal/offline-09-10-01/build_RHEL5-x86_64",
                  dest="I3D", help="IceTray build directory")

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
if options.IBD:
    inBaseDir = options.IBD
else:
    print "Please specify the input base directory with option \"-i\"!"
    sys.exit()

if options.OBD:
    outBaseDir = options.OBD
else:
    print "Please specify the output base directory with option \"-o\"!"
    sys.exit()
    
if options.I3D:
    i3BuildDir = options.I3D
else:
    print "Please specify the IceTray build directory with option \"-b\"!"
    sys.exit()


# Find last processed date
startDate = datetime.datetime(2009, 01, 01)
fileList = glob.glob("%s/[0-9][0-9][0-9][0-9]/RawHist/[0-9][0-9][0-9][0-9]/*" % outBaseDir)
fileList.sort()
if (len(fileList)>0):
    year = os.path.basename(os.path.dirname(os.path.dirname(os.path.dirname(fileList[-1]))))
    monthday = os.path.basename(os.path.dirname(fileList[-1]))
    startDate = datetime.datetime(int(year), int(monthday[0:2]), int(monthday[2:5]))
        
# Start one month before the last processed date for safety reasons
dateShift = datetime.timedelta(weeks=4)
startDate = startDate - dateShift

# If start date is before 2009-06-25 set it to this date.
# This is one day after the automatic muon calibration started working
if (startDate < datetime.datetime(2009, 06, 25)):
    startDate = datetime.datetime(2009, 06, 25)

#print startDate

# Get list of years
yearList = []
for dir in glob.glob(inBaseDir + "/[0-9][0-9][0-9][0-9]"):
    year = int(os.path.basename(dir))
    if(year>=startDate.year):
        yearList.append(year)
yearList.sort()

if len(yearList)<=0:
    print "No data found in datawarehouse!"
    sys.exit()

fileList = []
for year in yearList:
    tempList = glob.glob("%s/%d/filtered/PFFilt/[0-9][0-9][0-9][0-9]/PFFilt_PhysicsTrig_PhysicsFiltering_Run*_Subrun00000000_00000000.tar.gz" % (inBaseDir, year))
    tempList.sort()
    for file in tempList:
        monthday = os.path.basename(os.path.dirname(file))
        date = datetime.datetime(year, int(monthday[0:2]), int(monthday[2:5]))
        if(date >= startDate):
            fileList.append(file)


# Do not include the two most recent days
# to ensure that all files are complete
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
    
    # Extract year and date directories
    yearDir = os.path.basename(os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(file)))))
    dateDir = os.path.basename(os.path.dirname(file))
    
    # Create and change to output directory
    outDir = "%s/%s/RawHist/%s" % (outBaseDir, yearDir, dateDir)
    if not os.path.exists(outDir):
        os.makedirs(outDir)
    os.chdir(outDir)

    # If file doesn't exist yet submit the job
    if(len(glob.glob(("*_Run%08d*.root" % runID)))==0):
        # Generate condor submit-file
        subFile = "IceTop_VEMCalData_Run%08d.submit" % runID
        f = open(subFile, "w")
        f.write("universe     = vanilla\n")
        f.write("executable   = %s/vemcal/resources/scripts/cron/vemcal_process.py\n" % i3BuildDir)
        f.write("arguments    = --runID=%d --inBaseDir=%s --outDir=%s\n" % (runID, inBaseDir, outDir))
        f.write("getenv       = True\n")
        f.write("log          = IceTop_VEMCalData_Run%08d.log\n" % runID)
        f.write("output       = IceTop_VEMCalData_Run%08d.out\n" % runID)
        f.write("error        = IceTop_VEMCalData_Run%08d.err\n" % runID)
        f.write("notification = Error\n") # Only notify vemcal-user by email when error occurs
        f.write("queue\n")
        f.close()
            
        condorCmd = "%s/env-shell.sh condor_submit %s" % (i3BuildDir, subFile)
        print ""
        print "Submitting Run%08d from %s-%s-%s" % (runID, yearDir, dateDir[0:2], dateDir[2:5])
        os.system(condorCmd)
        
