#!/usr/bin/env python
#
# NuGen propagator example
#
#

from I3Tray import *

from os.path import expandvars

from icecube import icetray, dataclasses, phys_services, sim_services, dataio, earthmodel_service, neutrino_generator

import os
import sys

#
# for logging
#
icetray.I3Logger.global_logger = icetray.I3PrintfLogger()
icetray.set_log_level(icetray.I3LogLevel.LOG_WARN)

#icetray.set_log_level_for_unit("EarthModelService",icetray.I3LogLevel.LOG_TRACE)
#icetray.set_log_level_for_unit("I3NuG",icetray.I3LogLevel.LOG_INFO)
#icetray.logging.set_level_for_unit('I3PropagatorModule', 'INFO')

steername = "steering"

#------------------
# params for earthmodel-service
#------------------
earthmodel = ["PREM_mmc"]
materialmodel = ["Standard"]
#icecap = "SimpleIceCap"
icecap = "IceSheet"
icecapangle = 30.*I3Units.degree
detdepth = 1948*I3Units.m

#----------------
# arguments
#----------------

from optparse import OptionParser
usage = "usage: %prog [options] inputfile"
parser = OptionParser(usage)

parser.add_option("-n", "--ngen", type="int", default=100, dest="NGEN", help="number of generated events per file")
parser.add_option("-o", "--outfilebase",default="", dest="OUTFILE", help="output file base name")
parser.add_option("-c", "--compress",default="", dest="COMPRESS", help="suffix for compressed file (gz, bz2)")
parser.add_option("-f", "--flavor",default="NuMu", dest="FLAVOR", help="flavor of input neutrino")
parser.add_option("-g", "--gamma",type="float", default=1.0, dest="GAMMA", help="generation gamma factor")
parser.add_option("-e", "--energylog", default="2:8", dest="ELOG", help="energy range in log10, min:max")
parser.add_option("-z", "--zenith", default="0:180", dest="ZENITH", help="zenith range in degrees, min:max")
parser.add_option("-a", "--zenithweight", type="float", default=1.0, dest="ZENITHW", help="zenith weight param: 0.1 - 1.9")

parser.add_option("-p", "--propmode",default="AutoDetect", dest="PROPMODE", help="propagation weight mode: AutoDetect, NoPropWeight, NCGRWeighted")
parser.add_option("-s", "--simmode",default="Full", dest="SIMMODE", help="simulation mode: Full, InEarth, FinalOnly")
parser.add_option("-x", "--xsecmodel",default="csms", dest="XSECMODEL", help="cross section model: csms, cteq5")
parser.add_option("-i", "--injectionmode",default="Surface", dest="INJMODE", help="injection mode: Surface, Circle")
parser.add_option("-j", "--injectionrad",type="float", default=1200, dest="INJRAD", help="injection radius for cylinder mode")
parser.add_option("-d", "--distancetoentrance",type="float", default=1200, dest="DENT", help="distanceEntrance for cylinder mode")
parser.add_option("-r", "--detcylrad",type="float", default=950, dest="DETCYLRAD", help="cylinder radius for surface mode")
parser.add_option("-l", "--detcyllen",type="float", default=1900, dest="DETCYLLEN", help="cylinder length for surface mode")

parser.add_option("-t", "--seed",type="int", default=1234, dest="SEED", help="seed for random generator")
parser.add_option("-m", "--nfiles", type="int", default=1, dest="NFILES", help="number of generated file")
parser.add_option("-y", "--fileno", type="int", default=0, dest="FILENO", help="File number (run number)")

(options,args) = parser.parse_args()

# arg 1 : flavor
flavorString = options.FLAVOR

# arg 2 : simmode
simmode = options.SIMMODE

# arg 3 : NGen
ev_n = options.NGEN

# arg 4 : gamma index
gamma = options.GAMMA

# arg 5 : minlogE:maxlogE
elogs= (options.ELOG).split(':')
elogmin = float(elogs[0])
elogmax = float(elogs[1])

# arg 6 : zenmindeg:zenmaxdeg
zens= (options.ZENITH).split(':')
zenmin= float(zens[0])*I3Units.degree
zenmax = float(zens[1])*I3Units.degree

# arg 7 : zenith generation weight
# 0.1 to 1.9, larger value gives more virtically upgoing events
# 1.0 gives flat distribution
zenalpha = options.ZENITHW

# arg 8: cross section
xsecmodel = options.XSECMODEL

