#!/usr/bin/env python
#
# NuGen InEarth propagation script
# for Old NuGen setup (NuGen v2)
#

from os.path import expandvars
from icecube import icetray, dataclasses, phys_services, sim_services, dataio,earthmodel_service, neutrino_generator
import os
import sys

from I3Tray import *
load("libicetray")

#----------------
# default values
#----------------

from optparse import OptionParser
usage = "usage: %prog [options] inputfile"
parser = OptionParser(usage)

# output params
parser.add_option("-n", "--ngen", type="int", default=100, dest="NGEN", help="number of generated events per file")
parser.add_option("-o", "--outfilebase",default="", dest="OUTFILE", help="output file base name")
parser.add_option("-c", "--compress",default="bz2", dest="COMPRESS", help="suffix for compressed file (gz, bz2)")
parser.add_option("-S", "--seed",type="int", default=1234567, dest="SEED", help="seed for random generator") 
parser.add_option("-N", "--nfiles", type="int", default=1, dest="NFILES", help="number of generated file")
parser.add_option("-F", "--fileno", type="int", default=0, dest="FILENO", help="File number (run number)")

# primary params
parser.add_option("-f", "--flavor",default="", dest="FLAVOR", help="DEPRECATED:flavor of input neutrino")
parser.add_option("-T", "--types",default="NuE:NuEBar:NuMu:NuMuBar:NuTau:NuTauBar", dest="TYPES", help="type of input neutrino")
parser.add_option("-R", "--ratios",default="1.0:1.0:1.0:1.0:1.0:1.0", dest="RATIOS", help="ratio of input neutrino")
parser.add_option("-g", "--gamma",type="float", default=1.0, dest="GAMMA", help="generation gamma factor")
parser.add_option("-e", "--energylog", default="2:9", dest="ELOG", help="energy range in log10, min:max")
parser.add_option("-z", "--zenith", default="90:180", dest="ZENITH", help="zenith range in degrees, min:max")

# cross section
parser.add_option("-x", "--xsecmodel",default="csms_differential_v1.0", dest="XSECMODEL", help="cross section model: csms, cteq5")
parser.add_option("-D", "--xsecdir", default="", dest="XSECDIR", help="cross section dir")
parser.add_option("-d", "--differentialXsec", type="int", default=1, dest="DIFFERENTIAL", help="do you use differential Xsection?")
parser.add_option("-G", "--globalxsecscale", default="1:1:1", dest="GLOBALXSECSCALE", help="global xsec scale")

# EarthModel
parser.add_option("-E", "--earthmodel", default="PREM_mmc", dest="EARTHMODELS", help="list of earth models")

(options,args) = parser.parse_args()

propmode = neutrino_generator.autodetect
simmode = "InEarth"
material = ["Standard"]
angmodestr = "ANG"

# arg 1 : out filename
outi3filename = options.OUTFILE + ".i3"
if options.COMPRESS != "" :
    outi3filename = outi3filename + "." + options.COMPRESS

flavorString = options.FLAVOR
typeString = options.TYPES
ratioString = options.RATIOS

typevec = typeString.split(":")
ratiostvec = ratioString.split(":")
ratiovec = []
for ratio in ratiostvec:
    ratiovec.append(float(ratio))

ev_n = int(options.NGEN)

gamma = float(options.GAMMA)

elogs= (options.ELOG).split(':')
elogmin = float(elogs[0])
elogmax = float(elogs[1])

zens= (options.ZENITH).split(':')
zenmin= float(zens[0])*I3Units.degree
zenmax = float(zens[1])*I3Units.degree

xsecmodel = options.XSECMODEL

earth = (options.EARTHMODELS).split(':')

xsecdir = options.XSECDIR

gscale= (options.GLOBALXSECSCALE).split(':')
global_xsec_scale = [float(gscale[0]), float(gscale[1]), float(gscale[2])]

seed = int(options.SEED)

nfiles = int(options.NFILES)

# arg 13 : file ID
fileno  = int(options.FILENO)


