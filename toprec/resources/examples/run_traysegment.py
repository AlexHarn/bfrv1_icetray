#!/usr/bin/python

## This is an example of how to run a Laputop traysegment.

import os
import sys,getopt
#import logging
from os.path import expandvars

from I3Tray import *
from icecube import icetray, dataclasses, dataio, tableio, toprec, rootwriter, recclasses
from icecube.dataclasses import I3Particle

## Are you using tableio?
from icecube.tableio import I3TableWriter
from icecube.rootwriter import I3ROOTTableService


load("libicetray")
load("libdataclasses")
load("libphys-services")
load("libdataio")
load("libtoprec")
load("librootwriter")
load("libtableio")
load("libicepick")


## Set the log level
## Uncomment this in if you're doing code development!
#icetray.set_log_level(icetray.I3LogLevel.LOG_DEBUG)
#icetray.set_log_level_for_unit('Laputop', icetray.I3LogLevel.LOG_DEBUG)


workspace = expandvars("$I3_BUILD")
tools = expandvars("$I3_PORTS")
HOME = expandvars("$HOME")

#### PUT YOUR FAVORITE GCD AND INPUT FILE HERE
# KR's Mac:  these files contain a mixture of good events with bad events (failed seeds, insufficient hits, etc.)
infiles = [ HOME+"/data/exp_it73/l2a/Level2a_IC79_data_Run00116080_GCD.i3.gz",  # GCD
            HOME+"/data/exp_it73/l2a/Level2a_IC79_data_Run00116080_Part00000000_IT.i3.bz2",
            HOME+"/data/exp_it73/l2a/Level2a_IC79_data_Run00116080_Part00000001_IT.i3.bz2",
            HOME+"/data/exp_it73/l2a/Level2a_IC79_data_Run00116080_Part00000002_IT.i3.bz2",
            HOME+"/data/exp_it73/l2a/Level2a_IC79_data_Run00116080_Part00000003_IT.i3.bz2",
            HOME+"/data/exp_it73/l2a/Level2a_IC79_data_Run00116080_Part00000004_IT.i3.bz2",
            HOME+"/data/exp_it73/l2a/Level2a_IC79_data_Run00116080_Part00000005_IT.i3.bz2",
            HOME+"/data/exp_it73/l2a/Level2a_IC79_data_Run00116080_Part00000006_IT.i3.bz2" ]

# KR's desktop:  these files contain some "pre-cleaned" good events
#infiles = [ "/media/_basil/exp_it73/gcd/Level2_IC79_data_Run00116097_monthlysnow_GCD.i3.gz", #GCD
#            "/media/_basil/exp_it73/snow15reco/snowcorr_116097_monthlysnow.i3.gz" ]

#### NAME YOUR OUTPUT FILES HERE
I3_OUTFILE = HOME+"/testout/toprec_testme.i3"
ROOTFILE = HOME+"/testout/toprec_testme.root"


tray = I3Tray()


#------------------- LET'S RUN SOME MODULES!  ------------------

#**************************************************
#                 Reader and whatnot
#**************************************************
tray.AddModule("I3Reader","reader")(
    ("FileNameList", infiles)
    )

# Add this one if your input file does NOT come from Q-frame-land:
#tray.AddModule("QConverter","qconvert")(  
#    )


## What pulses do you want to use to make the fit?
#datareadoutName='IceTopVEMPulses_0'    # This one is old
datareadoutName='CleanedHLCTankPulses'   # This one is more cleaned, and more commonly in standard use.


"""
# You've probably already got these in your input file.  Uncomment these if you don't.
#**************************************************
#     first guess reconstruction - plane & cog
#**************************************************

tray.AddModule("I3TopRecoCore", "core")(
    ("datareadout", datareadoutName),
    )

tray.AddModule("I3TopRecoPlane","plane")(
    ("datareadout", datareadoutName),
    )
"""

"""
## This bit will get rid of the bad seeds, and focus only on 'good' events
class QuickFilter(icetray.I3Module): 
    def __init__(self, context):                       ## Constructor 
        icetray.I3Module.__init__(self, context) 
        self.AddOutBox('OutBox')

    def Configure(self):
        # ain't got nothin'
        #        self.outputname = self.GetParameter('OutputName') 
        self.fakenumber = 6

    def Physics(self, frame): 
        cog = frame.Get('ShowerCOG');
        plane = frame.Get('ShowerPlane');
        ok1 = cog.fit_status
        ok2 = plane.fit_status
        if (ok1 == I3Particle.FitStatus.OK & ok2 == I3Particle.FitStatus.OK): 
            self.PushFrame(frame)                     # push the frame 
        return

tray.AddModule(QuickFilter,"prepareme")
"""


#tray.AddModule("Dump","dump")()


##------------------- RUN A FEW LAPUTOP'S ------------------

from toprec import LaputopStandardMay2014
from toprec import LaputopSmallShower
from laputop_laststeponly_traysegment import LaputopLastStepSnowOnly

## This one runs TF's favorite settings from the beginning
tray.AddSegment(LaputopStandardMay2014,"Laputop", pulses=datareadoutName)

## This one runs just one step (direction fixed, and no curvature)
tray.AddSegment(LaputopSmallShower,"LapSmallShower", pulses=datareadoutName)

## This one takes a previous Laputop (or ShowerCombined) as a seed, and 
## runs just the last step again, with different snow factor
tray.AddSegment(LaputopLastStepSnowOnly,"LapAgain", 
                seed="Laputop", pulses=datareadoutName, snowfactor=4.0)



##----------------- OUTPUT --------------------------


## Output i3 file
tray.AddModule("I3Writer","EventWriter")(
    ("DropOrphanStreams", [icetray.I3Frame.DAQ]),
    ("Filename",I3_OUTFILE),
    ) 


## Output root file
root = I3ROOTTableService(ROOTFILE,"aTree")
tray.AddModule(I3TableWriter,'writer')(
    ("tableservice", root),
    ("keys",[ "Laputop", "LaputopParams", "LaputopSnowDiagnostics",
              "LapSmallShower", "LapSmallShowerParams", "LapSmallShowerSnowDiagnostics",
              "LapAgain", "LapAgainParams", "LapAgainSnowDiagnostics",
              'ShowerCOG', 'ShowerPlane', 'ShowerPlaneParams', ]),
    ("subeventstreams",["top_hlc_clusters"])
    )



 
   
# Execute the Tray
# one event
#tray.Execute(5)
tray.Execute(1000)


   
