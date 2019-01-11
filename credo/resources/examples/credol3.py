#!/usr/bin/env python
"""
 id     : $Id: credol3.py 107982 2013-07-09 19:16:29Z nwhitehorn $
 version: $Revision: 107982 $
 date   : $Date: 2013-07-09 15:16:29 -0400 (Tue, 09 Jul 2013) $
 author : emiddell
 script to run credo on the l3 test files created by Sebastian
"""

from I3Tray import *

import numpy as n
import os, sys, datetime, glob
from stat import *
from os.path import expandvars
from math import pi, log10
from optparse import OptionParser

usage = """usage: %prog [options] <infiles>
infile is the i3 file to process. 
"""
parser = OptionParser(usage=usage)
parser.add_option("-g", "--gcd", dest="gcdfile", type="str", default="", help="path to the gcdfile")
parser.add_option("-b", "--badomlist", dest="baddomlist", type="str", default="", help="path to the baddomlist.txt")
parser.add_option("-o", "--outputprefix", dest="outputprefix", type="str", default="credol3", help="prefix of all outputfiles")
parser.add_option("-n", "--nugen", dest="nugen", action="store_true",
                          default=False, help="this is a neutrino-generator dataset. look for corresponding mcweightdict.")
parser.add_option("-i", "--iterations", dest="iterations", type="int",
                          default=0, help="number of iterations")
parser.add_option("-x", "--summary", dest="summary", action="store_true",
                          default=False, help="output json summary")
parser.add_option("-e", "--nevents", dest="nevents", type="int",
                          default=0, help="number of events to process")
parser.add_option("-m", "--mincharge", dest="mincharge", type="float",
                          default=0.0, help="mincharge parameter of credo llh")
parser.add_option("-y", "--highestenergy", dest="highestenergy", type="float",
                          default=0, help="upper energy boundary")
parser.add_option("-s", "--domsaturation", dest="domsaturation", type="float",
                          default=1e6, help="dom saturation limit")
parser.add_option("-p", "--photonicssaturation", dest="photonicssaturation", action="store_true",
                          default=False, help="detect photonics saturation")
parser.add_option("-w", "--write", dest="writer", action="store_true",
                          default=False, help="write physics frame to i3 files")
parser.add_option("-a", "--atwdonly", dest="atwdonly", action="store_true",
                          default=False, help="only use the atwd in the reconstruction")
parser.add_option("-E", "--useEmptyPulses", dest="useemptypulses", action="store_true",
                          default=False, help="fill gaps in the pulsemap with empty pulses")

opts, args = parser.parse_args()
if len(args) < 1:
    parser.error("incorrect numbers of arguments")

print("begin:", datetime.datetime.now())

# icetray libs
from I3Tray import *
load("libicetray")
load("libdataclasses")
load("libdataio")
load("libphys-services")
load("libgulliver")
load("libgulliver-modules")
load("libicepick")
load("libphotonics-service")
load("libcredo")
load("libparticleforge")
load("libhdf-writer")
load("libpulse-splitter")
load("libFeatureExtractor")

from icecube.simprod.util import ReadI3Summary, WriteI3Summary
from icecube import dataclasses

################################################################################
# VARIABLES
################################################################################


launchname="HLCInIceRawData"
pulsename = "CredoFeatureExtractedPulses" # the final RecoPulseSeries that is used
firstguessname="ParticleForge"

noiserate = 700.*I3Units.hertz

baddomlist = [OMKey(int(j[0]), int(j[1])) for j in [i.strip().split(".") for i in open(opts.baddomlist, "r").readlines()]]

llhpulsename = pulsename

#####################################################################
# DATAFILES
#####################################################################

inputFiles= args[:]
outprefix = opts.outputprefix

outFile        = outprefix + ".i3.gz"
summaryfile = outprefix + ".json" 
hdf_output     = outprefix + ".h5"

filenamelist = [opts.gcdfile] + inputFiles

print("found the following files to process:")

problems = False
for f in filenamelist:
    print(f+"....")
    if os.access(f,os.R_OK) == False:
        print("not readable!")
        problems = True
    else:
        print("ok!")
        
if problems:
    print("problems with given files. see above!",f,"exiting.")
    sys.exit(1)


#####################################################################
# PHOTONICS
#####################################################################

tabledir = "/afs/ifh.de/user/m/middell/scratch/data/photonicstables/AHAv1"
driverpath = "listfiles_AHA07v1ice/I3Coord_I3Span_z80_a20"
driverfile = "level2_shower_gauss_photorec.list"

#####################################################################
# BOOT INTO ICETRAY
#####################################################################
tray = I3Tray()

#####################################################################
# SERVICES
#####################################################################

