#!/usr/bin/env python
'''
NuGen example

*** July 11 2017 K.Hoshina
If you set FLAVOR parameter, this script uses legacy I3NuGInjector service.
The new setting TYPES and RATIOS gives more freedom of injection particles.
For example, if TYPES is NuMu:NuMuBar:NuTau:NuTauBar and RATIO is 1:1:0.5:0.5, it injects NuMu, NuMuBar, NuTau, NuTauBar with ratio of [1:1:0.5:0.5]. 
By changing the ratio from 1, you must use OneWeightPerType instead of OneWeight.
See details at
http://software.icecube.wisc.edu/documentation/projects/neutrino-generator/weightdict.html#weights

Example usages:

1) Generate standard NuMu:NuMuBar(1:1) dataset with energy range 10^2 to 10^7, E^-2 spectrum with Earth propagation

$ python NuGen_trunk.py -f NuMu -g 2 -e 2:7 -s Full

2) Generate NuE:NuEBar generation ratio 1:3, energy range 10^2 to 10^8, E^-1.5 spectrum without Earth propagation

$ python NuGen_trunk.py -T NuE:NuEBar -R 1.0:3.0 -g 1.5 -e 2:8 -s Detector

3) Generate NuTau only dataset with energy range 10^2 to 10^8, zenith range 90 to 180 deg, E^-1.5 spectrum, no Earth propagation, using Carlos's differential cross section file

$ python NuGen_trunk.py -T NuTau -R 1.0 -g 1.5 -e 2:8 -z 90:180 -s Detector -d 1 -x carlos -D path_to_xsec_dir

'''

from I3Tray import *

from os.path import expandvars

from icecube import icetray, dataclasses, phys_services, sim_services, dataio,  earthmodel_service, neutrino_generator
from icecube import tableio, hdfwriter

#from manage_seeds import load_seed, save_seed

import os
import sys


#
# for logging
#
icetray.I3Logger.global_logger = icetray.I3PrintfLogger()
icetray.set_log_level(icetray.I3LogLevel.LOG_WARN)
#icetray.set_log_level_for_unit("EarthModelService",icetray.I3LogLevel.LOG_TRACE)
#icetray.set_log_level_for_unit("I3NuG",icetray.I3LogLevel.LOG_TRACE)
icetray.set_log_level_for_unit("ZenithSampler",icetray.I3LogLevel.LOG_DEBUG)
icetray.set_log_level_for_unit("I3NuG",icetray.I3LogLevel.LOG_DEBUG)
#icetray.set_log_level_for_unit("I3NuG",icetray.I3LogLevel.LOG_INFO)

#----------------
# default params
#----------------

# earthmodel-service
earth = ["PREM_mmc"]
material = ["Standard"]

# icecap model
# default is now SimpleIceCap. You need to set IceSheet explicitly 
# if you want to reproduce old simulation.
#icecapmodel = "SimpleIceCap"
icecapmodel = "IceSheet"

#----------------
# arguments
#----------------

from optparse import OptionParser
usage = "usage: %prog [options] inputfile"
parser = OptionParser(usage)

# output params
parser.add_option("-n", "--ngen", type="int", default=100, dest="NGEN", help="number of generated events per file")
parser.add_option("-o", "--outfilebase",default="", dest="OUTFILE", help="output file base name")
parser.add_option("-c", "--compress",default="", dest="COMPRESS", help="suffix for compressed file (gz, bz2)")

# seed params
parser.add_option("-S", "--seed",type="int", default=1234567, dest="SEED", help="seed for random generator") 
parser.add_option("-N", "--nfiles", type="int", default=1, dest="NFILES", help="number of generated file")
parser.add_option("-F", "--fileno", type="int", default=0, dest="FILENO", help="File number (run number)")
parser.add_option("--useseed",default=0, dest="USESEED", help="if you want to use stored seed, set 1 and add GCD file and seed i3 file in args") 

