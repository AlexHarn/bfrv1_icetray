#!/usr/bin/env python
import os, sys
from os.path import expandvars

from I3Tray import *
from FRTestModule import FRTestModule, FRLiteTestModule, FRMapTestModule
from icecube import dataio, icetray, dataclasses, phys_services, dataio
from icecube import fill_ratio, clast

tray = I3Tray()
tray.Add("I3Reader", "reader",
         FileName=expandvars("$I3_TESTDATA/event-viewer/Level3aGCD_IC79_EEData_Run00115990.i3"),
)

tray.Add("I3CLastModule", "clast", 
         InputReadout="MaskedOfflinePulses")

tray.Add("I3FillRatioModule","fill-ratio",
         VertexName         = "clast",
         RecoPulseName      = "MaskedOfflinePulses",
         ResultName         = "FillRatioTest",
         SphericalRadiusRMS = 3.5)

tray.Add(FRTestModule,"test")

tray.Add(fill_ratio.FillRatioModule,"segment",
         VertexName         = "clast",
         MapName            = "FillRatioMap",
         RecoPulseName      = "MaskedOfflinePulses",
         ResultName         = "FillRatioInfo",
         )

tray.Add(FRMapTestModule,"test2")

tray.AddModule("I3FillRatioLite", "fill-ratio-lite",
               Vertex = "clast",
               Pulses = "MaskedOfflinePulses",
               Output = "FillRatioLiteTest",
               Scale = 2,
               AmplitudeWeightingPower = 1,
               ExcludeDOMs = [icetray.OMKey(2,30),
                              icetray.OMKey(74,3)])

tray.Add(FRLiteTestModule, "test3")



tray.Execute()