tray.AddModule("I3Reader", "reader")(
   ("filenamelist", filenamelist)
   )

tray.AddService("I3PhotonicsServiceFactory","photonics")(
    ("PhotonicsTopLevelDirectory", tabledir),
    ("DriverFileDirectory",  tabledir+"/"+driverpath),
    #("PhotonicsLevel1DriverFile",driverfile),
    ("PhotonicsLevel2DriverFile",driverfile),
    ("UseDummyService",False ),
    ("PhotonicsTableSelection",2)
    )

tray.AddService("I3BasicSeedServiceFactory","seedprep")(
    ("InputReadout", pulsename),
    ("TimeShiftType", "TNone"),
    ("NChEnergyGuessPolynomial", [1.7, 0., 0.7, 0.03]),  # parametrization for IC80, dataset 445
    #("FixedEnergy", 100.0*I3Units.TeV),
    ("FirstGuess", firstguessname))

tray.AddService("I3GSLRandomServiceFactory","I3RandomService")

tray.AddService("I3GulliverMinuitFactory","minuit")(
        ("MaxIterations",10000),
        #("MaxIterations",2000),
        ("Tolerance",0.1),
        ("Algorithm","SIMPLEX")
        )


# pulse-based likelihood
tray.AddService("I3PoissonGaussLogLikelihoodFactory","likelihood")(
    ("EventHypothesis", "Cascade"),
    ("InputPulses",llhpulsename),
    ("NoiseRate", noiserate ),
    ("SkipWeights", False),
    ("PDF","PhotonicsService"),
    ("EventLength", -1),
    ("GaussianErrorConstant", 1000),
    ("UseBaseContributions", True),
    ("MinChargeFraction", opts.mincharge),
    ("SaturationLimitIceCube", 15000),
    ("PhotonicsSaturation", opts.photonicssaturation),
    ("BadDOMListInFrame", ""), 
    ("BadDOMList", baddomlist),
    ("ATWDOnly", opts.atwdonly),
    ("UseEmptyPulses", opts.useemptypulses)
)

tray.AddService("I3SimpleParametrizationFactory","simpar")(
        ( "StepT", 10.0*I3Units.ns ),
        ( "StepX", 50.0*I3Units.m ),
        ( "StepY", 50.0*I3Units.m ),
        ( "StepZ", 50.0*I3Units.m ),
        ( "StepLogE", .1),
        ( "StepZenith", 0.1*I3Units.radian ),
        ( "StepAzimuth", 0.1*I3Units.radian ),
        ( "BoundsX", [-1200.0*I3Units.m,+1200.0*I3Units.m] ),
        ( "BoundsY", [-1200.0*I3Units.m,+1200.0*I3Units.m] ),
        ( "BoundsZ", [-1200.0*I3Units.m,+1200.0*I3Units.m] ),
        ( "BoundsLogE", [1,12] )
    )

tray.context['I3SummaryService'] = dataclasses.I3MapStringDouble()

 

tray.AddService('I3HDFWriterServiceFactory', 'hdfservice') (
    ('filename', hdf_output))

# ic40 strings
strings_ic40 = [74, 67, 59, 50, 73, 66, 58, 49, 40, 78, 72, 65, 57, 48, 39, 30, 77, 71, 64, 56, 
                47, 38, 29, 21, 76, 70, 63, 55, 46, 75, 69, 62, 54, 45, 68, 61, 53, 44, 60, 52]


selectedstrings = strings_ic40

stringstouse = ",".join(map(str,sorted(strings_ic40)))

#tray.AddService("I3GeometrySelectorServiceFactory", "geo-selector")(
#        ("StringsToUse" , stringstouse),
#        ("StationsToUse", ""),
#        ("ShiftToCenter", False),
#        ("GeoSelectorName", "IC80-NoTop-Geo")
#)


#####################################################################
# MODULE CHAIN
#####################################################################

nevents = 0
def eventcounter(frame):
    " simple event counter "
    global nevents
    nevents += 1
    if (nevents % 100 == 0):
        print("event:", nevents)
    return True

tray.AddModule(eventcounter, "counter")

if opts.nevents:
    tray.AddModule("I3IcePickModule<I3SkipNEventFilter>", "skip")(
        ("discardEvents", True),
        ("SkipNevents", 0),
        ("NeventStopick", opts.nevents)
    )

ffp = 15
if opts.atwdonly:
    ffp = 7

    