# primary params
parser.add_option("-f", "--flavor",default="", dest="FLAVOR", help="DEPRECATED:flavor of input neutrino")
parser.add_option("-T", "--types",default="NuMu:NuMuBar:NuTau:NuTauBar", dest="TYPES", help="type of input neutrino")
parser.add_option("-R", "--ratios",default="1.0:1.0:0.5:0.5", dest="RATIOS", help="ratio of input neutrino")
parser.add_option("-g", "--gamma",type="float", default=1.0, dest="GAMMA", help="generation gamma factor")
parser.add_option("-e", "--energylog", default="2:8", dest="ELOG", help="energy range in log10, min:max")
parser.add_option("-z", "--zenith", default="0:180", dest="ZENITH", help="zenith range in degrees, min:max")
parser.add_option("-a", "--zenithweight", type="float", default=1.0, dest="ZENITHW", help="zenith weight param: 0.1 - 1.9")
parser.add_option("-A", "--zenithsampling", default="ANGEMU", dest="ZENITHSUMPLING", help="zenith sampling switch")

# siumulation modes
parser.add_option("-s", "--simmode",default="Full", dest="SIMMODE", help="simulation mode: Full, InEarth, Detector")
parser.add_option("-X", "--domuonextension",type="int",default=1, dest="MUEXT", help="0:no extension,1:extend detection volume as a function of energy")
parser.add_option("-p", "--propmode",default="AutoDetect", dest="PROPMODE", help="propagation weight mode: AutoDetect, NoPropWeight, NCGRWeighted")
parser.add_option("-I", "--intfactor",default="1:1:1", dest="INTFACTOR", help="final interaction weight factor, CC:NC:GR")
parser.add_option("-i", "--injectionmode",default="Surface", dest="INJMODE", help="injection mode: Surface, Circle")
parser.add_option("-j", "--injectionrad",type="float", default=1200, dest="INJRAD", help="injection radius for cylinder mode")
parser.add_option("-t", "--distancetoentrance",type="float", default=1200, dest="DENT", help="distanceEntrance for cylinder mode")
parser.add_option("-r", "--detcylrad",type="float", default=950, dest="DETCYLRAD", help="cylinder radius for surface mode")
parser.add_option("-l", "--detcyllen",type="float", default=1900, dest="DETCYLLEN", help="cylinder length for surface mode")
parser.add_option("-u", "--usesimplescatter",type="int", default=0, dest="SIMPLESCATTER", help="use simple scatter")

# cross section
parser.add_option("-x", "--xsecmodel",default="csms_differential_v1.0", dest="XSECMODEL", help="cross section model: csms, cteq5")
parser.add_option("-D", "--xsecdir", default="", dest="XSECDIR", help="cross section dir")
parser.add_option("-d", "--differentialXsec", type="int", default=1, dest="DIFFERENTIAL", help="set 1 to use differential cross section")

(options,args) = parser.parse_args()
print "options", options
print "args", args

# AngleSamplingMode
angmodestr = options.ZENITHSUMPLING
angmode = neutrino_generator.to_angle_sampling_mode(angmodestr)

# flavor and types
flavorString = options.FLAVOR
typeString = options.TYPES
ratioString = options.RATIOS

typevec = typeString.split(":")
ratiostvec = ratioString.split(":")
ratiovec = []
for ratio in ratiostvec:
    ratiovec.append(float(ratio))

# simmode
simmode = options.SIMMODE

# NGen
ev_n = options.NGEN

# gamma index
gamma = options.GAMMA

# minlogE:maxlogE
elogs= (options.ELOG).split(':')
elogmin = float(elogs[0])
elogmax = float(elogs[1])

# zenmindeg:zenmaxdeg
zens= (options.ZENITH).split(':')
zenmin= float(zens[0])*I3Units.degree
zenmax = float(zens[1])*I3Units.degree

# zenith generation weight
# 0.1 to 1.9, larger value gives more virtically upgoing events
# 1.0 gives flat distribution
zenalpha = options.ZENITHW

# cross section
xsecmodel = options.XSECMODEL
xsecdir  = options.XSECDIR

# propagation mode
# for this example I set propmode as AUTODETECT
# to keep all input neutrinos. 
# if you want to simulate CC interaction inside Earth 
# so that some neutrinos will be absorbed,
# set nugen.nopropweight instead.
# AUTODETECT option takes into account of particle flavor,
# if a propagating particle is NuTau it switch off weighted propataion.
propmodestring = options.PROPMODE
propmode = neutrino_generator.to_propagation_mode(propmodestring)

# injection mode
# default is now Surface (old name : Cylinder), which is similar to MuonGun(more efficient).
# You need to set Circle if you want to reproduce old simulation.

injectionmode = options.INJMODE

if injectionmode == "Surface" :
    detcylrad = options.DETCYLRAD*I3Units.m
    detcyllen = options.DETCYLLEN*I3Units.m
    origin_x = 0.*I3Units.m
    origin_y = 0.*I3Units.m
    origin_z = 0.*I3Units.m
    cylinderparams = [detcylrad,detcyllen,origin_x,origin_y,origin_z]

