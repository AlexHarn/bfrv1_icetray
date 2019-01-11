#!/usr/bin/python

import glob
import re
import os
import sys

def generateHistory(vemFile, runList, baseDir="/data/exp/IceCube", scratchDir="."):
        
    # Build basic command
    cmd = "vemcal-history " + vemFile
    
    runList.sort()
    if len(runList)>0:
        # Create scratch directory
        if not os.path.exists(scratchDir):
            print "ERROR: Scratch directory %s doesn't exist!" % scratchDir
            sys.exit()
        else:
            scratchDir = "%s/monifiles_%d-%d" % (scratchDir, runList[0], runList[-1])
            os.mkdir(scratchDir)
            os.system("rm -rf %s/*" % scratchDir)
            
        # Define filename pattern
        runPattern  = re.compile('moni_histo_(\d*).tar.gz')

        # Untar moni-files into scratch directory
        yearDirs = glob.glob("%s/[0-9][0-9][0-9][0-9]" % baseDir)
        yearDirs.sort()
        for yearDir in yearDirs:
            moniFiles = glob.glob("%s/monitoring/moni/[0-9][0-9][0-9][0-9]/moni_histo_*.tar.gz" % yearDir)
            moniFiles.sort()
         
            if len(moniFiles)==0:
                continue

            firstRunID = int(runPattern.search(moniFiles[0]).group(1))
            if firstRunID > runList[-1]:
                continue
                  
            lastRunID = int(runPattern.search(moniFiles[-1]).group(1))
            if lastRunID < runList[0]:
                continue
         
            for file in moniFiles:
                runID = int(runPattern.search(file).group(1))
                if runID in runList:
                    os.system("tar -xzf %s --directory %s" % (file, scratchDir))

    
        # Add moni rootfiles to command
        rootFiles = glob.glob("%s/*.root" % scratchDir)
        rootFiles.sort()
        for file in rootFiles:
            cmd = cmd + " " + file

    
    # Execute command (create history file)
    print cmd
    os.system(cmd)
    
    # Remove scratch directory
    if os.path.exists(scratchDir):
        os.system("rm -rf %s" % scratchDir)
