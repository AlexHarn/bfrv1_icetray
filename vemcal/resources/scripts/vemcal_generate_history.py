#!/usr/bin/python

import glob
import re
import os
import sys

from optparse import OptionParser
import xml.dom.minidom

# Append "cron" directory to python path
sys.path.append(sys.path[0] + "/cron")
from vemcal_history import generateHistory


# Define arguments and default values
usage = "usage: %prog [options]"
parser = OptionParser(usage)
parser.add_option("-f", "--vemFile", default="", dest="VEMFILE", help="VEMCal xml-file")

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
fileList = []
if options.VEMFILE:
    fileList = [os.path.abspath(options.VEMFILE)]
else:
    print "Please specify vemcal xml-file with option \"-f\"!"
    sys.exit()


#baseDir="/home/vemcal/data"
#fileList = glob.glob("%s/[0-9][0-9][0-9][0-9]/VEMCal/[0-9][0-9][0-9][0-9]/VEM_calibration_*.xml" % baseDir)
#fileList.sort()


for file in fileList:
    dom = xml.dom.minidom.parse(file)

    firstRunID = 0
    lastRunID = 0
    
    nodeList = dom.getElementsByTagName('FirstRun')
    if len(nodeList)==1:
        firstRunID = int(nodeList[0].firstChild.data)
        
    nodeList = dom.getElementsByTagName('LastRun')
    if len(nodeList)==1:
        lastRunID = int(nodeList[0].firstChild.data)

    runList = range(firstRunID, lastRunID)
    workDir = os.path.dirname(file)
    vemFile = os.path.basename(file)
    os.chdir(workDir)

    print "Generating history for %s ..." % vemFile 
    generateHistory(vemFile, runList)
