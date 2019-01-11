#!/usr/bin/env python
################################################################################
# script to run credo on the testdata
################################################################################
from I3Tray import *

import os, sys, datetime, glob

try:
    import numpy as n
except:
    print("Failed to load numpy. Skipping this test.")
    sys.exit(0)
    
from stat import *
from math import pi, log10

# icetray libs
from I3Tray import *
load("libphys-services")
load("libgulliver")
load("libgulliver-modules")
load("liblilliput")
load("libphotonics-service")
load("libclast")

from icecube import icetray, dataclasses, dataio, credo

################################################################################
# VARIABLES
################################################################################


pulsename = "MaskedOfflinePulses" # the final RecoPulseSeries that is used
firstguessname="ParticleForge"

noiserate = 700.*I3Units.hertz

#####################################################################
# DATAFILES
#####################################################################

testdata = os.path.expandvars("$I3_TESTDATA")
inputfile = "event-viewer/Level3aGCD_IC79_EEData_Run00115990.i3"
filenamelist  = [os.path.join(testdata, inputfile)] 

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
    ("UseDummyService",True ),
    )

tray.AddService("I3BasicSeedServiceFactory","seedprep")(
    ("InputReadout", pulsename),
    ("TimeShiftType", "TNone"),
    ("NChEnergyGuessPolynomial", [1.7, 0., 0.7, 0.03]),  # parametrization for IC80, dataset 445
    ("FirstGuess", "forgedseed"))

tray.AddService("I3GSLRandomServiceFactory","I3RandomService")

tray.AddService("I3GulliverMinuitFactory","minuit")(
        ("MaxIterations",10000),
        ("Tolerance",0.1),
        ("Algorithm","SIMPLEX")
        )

# pulse-based likelihood
tray.AddService("I3PoissonGaussLogLikelihoodFactory","likelihood")(
    ("InputPulses",pulsename),
    ("NoiseRate", noiserate ),
    ("PDF","I3PhotonicsService"),
    ("EventLength", -1),
    ("GaussianErrorConstant", 1000),
    ("UseBaseContributions", True),
    ("MinChargeFraction", 0),
    ("SaturationLimitIceCube", 0),
    ("PhotonicsSaturation", 0),
    ("BadDOMListInFrame", ""), 
    ("BadDOMList", []),
    ("ATWDOnly", False),
    ("UseIC40Correction", False),
    ("LightScale", 1.0 ),
    ("UseEmptyPulses", False)
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


#####################################################################
# MODULE CHAIN
#####################################################################

tray.Add("I3CLastModule", Name="forgedseed", InputReadout=pulsename, MinHits=8)

#
# gulliver likelihood fit
#

# iterative fit

tray.AddModule("I3IterativeFitter")(
        ("OutputName","credofit_iter"),
        ("SeedService","seedprep"),
        ("RandomService","SOBOL"),
        ("Parametrization","simpar"),
        ("LogLikelihood","likelihood"),
        ("Minimizer","minuit"),
        ("NIterations", 3)
)

tray.AddModule("I3SimpleFitter")(
        ("OutputName","credofit_single"),
        ("SeedService","seedprep"),
        ("Parametrization","simpar"),
        ("LogLikelihood","likelihood"),
        ("Minimizer","minuit"),
)
        
def printresult(frame, key):
    prt = frame[key]
    print( "%s | vertex: (%.1f, %.1f, %.1f %.1f), dir: (%.1f, %.1f), energy: %.1f, status: %s" % (key, prt.time,
           prt.pos.x, prt.pos.y, prt.pos.z, prt.dir.zenith, prt.dir.azimuth, prt.energy, str(prt.FitStatus)))
    return True

def printdiag(frame, key):
    if key in frame:
        for i in frame[key].cacheMap:
            omkey = i.key()
            domcache = i.data()
            print("%s amp_recorded=%.2f amp_predicted=%.2f amp_correction=%.2f" % (str(omkey), domcache.npe, domcache.expectedAmpliude, domcache.amplitudeCorrection))

tray.AddModule(printresult, "printer1", key="forgedseed")
tray.AddModule(printresult, "printer2", key="credofit_iter")
tray.AddModule(printresult, "printer3", key="credofit_single")
tray.AddModule(printdiag, "printer4", key="credodiagnostics")

tray.Execute(10)


