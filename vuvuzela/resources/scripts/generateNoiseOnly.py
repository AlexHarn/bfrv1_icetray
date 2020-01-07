#!/usr/bin/env python
from optparse import OptionParser
from os.path import expandvars
import unittest

usage = "usage: %prog [options]"
parser = OptionParser(usage)
parser.add_option("-o", "--outfile",default="",
                  dest="OUTFILE", help="Write output to OUTFILE (.i3{.gz} format)")
parser.add_option("-s", "--seed",type="int",default=-1,
                  dest="SEED", help="Initial seed for the random number generator")
parser.add_option("-n", "--numevents", type="int", default=50,
                  dest="NUMEVENTS", help="The number of events per run")
parser.add_option("-g", "--gcdfile", default=expandvars("$I3_TESTDATA/GCD/GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz"),
		  dest="GCDFILE", help="Read in GCD file")
parser.add_option("-r", "--runnumber", type="int", default=1,
		  dest="RUNNUMBER", help="Run number")
parser.add_option("--notest", help="Don't run through various tests.",
		  dest="TEST", action="store_false", default=True)

# parse cmd line args, bail out if anything is not understood
(options,args) = parser.parse_args()
if len(args) != 0:
        crap = "Got undefined options:"
        for a in args:
                crap += a
                crap += " "
        parser.error(crap)
		  
import I3Tray
from I3Tray import *
import os
import sys
import random

############################################################
# Write a test module to make sure we get the right number of
#  frames, triggers, and hits in each frame.
############################################################
nfound = 0
nhits = []
triggers = {1006:[], 1011:[]}
class NoiseTriggerTest(unittest.TestCase):
	def test_nframes(self):
		#self.assertEqual(nfound, 3)
		self.assertGreater(nfound,1)
	
	# Test the triggers. We shouldn't get any SMT8s, but should get a bunch of SMT3s.
	def test_triggers(self):
		expected = {1011: [True for i in range(nfound)],
			    1006: [False for i in range(nfound)],}
		self.assertEqual( expected, triggers )
		

############################################################
# IceTray specific stuff. If the seed is set to -1 pick a random one
############################################################
from icecube import icetray, dataclasses, dataio, phys_services, sim_services
from icecube.icetray import logging, I3Frame, I3Units
logging.I3LogLevel(0)

tray = I3Tray()

# Check to see if the user defined a seed for the random number generator
if options.SEED == -1:
	# if not, use a random number
	options.SEED = random.randint(0, 1e6)

# Always use the same seed when running a test!
if options.TEST:
	options.SEED = 10

############################################################
#  Set up the header and read in the GCD frames
############################################################
tray.AddModule("I3InfiniteSource","streams",
               Stream=icetray.I3Frame.DAQ,
               prefix=options.GCDFILE,
               )

tray.AddService("I3GSLRandomServiceFactory","random",
		InstallServiceAs = "I3RandomService",
		Seed = options.SEED,
		)

############################################################
# Vuvuzela
############################################################
from icecube import vuvuzela
tray.AddModule("Inject", "AddNoiseParams",
	       InputNoiseFile = expandvars("$I3_SRC/vuvuzela/resources/data/parameters.dat"),
	       )

tray.AddModule("Vuvuzela", "vuvuzela",
	       InputHitSeriesMapName  = "",
	       OutputHitSeriesMapName = "I3MCPESeriesMap",
	       StartWindow            = 0,
	       EndWindow              = 25*I3Units.millisecond,
               OMTypes                = [dataclasses.I3OMGeo.IceCube, 
                                         dataclasses.I3OMGeo.PDOM,
                                         dataclasses.I3OMGeo.mDOM,],
               RandomServiceName      = "I3RandomService",
	       UseIndividual          = True,
	       DisableLowDTCutoff     = True,
	       )


############################################################
# Test some of the stranger options to make sure they work.
############################################################
if options.TEST:
	tray.AddModule("Vuvuzela", "vuvuzela_OptionTester",
		       InputHitSeriesMapName  = "",
		       OutputHitSeriesMapName = "I3MCPESeriesMapTester",
		       StartWindow            = 0,
		       EndWindow              = 10*I3Units.microsecond,
                       ScaleFactor            = 1.2,
		       DeepCoreScaleFactor    = 1.1,
		       RandomServiceName      = "I3RandomService",
                       OMTypes                = [dataclasses.I3OMGeo.IceCube, 
                                                 dataclasses.I3OMGeo.PDOM,
                                                 dataclasses.I3OMGeo.mDOM,],
                       UseIndividual          = True,
		       DisableLowDTCutoff     = False,
		       )

