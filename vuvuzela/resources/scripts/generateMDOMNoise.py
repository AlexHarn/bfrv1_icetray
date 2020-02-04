#!/usr/bin/env python
from optparse import OptionParser
from os.path import expandvars
import unittest

usage = "usage: %prog [options]"
parser = OptionParser(usage)
parser.add_option("-o", "--outfile",default="test.i3.bz2",
                  dest="OUTFILE", help="Write output to OUTFILE (.i3{.gz} format)")
parser.add_option("-s", "--seed",type="int",default=-1,
                  dest="SEED", help="Initial seed for the random number generator")
parser.add_option("-n", "--numevents", type="int", default=50,
                  dest="NUMEVENTS", help="The number of events per run")
parser.add_option("-g", "--gcdfile", default=expandvars("/data/sim/IceCubeUpgrade/geometries/GCDs/GeoCalibDetectorStatus_ICUpgrade.v53.mixed.V1.i3.bz2"),
		  dest="GCDFILE", help="Read in GCD file")
parser.add_option("-r", "--runnumber", type="int", default=1,
		  dest="RUNNUMBER", help="Run number")

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

random = phys_services.I3GSLRandomService(options.SEED)

############################################################
# Vuvuzela
############################################################
from icecube import vuvuzela
tray.AddModule("Inject", "AddNoiseParams",
	       InputNoiseFile = expandvars("$I3_SRC/vuvuzela/resources/data/parameters.dat"),
	       )

t0 = -15*I3Units.microsecond
tf = 15*I3Units.microsecond

import time 
def start(frame):
        if 'start' in frame.keys(): del frame['start']
        frame['start'] = dataclasses.I3Double(time.time())
tray.Add(start, Streams=[icetray.I3Frame.DAQ])

tray.AddModule("Vuvuzela", "vuvuzela",
	       InputHitSeriesMapName  = "",
	       OutputHitSeriesMapName = "I3MCPESeriesMap",
	       StartWindow            = t0,
	       EndWindow              = tf,
               OMTypes                = [dataclasses.I3OMGeo.IceCube, 
                                         dataclasses.I3OMGeo.PDOM,
                                         dataclasses.I3OMGeo.DEgg
                                         ],
               RandomServiceName      = "I3RandomService",
	       UseIndividual          = True,
	       DisableLowDTCutoff     = True,
               SimulateNewDOMs        = True,
	       )
def end(frame, name='\nvuvuzela'):
        t = time.time() - frame['start'].value
        print "{} took {} seconds to produce {} seconds of livetime".format(name, t, tf-t0)
tray.Add(end, Streams=[icetray.I3Frame.DAQ])


############################################################
# Add hits for the mDOM as well
############################################################
tray.Add(start, Streams=[icetray.I3Frame.DAQ])
tray.AddModule(vuvuzela.PregeneratedSampler, 'mdom_vuvuzela',
               InputPath = '/cvmfs/icecube.opensciencegrid.org/users/mlarson/mdom_noise/mDOM_darkrate_-25C.*.npy',
               PhysicsHitMap = None,
               OutputHitMap = "I3MCPESeriesMap_mdom",
               StartWindow = t0,
               EndWindow = tf,
               ModuleType = dataclasses.I3ModuleGeo.ModuleType.mDOM,
               RandomService = random,
)
tray.Add(end, name='PS', Streams=[icetray.I3Frame.DAQ])


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

