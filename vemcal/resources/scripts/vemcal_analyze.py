#!/usr/bin/env python

import os
import sys
import glob
import re
import datetime
from optparse import OptionParser
from vemcal_graphs import * 

firstDate = datetime.datetime(2019,9,15)
lastDate = datetime.datetime(2019,9,25)
firstRun =133063
lastRun = 133092

print "Next calibration period:"
print "VEMCal start:", firstDate, ", runID =", firstRun
print "VEMCal stop :", lastDate, ", runID =", lastRun

baseDir = "/home/vemcal/data"
fileList = glob.glob("%s/[0-9][0-9][0-9][0-9]/RawHist/[0-9][0-9][0-9][0-9]/IceTop_VEMCalData_Run*.root" % baseDir)
fileList.sort()

# Extract run ID from very last file
runPattern  = re.compile('_Run(\d{8})_')
match = runPattern.search(fileList[-1])
runID = int(match.group(1))
if(runID<lastRun):
    print "Not all the data is available yet. Will try it later again."
    sys.exit()

# Create and change out directory
outDir = "%s/%s/VEMCal/%s" % (baseDir, firstDate.strftime("%Y"), firstDate.strftime("%m%d"))
if not os.path.exists(outDir):
    os.makedirs(outDir)
os.chdir(outDir)


cmd = "vemcal-analyze"
runList = []
for path in fileList:
    match = runPattern.search(path)
    runID = int(match.group(1))
    
    if runID>=firstRun and runID<=lastRun:
        cmd = cmd + " " + path
        runList.append(runID)
        print runID, path

print "Analyzing ..."
cmd = cmd + " &> vemcal.log"
#print cmd
os.system(cmd)

# Check for xml file
vemList = glob.glob(outDir + "/VEM_calibration_*.xml")
if(len(vemList)==1):
    if vemList[0].find("_failed")>=0:
        print "VEM calibration FAILED!"
    else:
        print "VEM calibration SUCCEEDED!"
else:
    print "VEM calibration FAILED!"
    sys.exit()

os.chdir(outDir)
createGraphs(outDir)

