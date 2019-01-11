#!/usr/bin/env python

######################################
# This script demonstrates the use of the spline-reco segments
# for the configurations "default", "fast", "recommended" and "max"
# with and without additional "MuEXAngular" seed.
# See https://wiki.icecube.wisc.edu/index.php/Spline-reco for
# documentation as well as speed and resolution comparisons.
#
# Usage example: python SplineRecoSegments.py Level3_nugen_numu_IC86.2011.009366.000000 Level3b_nugen_numu_IC86.2011.009366.000000 GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz
#####################################

import os, sys
from stat import *
from os.path import expandvars
import glob
import icecube
from icecube import icetray, dataclasses, dataio, photonics_service, hdfwriter, tableio, spline_reco
from I3Tray import *

# Retrieve arguments
if len(sys.argv) <= 1:
    print '\nERROR:\nRequired arguments: outputfile.i3\n'
    raise Exception("ERROR: arguments not passed")

#inputfile = sys.argv[1]
inputfile = os.environ['I3_TESTDATA']+'/sim/Level2_IC86.2011_corsika.010281.001664.00.i3.bz2'
print 'Preparing to read input file  %s' % inputfile
if os.access(inputfile, os.R_OK) == False:
    raise Exception("ERROR: Cannot find $I3_TESTDATA input i3 file %s!" % inputfile)

#gcdfile = sys.argv[2]
gcdfile = os.environ['I3_TESTDATA']+'/sim/GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz'
print 'Preparing to read GCD file  %s' % gcdfile
if os.access(gcdfile, os.R_OK) == False:
    raise Exception("ERROR: Cannot find GCD i3 file %s!" % gcdfile)

outputfile = sys.argv[1]
print 'Preparing to write i3 file  %s' % outputfile
if os.access(outputfile, os.R_OK) == True:
    raise Exception("ERROR: Output file %s already exists!" % outputfile)

#outrootfile_flat = outputfile + "_flat.root"
# This is meant to process all parts of a run
#inputfiles = glob.glob(inputfile+"*.i3.bz2")
#inputfiles.sort()
inputfiles = sorted(glob.glob(inputfile))
inputfiles.insert(0,gcdfile)
print 'Preparing to read i3 file(s)  %s' % inputfiles

#load("libdataio")
#load("libflat-ntuple")

tray = I3Tray()
icetray.set_log_level(icetray.I3LogLevel.LOG_INFO)

##################################################################
# CONFIG
##################################################################

#spline paths Mainz
#timingSplinePath = '/project/icecube/maigrid.data_icecube/schatto/InfBareMu_mie_spline/InfBareMu_mie_prob_z20a10.fits'
#amplitudeSplinePath = '/project/icecube/maigrid.data_icecube/schatto/InfBareMu_mie_spline/InfBareMu_mie_abs_z20a10.fits'

#stochTimingSplinePath = '/project/icecube/maigrid.data_icecube/schatto/InfHighEStoch_mie_spline/InfHighEStoch_mie_prob_z20a10.fits'
#stochAmplitudeSplinePath = '/project/icecube/maigrid.data_icecube/schatto/InfHighEStoch_mie_spline/InfHighEStoch_mie_abs_z20a10.fits'

#spline paths Madison
timingSplinePath = '/cvmfs/icecube.opensciencegrid.org/data/photon-tables/splines/InfBareMu_mie_prob_z20a10_V2.fits'
amplitudeSplinePath = '/cvmfs/icecube.opensciencegrid.org/data/photon-tables/splines/InfBareMu_mie_abs_z20a10_V2.fits'
stochTimingSplinePath = '/cvmfs/icecube.opensciencegrid.org/data/photon-tables/splines/InfHighEStoch_mie_prob_z20a10.fits'
stochAmplitudeSplinePath = '/cvmfs/icecube.opensciencegrid.org/data/photon-tables/splines/InfHighEStoch_mie_abs_z20a10.fits'
pulses = "SRTOfflinePulses"
EnEstis = ["SplineMPETruncatedEnergy_SPICEMie_AllDOMS_Muon",
        "SplineMPETruncatedEnergy_SPICEMie_DOMS_Muon",
        "SplineMPETruncatedEnergy_SPICEMie_AllBINS_Muon",
        "SplineMPETruncatedEnergy_SPICEMie_BINS_Muon",
        "SplineMPETruncatedEnergy_SPICEMie_ORIG_Muon"]


##################################################################
# Read
##################################################################

# Read level1 filtered files
tray.AddModule("I3Reader", "Reader")(
       ("FilenameList", inputfiles),
       ("skipkeys",["AtmCscdEnergyRecoParams","CascadeLast_DCParams","CascadeLastParams","OldAtmCscdEnergyRecoParams"]),
      )

