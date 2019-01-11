#!/usr/bin/env python

from I3Tray import *
from os.path import expandvars
from icecube import dataio
import os
import sys
load("libdataclasses")
load("libdataio")
load("libphys-services")
load("libcscd-llh")
load("libclast")

workspace = expandvars("$I3_BUILD")
i3testdata = expandvars("$I3_TESTDATA")
dataf = i3testdata + "/event-viewer/Level3aGCD_IC79_EEData_Run00115990.i3"

tray = I3Tray()

tray.AddModule("I3Reader","reader")
tray.SetParameter("reader", "Filename", dataf)

tray.AddModule("I3CLastModule","cfirst")
tray.SetParameter("cfirst", "MinHits", 10)
tray.SetParameter("cfirst", "DirectHitRadius", 100.0)
tray.SetParameter("cfirst", "InputReadout", "MaskedOfflinePulses")
tray.SetParameter("cfirst", "Name", "CFirst")


tray.AddModule("I3CscdLlhModule", "cscd-llh")

tray.SetParameter("cscd-llh", "InputType", "RecoPulse")
tray.SetParameter("cscd-llh", "RecoSeries","MaskedOfflinePulses")
tray.SetParameter("cscd-llh", "SeedKey", "CFirst")
tray.SetParameter("cscd-llh", "EnergySeed", 2.0)
tray.SetParameter("cscd-llh", "MinHits", 10)
tray.SetParameter("cscd-llh", "AmpWeightPower", 0.0)
tray.SetParameter("cscd-llh", "ResultName", "CscdLlh")

tray.SetParameter("cscd-llh", "Minimizer", "Simplex")
tray.SetParameter("cscd-llh", "MaxCalls", 50000)
tray.SetParameter("cscd-llh", "Tolerance", 0.0001)

#tray.SetParameter("cscd-llh", "ParamT", "50.0, 0.0, 0.0, false")
tray.SetParameter("cscd-llh", "ParamX", "10.0, 0.0, 0.0, false")
tray.SetParameter("cscd-llh", "ParamY", "10.0, 0.0, 0.0, false")
tray.SetParameter("cscd-llh", "ParamZ", "10.0, 0.0, 0.0, false")
#tray.SetParameter("cscd-llh", "ParamEnergy", "1.0, 0.0, 0.0, false")

tray.SetParameter("cscd-llh", "PDF", "HitNoHit")


# PndlHnh
tray.SetParameter("cscd-llh", "PndlHnhWeight", 1.0)

# UPandel
tray.SetParameter("cscd-llh", "PandelSmallProb", 1.0e-6)
tray.SetParameter("cscd-llh", "PandelTau", 450.0)
tray.SetParameter("cscd-llh", "PandelLambda", 47.0)
tray.SetParameter("cscd-llh", "PandelLambdaA", 96.0)
tray.SetParameter("cscd-llh", "PandelSigma", 15.0)
#tray.SetParameter("cscd-llh", "PandelLightSpeed",)
 
tray.SetParameter("cscd-llh", "PandelMaxDist", 0.0)

# Hit/No-hit
tray.SetParameter("cscd-llh", "HitNoHitNorm", 1.4)
tray.SetParameter("cscd-llh", "HitNoHitLambdaAttn", 29.0)
tray.SetParameter("cscd-llh", "HitNoHitNoise", 5.0e-3)
tray.SetParameter("cscd-llh", "HitNoHitDistCutoff", 0.5)
tray.SetParameter("cscd-llh", "HitNoHitDead", 0.05)
tray.SetParameter("cscd-llh", "HitNoHitSmallProb", 1.0e-40)



tray.Execute(10+3)