tray.AddModule( "I3FeatureExtractor", "Features" ) (
        ( "RawReadoutName",              "HLCInIceRawData" ),      
        ( "CalibratedATWDWaveforms",     "CalibratedATWD" ),       
        ( "CalibratedFADCWaveforms",     "CalibratedFADC" ),       
        ( "InitialHitSeriesReco",        "NotUsedHits" ),
        ( "InitialPulseSeriesReco",      pulsename),    
        ( "DisableHitSeries",            True ),                   
        ( "MaxNumHits",                  0 ),                      
        ( "FastPeakUnfolding",           0 ),                      
        ( "FastFirstPeak",               ffp), 
        ( "ADCThreshold",                1.1),          
        ( "MaxSPEWidth",                 20 ),                     
        ( "MinSPEWidth",                 4 ),                      
#        ( "PMTTransit",                  2 ),                      
        ( "UseNewDiscThreshold",         False ))


tray.AddModule("I3EventCounter","EventCounter")(
        ( "CounterStep",               5 ),                        # Default
        ( "Dump",                      False ),                      # Default
        #( "NEvents",                   NumEvents ),                  # ! Set according to skript option
        ( "EventHeaderName",           "I3EventHeader" ),            # Default
        ( "PhysicsCounterName",        "InputEvents" ),              # ! Name of counter for summary
        ( "GeometryCounterName",       "" ),                         # Default
        ( "CalibrationCounterName",    "" ),                         # Default
        ( "DetectorStatusCounterName", "" )                          # Default
    )
tray.AddModule("I3ParticleForgeModule","mcforge1")(
        ("Shape",    "cascade"),
        ("Time",     "mc"),
        ("Position", "mc"),
        ("Direction","mc"),
        ("Energy",   "mc"),
        ("MCTree",   "I3MCTree"),
        ("MCMethod", "ReferenceCascade_Visible"),
        ("output",   "CredoRefCascade")
)

tray.AddModule("I3ParticleForgeModule","mcforge2")(
        ("Shape",    "cascade"),
        ("Time",     "mc"),
        ("Position", "mc"),
        ("Direction","mc"),
        ("Energy",   "mc"),
        ("MCTree",   "I3MCTree"),
        ("MCMethod", "MostenErgeticCascade"),
        ("output",   "CredoMECascade")
)

#
# gulliver likelihood fit
#

# iterative fit

if opts.iterations > 0:
    tray.AddModule("I3IterativeFitter")(
        ("OutputName","credofit"),
        ("SeedService","seedprep"),
        ("RandomService","SOBOL"),
        ("Parametrization","simpar"),
        ("LogLikelihood","likelihood"),
        ("Minimizer","minuit"),
        ("NIterations", opts.iterations)
    )
else:
    tray.AddModule("I3SimpleFitter","credofit")(
            ("OutputName","credofit"),
            ("SeedService","seedprep"),
            ("Parametrization","simpar"),
            ("LogLikelihood","likelihood"),
            ("Minimizer","minuit"),
    )

# hdf output


tray.AddModule('I3HDFWriterOldStyle<I3GeometryToTable >', 'geodump')(
    ('tablename', 'geometry'))
tray.AddModule('I3HDFWriterOldStyle<I3EventHeaderToTable >', 'header')(
    ('tablename', 'header'))
tray.AddModule('I3HDFWriterOldStyle<EventInfoToTable<I3RecoPulse > >', 'eventinfo')(
    ('tablename', 'eventinfo_pulses'),
    ('key', pulsename))
tray.AddModule('I3HDFWriterOldStyle<DOMInfoToTable<I3RecoPulse > >', 'dominfo')(
    ('tablename', 'dominfo_pulses'),
    ('key', pulsename))
tray.AddModule('I3HDFWriterOldStyle<I3MCTreeToTable >', 'mctree')(
    ('tablename', 'mctree'),
    ('key', 'I3MCTree'))
tray.AddModule('I3HDFWriterOldStyle<I3MCWeightDictToTable >', 'nugenweightdict')(
    ('tablename', 'mcweights'),
    ('key', 'I3MCWeightDict'))

for particle in [firstguessname, "CredoMECascade", "CredoRefCascade", "credofit"]:
    tray.AddModule('I3HDFWriterOldStyle<I3ParticleToTable >', '%s2table' % particle)(
        ('tablename', particle),
        ('key', particle))
    

tray.AddModule('I3HDFWriterOldStyle<I3LogLikelihoodFitParamsToTable >', 'llhparams2table')(
    ('tablename', 'credollhparams'),
    ('key', 'credofitFitParams'))


if opts.writer:
    tray.AddModule("I3Writer","writer")(
        ("filename", outFile),
        ('streams', [icetray.I3Frame.Physics]),
        )




# do it
tray.Execute()


if opts.summary:
        summary = tray.context['I3SummaryService']
        WriteI3Summary(summary, summaryfile)

print("end:", datetime.datetime.now())