print("outfile %s" % outi3filename)
print("flavor %s" % flavorString)
print("type %s" % typeString)
print("ratio %s" % ratioString)
print("NGen %d" % ev_n)
print("gamma %f" % gamma)
print("elogmin %f, elogmax %f" % (elogmin, elogmax))
print("zenmin %f, zenmax %f" % (zenmin, zenmax))
print("xsec %s" % xsecmodel)
print("earthmodel " , earth)
print("seed %d" % seed)
print("nfiles %d" % nfiles)
print("fileno %d" % fileno)

#--------------
# for logging
#--------------

icetray.I3Logger.global_logger = icetray.I3PrintfLogger()
icetray.set_log_level(icetray.I3LogLevel.LOG_WARN)
icetray.set_log_level_for_unit("EarthModelService",icetray.I3LogLevel.LOG_INFO)
#icetray.set_log_level_for_unit("I3NuG",icetray.I3LogLevel.LOG_TRACE)
#icetray.set_log_level_for_unit("I3NuG",icetray.I3LogLevel.LOG_INFO)
icetray.set_log_level_for_unit("I3NuG",icetray.I3LogLevel.LOG_WARN)

#--------------
# icetray start
#--------------

tray = I3Tray()

#from icecube.examples_simulation.configure_simulation import configure_service_modules
#configure_service_modules( tray, season )
#random = tray.context['random']

# generate random service

print ("RandomService params: Seed %d, NFiles %d, FileNo %d" %(seed, nfiles, fileno))

random = phys_services.I3SPRNGRandomService(seed, nfiles, fileno)
tray.context['I3RandomService'] = random


from os.path import expandvars
tray.AddModule("I3InfiniteSource", "source",
               prefix = expandvars("$I3_TESTDATA/sim/GeoCalibDetectorStatus_IC86.55380_corrected.i3.gz")
               )

tray.AddModule("I3MCEventHeaderGenerator","ev",
               IncrementEventID = True)

#
# At least EarthModelService & Steering Service are required
#

tray.AddService("I3EarthModelServiceFactory", "EarthModelService",
                EarthModels = earth,
                MaterialModels = material,
                IceCapType = "IceSheet",
                DetectorDepth = 1948*I3Units.m,
                PathToDataFileDir = "")

tray.AddService("I3NuGSteeringFactory", "steering",
                EarthModelName = "EarthModelService",
                NEvents = ev_n,
                SimMode = simmode, 
                VTXGenMode = "NuGen", 
                GlobalXsecScaleFactor = global_xsec_scale
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
    #
    tray.AddModule("I3NuGDiffuseSource","diffusesource",
                   SteeringName = "steering",
                   RandomService = random,
                   NuTypes = typevec,
                   PrimaryTypeRatio = ratiovec,
                   GammaIndex = gamma,
                   EnergyMinLog = elogmin,
                   EnergyMaxLog = elogmax,
                   ZenithMin = zenmin,
                   ZenithMax = zenmax,
                   AngleSamplingMode = angmodestr
                  )

#
# In both cases you need to add interaction service.
#
if options.DIFFERENTIAL == 0 :
    # need to set correct energy range that the cross section table supports
    tray.AddService("I3NuGInteractionInfoFactory", "interaction",
                RandomService = random,
                SteeringName = "steering",
                TablesDir = xsecdir,
                CrossSectionModel = xsecmodel
               )
else :
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
                PropagationWeightMode = propmode 
              )

tray.AddModule("I3Writer","writer")(
    ("filename", outi3filename), 
    ("streams", [icetray.I3Frame.DAQ]), 
    ("skipkeys", ["I3MCWeightDict"]), 
    )

class Counter(icetray.I3ConditionalModule) : 
    def __init__(self, ctx):
        icetray.I3ConditionalModule.__init__(self, ctx)
        self.counter_ = 0

    def Configure(self):
        self.counter_ = 0

    def DAQ(self, frame):
        if (self.counter_ % 10000 == 0) :
            print("%d events finished" % self.counter_)
        self.counter_ = self.counter_ + 1
        return True

    def Finish(self):
        return


tray.AddModule(Counter, "counter")



tray.Execute()


