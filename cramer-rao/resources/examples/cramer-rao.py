#!/usr/bin/env python
from I3Tray import *
from os.path import expandvars

import os
import sys

# Retrieve arguments
if len(sys.argv) <= 4:
    print('\nERROR:\nRequired arguments: inputfile gcdfile outputfile datatype\n')
    raise ValueError("ERROR: arguments not passed")
        
inputfile = sys.argv[1]
gcdfile = sys.argv[2]
outfile = sys.argv[3]
datatype = sys.argv[4]
outrootfile = outfile + ".root"
if os.access(outfile,os.R_OK) == True:
    raise RuntimeError("ERROR: output file already exists!")

load("libicetray")
load("libdataclasses")
load("libdataio")
load("libcramer-rao")
load("libanalysis-tree")
load("libparaboloid")

tray = I3Tray()

tray.AddService("I3RootTreeServiceFactory","treeservice")(
    ("TreeFileName",outrootfile))

tray.AddModule("I3Reader","reader")(
    ("Filenamelist", [gcdfile,inputfile])
    )

tray.AddModule("CramerRao","cr")(
    ("InputHits","InitialHitSeriesReco"),
    ("InputTrack","TrackLlhFit"),
    ("OutputResult","CramerRao"),
    ("AllHits",1),
    )

tray.AddModule("I3RootTreeModule","roottree")

if (datatype == "nugen_numu"):
    tray.AddModule("I3MapStringDoubleTreeModule","nugenWeighttree")(
        ("KeyName","I3MCWeightDict")
        )

tray.AddModule("I3MCTreeModule","mctree")(
    ("MCName","I3MCTree")
    )

tray.AddModule("I3Writer","writer")(
    ("filename", outfile)
    )

tray.Execute()
