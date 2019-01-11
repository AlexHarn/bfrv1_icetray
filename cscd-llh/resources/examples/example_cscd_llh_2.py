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

tray.AddModule("I3Reader","reader")(
    ("Filename", dataf)
    )

tray.AddModule("I3CLastModule","cfirst")(
  ("InputReadout","MaskedOfflinePulses"),
  ("Name","CFirst")
  )

#minimizer_collection = ['Brent', 'GfxMinimizer', 'Minuit', 'Powell', 'Simplex']
minimizer_collection = ['Brent', 'Powell', 'Simplex']

#pdf_collection = ['UPandel', 'UPandelMpe', 'HitNoHit', 'HitNoHitMpe', 'PndlHnh', 'HnhDir']
pdf_collection = ['UPandel', 'UPandelMpe', 'PndlHnh']


number=0
for minimizer in minimizer_collection:
    number += 1  
    for pdf in pdf_collection:
        number += 1
        cscd_llh_name = "cscd-llh" + str(number)
        name = "Vertex_Reco_"+minimizer+"_"+pdf
      
        if name == 'Vertex_Reco_Brent_HitNoHit' or name == 'Vertex_Reco_Brent_HitNoHitMpe':
            continue
            print cscd_llh_name
            print name
            tray.AddModule("I3CscdLlhModule", cscd_llh_name)(
              ("InputType","RecoPulse"),
              ("RecoSeries","MaskedOfflinePulses"),
              ("SeedKey","CFirst"),
              ("ResultName", name),
              ("Minimizer",minimizer),
              ("PDF",pdf),
              ("EnergySeed",2.0),
              ("AmpWeightPower", 1.0),
              #("FirstLE", True),
              ("MinHits", 10),
              ("HitNoHitNorm", 1.4),
              ("HitNoHitLambdaAttn", 29.0),
              ("HitNoHitNoise", 5.0e-3),
              ("HitNoHitDistCutoff", 0.5),
              ("HitNoHitDead", 0.05),
              ("HitNoHitSmallProb", 1.0e-40)
              )
        else:
            print cscd_llh_name
            print name
            tray.AddModule("I3CscdLlhModule", cscd_llh_name)(
              ("InputType","RecoPulse"),
              ("RecoSeries","MaskedOfflinePulses"),
              ("SeedKey","CFirst"),
              ("ResultName", name),
              ("Minimizer",minimizer),
              ("PDF",pdf),
              ("EnergySeed",2),
              ("ParamT","0.0, 0.0, 0.0, false"),
              ("ParamX","0.0, 0.0, 0.0, false"),
              ("ParamY","0.0, 0.0, 0.0, false"),
              ("ParamZ","0.0, 0.0, 0.0, false")
              )

            outf = workspace + "/cscd-llh-test-" + cscd_llh_name + ".i3"
            write_name = "write_"+minimizer+"_"+pdf
            tray.AddModule("I3Writer",write_name)(
            ("filename", outf )
            )


tray.Execute(5+3)