# arg 9: prop mode
# for this example I set propmode as AUTODETECT
# to keep all input neutrinos. 
# if you want to simulate CC interaction inside Earth 
# so that some neutrinos will be absorbed,
# set nugen.nopropweight instead.
# AUTODETECT option takes into account of particle flavor,
# if a propagating particle is NuTau it switch off weighted propataion.
propmodestring = options.PROPMODE
propmode = neutrino_generator.to_propagation_mode(propmodestring)

# arg 10: injection mode
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
    injectionrad = 1200.*I3Units.m
    distanceEntrance = 1200*I3Units.m
    distanceExit = 1200*I3Units.m
    cylinderparams = [injectionrad, distanceEntrance, distanceExit]

# arg 11: random seed
seed = options.SEED

# arg 12 : number of files per dataset
nfiles = options.NFILES

# arg 13 : file ID
fileno  = options.FILENO

digit = len(str(nfiles)) + 1

if options.OUTFILE == "" :
    formatstr = ("{{0}}_E{{1}}_Z{{2}}_{{3}}_{{4}}_{{5:0>{0}}}".format(digit))
    options.OUTFILE = (formatstr.format(flavorString, options.ELOG, options.ZENITH, xsecmodel, simmode, fileno))

outi3filename = options.OUTFILE + ".i3"
if options.COMPRESS != "" :
    outi3filename = outi3filename + "." + options.COMPRESS
    
print("outfile %s" % outi3filename)
print("flavor %s" % flavorString)
print("NGen %d" % ev_n)
print("gamma %f" % gamma)
print("elogmin %f, elogmax %f" % (elogmin, elogmax))
print("zenmin %f, zenmax %f" % (zenmin, zenmax))
print("zenalpha %f" % zenalpha)
print("propmode %s" % propmodestring)
print("simmode %s" % simmode)

#----------------
# start IceTray
#----------------

tray = I3Tray()

# generate random service
print ("RandomService params: Seed %d, NFiles %d, FileNo %d" %(seed, nfiles, fileno))
random = phys_services.I3SPRNGRandomService(seed, nfiles, fileno)
tray.context['I3RandomService'] = random

from os.path import expandvars
tray.AddModule("I3InfiniteSource", "source",
               prefix = expandvars("$I3_TESTDATA/sim/GeoCalibDetectorStatus_IC86.55380_corrected.i3.gz")
               )

tray.AddModule("I3MCEventHeaderGenerator","ev")

#
# generate earthmodel service
#
earth = earthmodel_service.EarthModelService("EarthModelService","",
                              earthmodel, materialmodel, 
                              icecap, icecapangle, detdepth)
tray.context['EarthModel'] = earth 

#
# generate steering service
#
steer = neutrino_generator.Steering(earth, 
                          neutrino_generator.to_simulation_mode(simmode),
                          neutrino_generator.nugen, 
                          neutrino_generator.to_injection_mode(injectionmode))

steer.cylinder_params = cylinderparams
tray.context[steername] = steer

#
# generate interaction service
#
interaction = neutrino_generator.I3NuGInteractionInfo(random, steer, xsecmodel)
interaction.initialize()
#interaction.view_interaction_info()

#
# generate propagator
#

nugen = neutrino_generator.I3NeutrinoPropagator(random, steer, interaction)
nugen.prop_mode = propmode

# Base propagators for charged leptons
from icecube.sim_services.propagation import get_propagators
from icecube import sim_services

# this function makes propagators for ALL particles!
#propagators = get_propagators()

# For single unit test, use the function...
propagators = sim_services.I3ParticleTypePropagatorServiceMap()

for flavor in 'E', 'Mu', 'Tau':
	for ptype in '', 'Bar':
		propagators[getattr(dataclasses.I3Particle.ParticleType, 'Nu'+flavor+ptype)] = nugen

#
# start icetray
#

# you have to set number of events manually 
# if you use DiffuseSource module

steer.n_gen = ev_n

tray.AddModule("I3NuGDiffuseSource","diffusesource", 
               SteeringName = steername,
               NuFlavor = flavorString,
               GammaIndex = gamma,
               EnergyMinLog = elogmin,
               EnergyMaxLog = elogmax,
               ZenithMin = zenmin,
               ZenithMax = zenmax,
               AzimuthMin = 0,
               AzimuthMax = 360*I3Units.deg,
               ZenithWeightParam = zenalpha
              )

tray.Add('I3PropagatorModule', PropagatorServices=propagators, 
          RandomService=random)

tray.AddModule("I3Writer","writer")(
    ("filename", outi3filename))



tray.Execute()


