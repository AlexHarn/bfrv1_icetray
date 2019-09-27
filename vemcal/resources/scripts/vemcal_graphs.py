#!/usr/bin/python

import glob
import re
import os
import sys
import datetime

def createThumbNails(workDir=".", format="png"):
    # Delete old thumbnaild in workDir
    os.system("rm -f %s/*_thumb.%s" % (workDir, format))

    fileList = glob.glob("%s/*.png" % workDir)
    fileList.sort()
    for fileName in fileList:
        thumbName = fileName.replace('.png', '_thumb.png')
        cmd = "convert -depth 8 -resize 400x400 -colors 16 " + fileName + " " + thumbName
        #print cmd
        os.system(cmd)

        
def createGraphs(srcDir="."):
    fileList = glob.glob("%s/VEM_calibration_*.root" % srcDir)
    if(len(fileList)==1):
        # Generate output directory for graphs
        outDir = srcDir + "/plots"
        if not os.path.exists(outDir):
            os.makedirs(outDir)

        # Change to outDir and create graphs
        os.chdir(outDir)
        print "Generating plots ..."
        os.system("vemcal-plot %s" % fileList[0])

        print "Creating thumbnails ..."
        createThumbNails(outDir, "png")
    else:
        print "No VEMCal rootfile was found in %s! Didn't create graphs." % srcDir


def createHistoryGraphs(year, baseDir="/home/vemcal/data"):
    # Generate history plots
    histList = glob.glob("%s/%d/VEMCal/[0-1][0-9][0-3][0-9]/VEM_history_*.root" % (baseDir, year))
    histList.sort()
    
    if(len(histList)>0):
        # Create history directory if it doesn't exist yet
        histDir = "%s/%d/VEMCal/history" % (baseDir, year)
        if not os.path.exists(histDir):
            os.makedirs(histDir)

        # Remove old plots
        os.system("rm -f %s/*.png" % histDir) 

        # Assemble command 
        cmd = "vemcal-history-plot"
        for histFile in histList:
            cmd = cmd + " " + histFile
        #print cmd

        print "Generating history plots ..."
        os.chdir(histDir)
        os.system(cmd)

        print "Creating thumbnails ..."
        createThumbNails(histDir, "png")
    else:
        print "No history rootfiles found. Cannot create history graphs!"


def createRunningHistoryGraphs(numWeeks=52, baseDir="/home/vemcal/data"):
    # Generate running history plots
    
    fileList = glob.glob("%s/[0-9][0-9][0-9][0-9]/VEMCal/[0-1][0-9][0-3][0-9]/VEM_history_*.root" % baseDir)
    fileList.sort()
    
    if(len(fileList)>0):
        datePattern = re.compile('_(\d{4})-(\d{2})-(\d{2})_(\d{2})(\d{2})(\d{2})')
        match = datePattern.search(fileList[-1])
        lastDate = datetime.datetime(int(match.group(1)), int(match.group(2)), int(match.group(3)),
                                     int(match.group(4)), int(match.group(5)), int(match.group(6)))

        plotPeriod = datetime.timedelta(weeks=numWeeks)
        firstDate = lastDate - plotPeriod
        
        # Remove files outside of period
        histList = []
        for file in fileList:
            match = datePattern.search(file)
            date  = datetime.datetime(int(match.group(1)), int(match.group(2)), int(match.group(3)),
                                      int(match.group(4)), int(match.group(5)), int(match.group(6)))
            
            if(date>=firstDate and date<=lastDate):
                histList.append(file)
                
        if(len(histList)>0):
            # Create history directory if it doesn't exist yet
            histDir = "%s/RunningHistory" % baseDir
            if not os.path.exists(histDir):
                os.makedirs(histDir)

            # Remove old plots
            os.system("rm -f %s/*.png" % histDir) 

            # Assemble command
            cmd = "vemcal-history-plot"
            for histFile in histList:
                cmd = cmd + " " + histFile
            #print cmd

            print "Generating running history plots ..."
            os.chdir(histDir)
            os.system(cmd)

            print "Creating thumbnails ..."
            createThumbNails(histDir, "png")
        else:
            print "No history rootfiles found between %s and %s. Cannot create running history graphs!" % (firstDate, lastDate)
    else:
        print "No history rootfiles found. Cannot create running history graphs!"
