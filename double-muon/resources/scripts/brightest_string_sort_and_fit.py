#!/usr/bin/env python

# This script illustrates sorting pulses in an event based on their proximity to the brightest string, 
# which is defined as the string with the most hits.
# This is good for vertical and near-vertical tracks, with a bundle of muons and a
# single muon with a large lateral separation (such as a muon generated from charm with 
# a large (>4 GeV/c) transverse momentum). Usually the muon bundle is brighter, so it generates
# the string with the most hits.
# This script is based on doublefit_10par.py. Please take the Feature Extraction settings with a grain of salt. 
# They are not official.

#import os, sys
from os.path import expandvars
from optparse import OptionParser

##################################################################
# parse options for scripts
##################################################################

usage = "usage: %prog [options] inputfile"
scriptname="brightest_string_sort_and_fit"
# filename = expandvars("$I3_TESTDATA/icesim-V01-09-02/corsika.000085.000007.10eventsIC9.i3.gz")
filename = expandvars("$I3_TESTDATA/event-viewer/Level3aGCD_IC79_EEData_Run00115990.i3")
parser = OptionParser(usage)
parser.add_option("-i", "--infile",default=filename,
                  dest="INFILE", help="read input to INFILE (.i3.gz format)")
parser.add_option("-g", "--gcdfile",default="",
                  dest="GCDFILE", help="read geo-cal-detstat to GCDFILE (.i3.gz)")
parser.add_option("-o", "--outfile",default=scriptname+"i3.gz",
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
### IceCube Processing: calibrate waveforms to mV, baseline subtraction etc
#tray.AddModule("I3DOMcalibrator","domcal")(
#    ("InputRawDataName","CleanInIceRawData"),
#    ("CorrectPedestalDroopDualTau",False),
#    )

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


# eliminate obvious outliers (only timewise)
tray.AddModule("I3TimeWindowCleaning<I3RecoPulse>","timewindow")(
    ("InputResponse","MaskedOfflinePulses"),
    ("OutputResponse","TWCleanPulseSeries"),
    ("TimeWindow",6000)
    )

#########################################################
# SPLIT PULSES ACCORDING TO PROXIMITY TO BRIGHTEST STRING
#########################################################

# put all pulses within 150 m of brightest string in one bin, the rest in second bin.

tray.AddModule("I3ResponseMapSplitter","BrightStringPulseSeries")(
    ("InputPulseMap","TWCleanPulseSeries"),
    ("DoBrightSt",True),
    ("MaxDBrightSt",150),
    ("SplitEvents",False)
    )

##################################
# LINE FIT FOR EACH SET OF PULSES
##################################

# fit pulses closest to brightest string
tray.AddModule("I3LineFit","BrightStringLineFit1")(
    ("LeadingEdge","FLE"),
    ("AmpWeightPower",0.0),
    ("InputRecoPulses","BrightStringPulseSeries1"),
    ("MinHits",2),
    ("Name","BrightStringLineFit1"),
    )

#Now fit the rest
tray.AddModule("I3LineFit","BrightStringLineFit2")(
    ("LeadingEdge","FLE"),
    ("AmpWeightPower",0.0),
    ("InputRecoPulses","BrightStringPulseSeries2"),
    ("MinHits",2),
    ("Name","BrightStringLineFit2"),
    )

##################
# WRITING OUTPUT #
##################

tray.AddModule("I3Writer","writer")(
    ("FileName",outfile),
    ("SkipKeys",["CalibratedATWD","CalibratedFADC","MCPMTResponseMap"]),
    ("Streams",[icetray.I3Frame.Physics]),
)


# take out the trash

    
tray.Execute(10+3)


# run as a test; clean up
import os
os.unlink(outfile)