############################################################
# Now generate the pulses from the simulated hits
############################################################
from icecube import DOMLauncher
#create PMT response
tray.AddModule("PMTResponseSimulator", "rosencrantz",
	       Input = "I3MCPESeriesMap", #default "MCHitSeriesMap"; The name of the I3MCHitSeriesMap to process.
	       Output = "I3MCPulseSeriesMap", #default "I3MCPulseSeriesMap"; The name of the I3MCHitSeriesMap to be produced
	       MergeHits = True, #defalut False; Whether hits very near in time should be merged in the output
	       )
		  
#create Detector response
tray.AddModule("DOMLauncher", "guildenstern",
	       Input = "I3MCPulseSeriesMap", #default "I3MCPulseSeriesMap"; Input is a MCHitseriesMap.
	       Output = "InIceRawData", #default "DOMLaunchSeries"; The output name of the DOMLaunchSeries.
	       MergePulses = True, #default True; Whether hits in the past should be merged into bunches.
	       UseTabulatedPT = True, #default False; Whether to use interpolated SPE templates.
	       )


############################################################
# Triggering uses trigger-sim and CAP
############################################################
from icecube import trigger_sim
tray.AddSegment(trigger_sim.TriggerSim, "trigger-sim",
                run_id = 12345,
		gcd_file = dataio.I3File(options.GCDFILE),
		)

# Fake an MCTree for CoincidenceAfterProcessing
from icecube.dataclasses import I3MCTree
def fakeTree(frame):
	frame["I3MCTree"] = I3MCTree()
	
tray.AddModule(fakeTree, "fakeDatTree",
	       Streams = [I3Frame.DAQ,])

# CAP also requires (but doesn't check for) something called noise_weight
from icecube.dataclasses import I3MapStringDouble
def NoiseWeight(frame):
	weightmap = I3MapStringDouble()
	weightmap["simulated_livetime"] = 25*I3Units.millisecond * options.NUMEVENTS
	weightmap["estimated_time_buffer"] = 20*I3Units.microsecond * options.NUMEVENTS
	weightmap["muon_flux_rate"] = 2800*I3Units.hertz
	weightmap["muon_trigger_time"] = 30*I3Units.microsecond
	weightmap["weight"] = 1/( (weightmap["simulated_livetime"]-weightmap["estimated_time_buffer"]) *
				  (1 - weightmap["muon_flux_rate"] * weightmap["muon_trigger_time"]) )
	frame["noise_weight"] = weightmap
	
tray.AddModule(NoiseWeight,  "addWeight",
	       Streams = [I3Frame.DAQ,])

tray.AddModule("CoincidenceAfterProcessing", "coincAfterproc",
	       MinimumSignalNch=0,
	       )

#clean obsolete trayKeys
tray.AddModule("Delete", "del",
	       keys = [ "MCPMTResponseMap",],# "I3MCPulseSeriesMap" ],
	       )

##############################
# If doing testing, grab all of the needed info from the frame.
# In principle, this could be a number of things.
# Testing the number of frames, hits, and triggers for now.
# We could add similar tests with the other options eventually.
##############################
if options.TEST:
	def collectInfo(frame):
		global nfound, triggers, nhits
		
		# Collect trigger info as well. We only care about SMT8 and SMT3
		triggerhierarchy = frame["I3TriggerHierarchy"]
		smt8, smt3 = False, False
		for t in triggerhierarchy:
                    if t.key.config_id == 1006: smt8 = True
                    if t.key.config_id == 1011: smt3 = True

                # We only want to include SMT3 and SMT8 here
		if (not smt8) and (not smt3):
                    return  
                    
		nfound += 1
		nhits.append( len(frame["InIceRawData"]) )

		triggers[1006].append(smt8)
		triggers[1011].append(smt3)

	tray.AddModule(collectInfo,
		       Streams = [I3Frame.DAQ])

##############################
# Write and run
##############################
if options.OUTFILE:
	tray.AddModule("I3Writer","writer",
		       Filename = options.OUTFILE,
		       Streams = [icetray.I3Frame.DAQ],
		       #SkipKeys = ["MCHitSeriesMap"],
		       )

tray.Execute(options.NUMEVENTS + 4) # 4 for IGCD

# If we're doing a test, make sure we have the correct number of frames and hits in each frame
if options.TEST:
	unittest.main()