elif injectionmode == "Circle" :
    injectionrad = options.INJRAD
    distanceEntrance = options.DENT
    distanceExit = options.DENT
    cylinderparams = [injectionrad, distanceEntrance, distanceExit]


usesimplescatter = options.SIMPLESCATTER
if usesimplescatter == 0 :
    usesimplescatter = False
else :
    usesimplescatter = True


# random seed
seed = options.SEED

# number of files per dataset
nfiles = options.NFILES

# file ID
fileno  = options.FILENO

# filename of i3 file that includes seed state
useseed = options.USESEED


# MuonRangeExtention
domuonext = True
if options.MUEXT == 0 :
    domuonext = False

# interaction factor
intfct= (options.INTFACTOR).split(':')
intfcc = float(intfct[0])
intfnc = float(intfct[1])
intfgr = float(intfct[2])

digit = len(str(nfiles)) + 1

prefilename = flavorString
if prefilename == "" :
    prefilename = typeString
if options.DIFFERENTIAL != 0 :
    prefilename += "Diff"
if injectionmode == "Circle" :
    prefilename += "Circle"

if options.OUTFILE == "" :
    if (ev_n / 1000 == 0) :
        prefilename = ("%s_N%d" % (prefilename, ev_n))
    elif (ev_n / 1000000 == 0) :
        prefilename = ("%s_N%dK" % (prefilename, ev_n/1000))
    elif (ev_n / 1000000000 == 0) :
        prefilename = ("%s_N%dM" % (prefilename, ev_n/1000000))
    elif (ev_n / 1000000000000 == 0) :
        prefilename = ("%s_N%dB" % (prefilename, ev_n/1000000000))
    formatstr = ("{{0}}_G{{1}}_E{{2}}_Z{{3}}_{{4}}_{{5}}_{{6:0>{0}}}".format(digit))
    options.OUTFILE = (formatstr.format(prefilename, options.GAMMA, options.ELOG, options.ZENITH, xsecmodel, simmode, fileno))

outi3filename = options.OUTFILE + ".i3"
if options.COMPRESS != "" :
    outi3filename = outi3filename + "." + options.COMPRESS
    
print("outfile %s" % outi3filename)
if flavorString != "" :
    print("flavor %s" % flavorString)
else :
    print("Types",  typevec)
    print("Ratio", ratiovec)
print("NGen %d" % ev_n)
print("gamma %f" % gamma)
print("elogmin %f, elogmax %f" % (elogmin, elogmax))
print("zenmin %f, zenmax %f" % (zenmin, zenmax))
print("zenalpha %f" % zenalpha)
print("propmode %s" % propmodestring)
print("simmode %s" % simmode)

#----------------
# generate random service
#----------------

tray = I3Tray()

# generate random service
print ("RandomService params: Seed %d, NFiles %d, FileNo %d" %(seed, nfiles, fileno))
random = phys_services.I3SPRNGRandomService(seed, nfiles, fileno)
tray.context['I3RandomService'] = random

class save_seed(icetray.I3ConditionalModule):
    def __init__(self, ctx):
        icetray.I3ConditionalModule.__init__(self, ctx)
        self.AddOutBox("OutBox")
    def Configure(self) :
        global random
        self.random = random
    def DAQ(self, frame) :
        state = self.random.state
        frame.Put("SPRNGRandomState", state)
        self.PushFrame(frame,"OutBox");
        return True 
    def Finish(self):
        return True

class load_seed(icetray.I3ConditionalModule):
    def __init__(self, ctx):
        icetray.I3ConditionalModule.__init__(self, ctx)
        self.AddOutBox("OutBox")
    def Configure(self) :
        global random
        self.random = random
        pass
    def DAQ(self, frame) :
        frame.Delete("I3EventHeader")
        frame.Delete("I3MCTree")
        frame.Delete("I3MCWeightDict")
        frame.Delete("MCTimeIncEventID")
        frame.Delete("NuGPrimary")
        state = frame["SPRNGRandomState"]
        self.random.state = state
        self.PushFrame(frame,"OutBox");
        return True 
    def Finish(self):
        return True

#----------------
# start simulation
#----------------

from os.path import expandvars

if useseed > 0:
    tray.AddModule("I3Reader", "reader", FilenameList=args)
    tray.AddModule(load_seed, "load_seed")

