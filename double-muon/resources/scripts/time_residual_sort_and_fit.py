#!/usr/bin/env python

# This script illustrates sorting pulses in an event based on their time residual
# relative to a specified reconstruction track.
# This is good for vertical and near-vertical tracks, with a bundle of muons and a
# single muon with a large lateral separation (such as a muon generated from charm with 
# a large (>4 GeV/c) transverse momentum). Once the bundle has been fit, hits from the muon
# will have a negative time residual relative to this fit.
# Unfortunately, the IC-9 data file isn't a very good example of this, but at least this will
# show how to call the algorithm.
# This script is based on doublefit_10par.py. Please take the Feature Extraction settings with a grain of salt. 
# They are not official.

#import os, sys
from os.path import expandvars
from optparse import OptionParser

##################################################################
# parse options for scripts
##################################################################

usage = "usage: %prog [options] inputfile"
# filename = expandvars("$I3_TESTDATA/icesim-V01-09-02/corsika.000085.000007.10eventsIC9.i3.gz")
filename = expandvars("$I3_TESTDATA/event-viewer/Level3aGCD_IC79_EEData_Run00115990.i3")
scriptname="time_residual_sort_and_fit"

parser = OptionParser(usage)
parser.add_option("-i", "--infile",default=filename,
                  dest="INFILE", help="read input to INFILE (.i3.gz format)")
parser.add_option("-g", "--gcdfile",default="",
                  dest="GCDFILE", help="read geo-cal-detstat to GCDFILE (.i3.gz)")
parser.add_option("-o", "--outfile",default=scriptname+".i3.gz",
                  dest="OUTFILE", help="write output to i3 OUTFILE (.i3 or .i3.gz format)")
parser.add_option("-r", "--rootfile",default=scriptname+".root",
                  dest="ROOTFILE", help="write output to flat-ntuple ROOTFILE (.root format)")
parser.add_option("-n", "--nevents",type="int",default=10,
                  dest="NEVENTS", help="Number of events to process (default: all)")
parser.add_option("-T", "--truthname",default="I3MCTree",
                  dest="MCTRUTH", help="Name of MC truth list/tree")

# parse cmd line args, bail out if anything is not understood
(options,args) = parser.parse_args()
if len(args) != 0:
    crap = "Got undefined options:" + " ".join(args)
    parser.error(crap)

# get/check filenames etc
infiles = []
if options.GCDFILE:
    infiles += [options.GCDFILE]
infiles += [options.INFILE]
outfile = options.OUTFILE
rootfile = options.ROOTFILE
nevents = options.NEVENTS

mctruth=""
if options.MCTRUTH:
    mctruth=options.MCTRUTH
    print("MC truth name %s" % mctruth)

##################################################################
# load icetray + libraries
##################################################################

from I3Tray import *

from icecube import icetray
from icecube import dataclasses
from icecube import dataio
load("libdouble-muon")
from icecube import DomTools
from icecube import linefit
from icecube import icepick

tray = I3Tray()

####################################
# READING DATA / EXTRACTING PULSES #
####################################

# read input data
tray.AddModule("I3Reader", "reader")(
    ("SkipKeys", ["AMANDAPMTPulseMap",
                  "MCPMTResponseMap",
                  "I3MCPMTResponse",
                  "CascadeFirstParams",
                  "IceCubePMTPulseMap",
                  "MMCTrackList"]),
    ("filenamelist", infiles ),
    )

### IceCube Processing: remove dead/bad DOMs (NOTE: only IC9)
#tray.AddModule("I3DOMLaunchCleaning","launchcleaning")(
#    ("InIceInput","InIceRawData"),
#    ("InIceOutput","CleanInIceRawData"),
#    ("FirstLaunchCleaning",True),
#    ("CleanedKeys",[OMKey(29,59),#Dead/problem list
#                    OMKey(29,60),
#                    OMKey(30,23),
#                    OMKey(30,60),
#                    OMKey(38,59),
#                    OMKey(39,22),
#                    OMKey(50,36),
#                    OMKey(74,9),
#                    OMKey(40,51),
#                    OMKey(40,52),
#                    OMKey(46,49),
#                    OMKey(46,50),
#                    OMKey(46,53),
#                    OMKey(46,54),
#                    OMKey(46,55),
#                    OMKey(46,56),
#                    OMKey(47,55),
#                    OMKey(47,56),
#                    OMKey(56,57),
#                    OMKey(56,58),
#                    OMKey(72,37),
#                    OMKey(72,38),
#                    OMKey(39,8), #Dark Noise mode only DOMS
#                    OMKey(50,58),
#                    OMKey(72,33),
#                    OMKey(72,34),
#                    OMKey(72,35),
#                    OMKey(72,36),
#                    OMKey(72,43),])
#    )
#

# eliminate obvious outliers (only timewise)
tray.AddModule("I3TimeWindowCleaning<I3RecoPulse>","timewindow")(
    ("InputResponse","MaskedOfflinePulses"),
    ("OutputResponse","TWCleanPulseSeries"),
    ("TimeWindow",6000)
    )

# require big events
tray.AddModule("I3IcePickModule<I3NLaunchFilter>","NhitRetrigger")(
    ("DiscardEvents",True),
    ("DecisionName","NhitRetrigger"),
    ("DataReadoutName","InIceRawData"),
    ("MinNLaunch",12),
)
tray.AddModule("I3IcePickModule<I3NLaunchFilter>","NstringRetrigger")(
    ("DiscardEvents",True),
    ("DecisionName","NstringRetrigger"),
    ("DataReadoutName","InIceRawData"),
    ("CountWhat","strings"),
    ("MinNLaunch",2),
)

###############################################
# LINE FIT TO USE AS BASIS FOR PULSE SEPARATION
###############################################

# fit all pulses
tray.AddModule("I3LineFit","LineFit")(
    ("LeadingEdge","FLE"),
    ("AmpWeightPower",0.0),
    ("InputRecoPulses","TWCleanPulseSeries"),
    ("MinHits",2),
    ("Name","LineFit"),
    )

#########################################
# SPLIT PULSES ACCORDING TO TIME RESIDUAL
#########################################

# put all pulses with a negative time residual in one bin, the rest in second bin.

tray.AddModule("I3ResponseMapSplitter","TRPulseSeries")(
    ("InputPulseMap","TWCleanPulseSeries"),
    ("InputTrackName","LineFit"),
    ("DoTRes",True),
    ("MinTRes",0.),
    ("MaxTRes",1.e9)
)

##################################
# LINE FIT FOR EACH SET OF PULSES
##################################

# fit pulses closest to brightest string
tray.AddModule("I3LineFit","TRLineFit1")(
    ("LeadingEdge","FLE"),
    ("AmpWeightPower",0.0),
    ("InputRecoPulses","TRPulseSeries1"),
    ("MinHits",2),
    ("Name","TRLineFit1"),
    )

#Now fit the rest
tray.AddModule("I3LineFit","TRLineFit2")(
    ("LeadingEdge","FLE"),
    ("AmpWeightPower",0.0),
    ("InputRecoPulses","TRPulseSeries2"),
    ("MinHits",2),
    ("Name","TRLineFit2"),
    )

##################
# WRITING OUTPUT #
##################

tray.AddModule("I3Writer","writer")(
    ("FileName",outfile),
    ("SkipKeys",["CalibratedATWD","CalibratedFADC","MCPMTResponseMap"]),
#    ("Streams",["Physics"])
)


# take out the trash

    
tray.Execute(10+3)


import os
os.unlink(outfile)

