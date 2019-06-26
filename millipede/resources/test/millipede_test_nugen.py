#!/usr/bin/env python

import sys,os.path
from I3Tray import *
from icecube import dataio,dataclasses,photonics_service
from icecube.millipede import HighEnergyExclusions
from monopod_test import get_pxs


try:
    pxs = get_pxs()
except:
    print("Can't find full-size spline tables, skipping test")
    sys.exit(0)

millipede_param_attrs = [
    'chi_squared','chi_squared_dof','logl', 'ndof', 'nmini',
    'predicted_qtotal','qtotal', 'rlogl','squared_residuals'
]
Suffix="HV"
testdir = os.environ["I3_TESTDATA"]
files = ["GCD/GeoCalibDetectorStatus_2012.56063_V0.i3.gz",
         "sim/Level3_nugen_numu_IC86.2012.011069.000000_20events.i3.bz2"]
filelist = [os.path.join(testdir, f) for f in files]

tray = I3Tray()

tray.AddModule("I3Reader","reader",
               FileNameList  = filelist
               )

exclusionsHE=tray.AddSegment(
    HighEnergyExclusions, "excludes_high_energies",
    Pulses="Millipede"+Suffix+"SplitPulses",
    ExcludeDeepCore="DeepCoreDOMs",
    ExcludeSaturatedDOMs=False,
    ExcludeBrightDOMS="BrightDOMs_TEST",
    BrightDOMThreshold=10,
    SaturationWindows="SaturationWindows",
    BadDomsList="BadDomsList",
    CalibrationErrata="CalibrationErrata")
exclusionsHE.append("Millipede"+Suffix+"SplitPulsesExcludedTimeRange")

tray.AddModule("MuMillipede", "millipede_highenergy_mie",
               MuonPhotonicsService=None,
               CascadePhotonicsService=pxs,
               PhotonsPerBin=15,
               MuonSpacing=0,
               ShowerSpacing=10,
               ShowerRegularization=1e-9,
               MuonRegularization=0,
               SeedTrack="SplineMPE",
               Output="SplineMPE_MillipedeHighEnergyMIE_TEST",
               ReadoutWindow="Millipede"+Suffix+"SplitPulsesReadoutWindow",
               ExcludedDOMs=exclusionsHE,
               DOMEfficiency=0.99,
               Pulses="Millipede"+Suffix+"SplitPulses")

def rel_diff(p1,p2,attr):

    a1 = getattr(p1,attr)
    a2 = getattr(p2,attr)
    if a1 > 0:
        return abs(a1-a2)/a1
    else:
        return 0.0

n_particles= 0
n_differences = 0

def test(frame):
    global n_particles,n_differences
    if "SplineMPE_MillipedeHighEnergyMIE" in frame:
        for i in range(len(frame["SplineMPE_MillipedeHighEnergyMIE"])):
            n_particles+=1
            r = rel_diff(
                frame["SplineMPE_MillipedeHighEnergyMIE"][i],
                frame["SplineMPE_MillipedeHighEnergyMIE_TEST"][i],
                "energy"
            )
            e1 = frame["SplineMPE_MillipedeHighEnergyMIE"][i].energy
            if e1> 10*I3Units.GeV and r > 0.01: 
                print(i,e1,frame["SplineMPE_MillipedeHighEnergyMIE_TEST"][i].energy,r)
                n_differences +=1
                        
        param1 = frame['SplineMPE_MillipedeHighEnergyMIEFitParams']
        param2 = frame['SplineMPE_MillipedeHighEnergyMIE_TESTFitParams']

        for attr in millipede_param_attrs:
            r = rel_diff(param1,param2,attr)
            assert (r < 0.01)

tray.AddModule(test)

tray.Execute()


r = float(n_differences)/n_particles
print("particles:",n_particles,"differences:",n_differences,r)
assert r < 0.01