else :
    tray.AddModule("I3InfiniteSource", "source",
                   prefix = expandvars("$I3_TESTDATA/sim/GeoCalibDetectorStatus_IC86.55380_corrected.i3.gz")
                   )

    tray.AddModule(save_seed, "save_seed")


tray.AddModule("I3MCEventHeaderGenerator","ev",
               IncrementEventID = True)

#
# At least EarthModelService & Steering Service are required
#

tray.AddService("I3EarthModelServiceFactory", "EarthModelService",
                EarthModels = earth,
                MaterialModels = material,
                IceCapType = icecapmodel,
                DetectorDepth = 1948*I3Units.m,
                PathToDataFileDir = "")

tray.AddService("I3NuGSteeringFactory", "steering",
                EarthModelName = "EarthModelService",
                NEvents = ev_n,
                SimMode = simmode,
                VTXGenMode = "NuGen",
                InjectionMode = injectionmode,
                CylinderParams = cylinderparams,
                DoMuonRangeExtension = domuonext,
                UseSimpleScatterForm = usesimplescatter
                )


if flavorString != "" :
    #
    # Old style configuration
    # I3NeutrinoGenerator module generates a 
    # primary particle.
    #
    tray.AddService("I3NuGInjectorFactory", "injector",
                    RandomService = random,
                    SteeringName = "steering",
                    NuFlavor = flavorString,
                    GammaIndex = gamma,
                    EnergyMinLog = elogmin,
                    EnergyMaxLog = elogmax,
                    ZenithMin = zenmin,
                    ZenithMax = zenmax,
                    ZenithWeightParam = zenalpha,
                    AngleSamplingMode = angmodestr
                   )

else :
    #
    # New style configuration
    # Primary particle is generated by 
    # I3NuGDiffuseSource. By default it
    # stores a primary particle with name of 
    # NuGPrimary, and if a particle exists with 
    # this name in frame, I3NeutrinoGenerator 
    # propagates the particle without making 
    # a new primary.
    # (primary name is configuable)
    # You may use I3NuGPointSource either.
    #
    tray.AddModule("I3NuGDiffuseSource","diffusesource", 
                   SteeringName = "steering",
                   NuTypes = typevec,
                   PrimaryTypeRatio = ratiovec,
                   GammaIndex = gamma,
                   EnergyMinLog = elogmin,
                   EnergyMaxLog = elogmax,
                   ZenithMin = zenmin,
                   ZenithMax = zenmax,
                   AzimuthMin = 0,
                   AzimuthMax = 360*I3Units.deg,
                   ZenithWeightParam = zenalpha,
                   AngleSamplingMode = angmodestr
                  )

#
# In both cases you need to add interaction service.
#
if options.DIFFERENTIAL == 0 :
    '''
    for table cross section (LEGACY)
    '''
    tray.AddService("I3NuGInteractionInfoFactory", "interaction",
                RandomService = random,
                SteeringName = "steering",
                TablesDir = xsecdir,
                CrossSectionModel = xsecmodel
               )
else :
    '''
    for differential cross section
    '''
    tray.AddService("I3NuGInteractionInfoDifferentialFactory", "interaction",
                RandomService = random,
                SteeringName = "steering",
                TablesDir = xsecdir,
                CrossSectionModel = xsecmodel
               )

tray.AddModule("I3NeutrinoGenerator","generator",
                RandomService = random,
                SteeringName = "steering",
                InjectorName = "injector",
                InteractionInfoName = "interaction",
                PropagationWeightMode = propmode,
                InteractionCCFactor = intfcc,
                InteractionNCFactor = intfnc,
                InteractionGRFactor = intfgr
              )

tray.AddModule("I3Writer","writer")(
    ("streams", [icetray.I3Frame.DAQ]), 
    ("filename", outi3filename))

tray.AddModule("I3NullSplitter", "fullevent",
                SubEventStreamName = "fullevent")

hdf_outputkeys = ["I3MCWeightDict"]
hdffilename = options.OUTFILE + ".h5"
hdftable = hdfwriter.I3HDFTableService(hdffilename)
tray.AddModule(tableio.I3TableWriter, "hdfwriter")(
              ("tableservice",hdftable),
              ("SubEventStreams",["fullevent"]),
              ("keys",hdf_outputkeys),
              )

tray.AddModule("TrashCan", "the can")

tray.Execute()

