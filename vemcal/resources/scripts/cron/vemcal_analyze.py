#!/usr/bin/env python

import os
import sys
import glob
import re
import MySQLdb
import datetime
from optparse import OptionParser
from vemcal_I3Db_tools import *
from vemcal_history import *
from vemcal_graphs import * 


# Define arguments and default values
usage = "usage: %prog [options]"
parser = OptionParser(usage)
parser.add_option("-b", "--baseDir", default="/home/vemcal/data",
                                     dest="BASEDIR", help="VEMCal base directory")
parser.add_option("-d", "--deltaT",  default="7",
                                     dest="DELTAT", help="Calibration time range [days]")

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
if options.BASEDIR:
    baseDir = options.BASEDIR
else:
    print "Please specify base directory with option \"-b\"!"
    sys.exit()

if options.DELTAT:
    calPeriod = datetime.timedelta(days=int(options.DELTAT))
else:
    print "Please specify time range with option \"-d\"!"
    sys.exit()


# Connect to I3Db
i3Db = MySQLdb.connect(
    host="dbs2.icecube.wisc.edu",
    db="I3OmDb",
    user="www",
    passwd="")
cursor = i3Db.cursor()


# Find last calibration date
dateList = []
datePattern= re.compile('_(\d{4})-(\d{2})-(\d{2})_(\d{2})(\d{2})(\d{2})')
for path in glob.glob("%s/[0-9][0-9][0-9][0-9]/VEMCal/[0-9][0-9][0-9][0-9]/VEM_calibration_*.xml" % baseDir):
    match = datePattern.search(path)
    date = datetime.datetime(int(match.group(1)), int(match.group(2)), int(match.group(3)),
                             int(match.group(4)), int(match.group(5)), int(match.group(6)))
    dateList.append(date)

dateList.sort()
#for date in dateList:
#    print date

firstDate = datetime.datetime(2009,06,25)
if(len(dateList)>0):
    print "Found last VEMCal from:", dateList[-1]
    firstDate = dateList[-1] + calPeriod  # Next calibration starts "calPeriod" days after the last one
else:
    print "No previous VEMCals were found. Setting start date to:", firstDate

# Get next calibration date after the first date
calDate = getNextCalDate(cursor, firstDate)
if(calDate!=None):
    if(firstDate + calPeriod > calDate):
        firstDate = calDate

# Get the first run after first date
# and update first date to the start date of the run
firstRun  = getFirstRunAfter(cursor, firstDate)
if(firstRun==None):
    print "No run found after", firstDate
    sys.exit()
firstDate = getStartTime(cursor, firstRun)


# Get next DOMCal date after the last VEMCal date
lastDate = firstDate + calPeriod
calDate = getNextCalDate(cursor, lastDate)
if(calDate!=None):
    if(lastDate + calPeriod > calDate):
        lastDate = calDate

# Determine last run
lastRun = getFirstRunAfter(cursor, lastDate)
if(lastRun==None):
    print "No run found after", lastDate
    sys.exit()
lastRun = lastRun-1

print "Next calibration period:"
print "VEMCal start:", firstDate, ", runID =", firstRun
print "VEMCal stop :", lastDate, ", runID =", lastRun

# Close the database here since
# it is not needed anymore
cursor.close()
i3Db.close()


# Loop over the VEMCalData files, select the files
# withing the runID range and build the vemcal-analyze command
fileList = glob.glob("%s/[0-9][0-9][0-9][0-9]/RawHist/[0-9][0-9][0-9][0-9]/IceTop_VEMCalData_Run*.root" % baseDir)
fileList.sort()

# Extract run ID from very last file
runPattern  = re.compile('_Run(\d{8})_')
match = runPattern.search(fileList[-1])
runID = int(match.group(1))
if(runID<lastRun):
    print "Not all the data is available yet. Will try it later again."
    sys.exit()

# Create and change directory
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
        #print runID, path

print "Analyzing ..."
cmd = cmd + " > vemcal.log"
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

# Rename log file

# ...

# Generate history rootfile
os.chdir(outDir)
fileList = glob.glob("VEM_calibration_*.xml")
if(len(fileList)==1):
    print ""
    print "Generating history rootfile ..."
    generateHistory(fileList[0], runList)

#
# ---------- Plotting --------------
#
createGraphs(outDir)

createHistoryGraphs(firstDate.year, baseDir)

createRunningHistoryGraphs(52, baseDir)