# Do 4 recos with different configs
tray.AddSegment(spline_reco.SplineMPE, "SplineMPEdefault",
        configuration="default", PulsesName=pulses,
        #TrackSeedList=["MPEFit_TT"], BareMuTimingSpline=timingSplinePath, # we don't have in test data
        TrackSeedList=["SPEFit2"], BareMuTimingSpline=timingSplinePath,
        BareMuAmplitudeSpline=amplitudeSplinePath)
tray.AddSegment(spline_reco.SplineMPE, "SplineMPEfast",
        #configuration="fast", PulsesName=pulses, TrackSeedList=["MPEFit_TT"], # we don't have in test data
        configuration="fast", PulsesName=pulses, TrackSeedList=["SPEFit2"],
        BareMuTimingSpline=timingSplinePath,
        BareMuAmplitudeSpline=amplitudeSplinePath, EnergyEstimators=EnEstis)
tray.AddSegment(spline_reco.SplineMPE, "SplineMPErecommended",
        configuration="recommended", PulsesName=pulses,
        #TrackSeedList=["MPEFit_TT"], BareMuTimingSpline=timingSplinePath, # we don't have in test data
        TrackSeedList=["SPEFit2"], BareMuTimingSpline=timingSplinePath,
        BareMuAmplitudeSpline=amplitudeSplinePath, EnergyEstimators=EnEstis)
tray.AddSegment(spline_reco.SplineMPE, "SplineMPEmax",
        #configuration="max", PulsesName=pulses, TrackSeedList=["MPEFit_TT"], # we don't have in test data
        configuration="max", PulsesName=pulses, TrackSeedList=["SPEFit2"],
        BareMuTimingSpline=timingSplinePath,
        BareMuAmplitudeSpline=amplitudeSplinePath,
        StochTimingSpline=stochTimingSplinePath,
        StochAmplitudeSpline=stochAmplitudeSplinePath,
        EnergyEstimators=EnEstis)

# Do the same recos with additional "MuEXAngular4" seed.
tray.AddSegment(spline_reco.SplineMPE, "SplineMPEdefaultMuEXSeed",
        configuration="default", PulsesName=pulses,
        #TrackSeedList=["MPEFit_TT","MuEXAngular4"], # we don't have in test data
        TrackSeedList=["SPEFit2","SPEFitSingle"],
        BareMuTimingSpline=timingSplinePath,
        BareMuAmplitudeSpline=amplitudeSplinePath)
tray.AddSegment(spline_reco.SplineMPE, "SplineMPEfastMuEXSeed",
        configuration="fast", PulsesName=pulses,
        #TrackSeedList=["MPEFit_TT","MuEXAngular4"], # we don't have in test data
        TrackSeedList=["SPEFit2","SPEFitSingle"],
        BareMuTimingSpline=timingSplinePath,
        BareMuAmplitudeSpline=amplitudeSplinePath, EnergyEstimators=EnEstis)
tray.AddSegment(spline_reco.SplineMPE, "SplineMPErecommendedMuEXSeed",
        configuration="recommended", PulsesName=pulses,
        #TrackSeedList=["MPEFit_TT","MuEXAngular4"], # we don't have in test data
        TrackSeedList=["SPEFit2","SPEFitSingle"],
        BareMuTimingSpline=timingSplinePath,
        BareMuAmplitudeSpline=amplitudeSplinePath, EnergyEstimators=EnEstis)
tray.AddSegment(spline_reco.SplineMPE, "SplineMPEmaxMuEXSeed",
        configuration="max", PulsesName=pulses,
        #TrackSeedList=["MPEFit_TT","MuEXAngular4"], # we don't have in test data
        TrackSeedList=["SPEFit2","SPEFitSingle"],
        BareMuTimingSpline=timingSplinePath,
        BareMuAmplitudeSpline=amplitudeSplinePath,
        StochTimingSpline=stochTimingSplinePath,
        StochAmplitudeSpline=stochAmplitudeSplinePath,
        EnergyEstimators=EnEstis)

# is anyone still using FlatNtuple?
# # write a simple root file
#tray.AddModule("I3FlatNtupleModule","flatnt")(
#         ("Outfile",outrootfile_flat),
#         ("EventHeader","I3EventHeader"),
#         ("MCTruthName","I3MCTree"),#2007
#         ("MCWeightsName","I3MCWeightDict"),
#         ("TreeName","Level3"),
#         ("FavoriteFit","SplineMPEmaxMuEXSeed"),
#         ("FavoritePulses",pulses),
#         ("BookTracks",1),
#         ("BookMuons",1),
#         ("BookNDirect",1),
#         ("BookSkyCoords",0),
#         ("BookContainment",0),
#         )

tray.AddModule('I3Writer', 'writeri3',
               FileName=outputfile,
               Streams=[icetray.I3Frame.Physics,icetray.I3Frame.DAQ])

# Stop execution of module chain


##################################################################
# Execute Tray and finish
##################################################################

#tray.Execute(100)
tray.Execute()


