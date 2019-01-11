#!/usr/bin/env python
import os, sys
from optparse import OptionParser
from os.path import expandvars

from I3Tray import *
import icecube
from icecube import dataio, dataclasses, icetray, fill_ratio, clast

#################################################
# i/o options
#################################################
usage = "%prog [options]"
parser = OptionParser(usage=usage)

parser.add_option("-i", "--inputfile",
                  type      = "string",
                  action    = "store",
                  default   = expandvars("$I3_TESTDATA/event-viewer/Level3aGCD_IC79_EEData_Run00115990.i3"),
                  metavar   = "<input file>",
                  help      = "Name of the input file",
                  )
parser.add_option("-g", "--gcdfile",
                  type      = "string",
                  action    = "store",
                  default   = None,
                  metavar   = "<geo file>",
                  help      = "Name of GCD file",
                  )
parser.add_option("-o", "--outputfile",
                  type      = "string",
                  action    = "store",
                  default   = "TEST",
                  metavar   = "<output file(s) name>",
                  help      = "Name of the output file(s). Should include a .i3 at the end.")

(options, args) = parser.parse_args()

#################################################
# Try running with the given input/output
#################################################
files = []
if options.gcdfile: files.append(options.gcdfile)
files.append(options.inputfile)

if not ".i3" in options.outputfile:
  print "No .i3 in outputfile %s. Appending a .i3" % options.outputfile
  options.outputfile += ".i3"

#---------------------------------------------------------------
# Read the files. Nothing exciting here.
#---------------------------------------------------------------
tray = I3Tray()

tray.Add(dataio.I3Reader,"reader",
         FilenameList = files,
         )
#---------------------------------------------------------------
# Calculate a vertex using cfirst. This can actually be any vertex
#  you want. If you have something different (first HLC hit, Monopod,
#  cascade-llh, ect), use that instead and replace the VertexName
#  in the modules below.
#---------------------------------------------------------------
tray.Add("I3CLastModule", Name="CFirst", 
         InputReadout="MaskedOfflinePulses")

#---------------------------------------------------------------
# Adding the module manually
# This is an example which modifies the defaults. In general, you'll
#  want to test some different values yourself in order to see what works
#  best with your events. These values can and likely will change based on
#  your event sample.
#---------------------------------------------------------------
tray.Add("I3FillRatioModule","fill-ratio",
         VertexName = "CFirst", #Set this instance of the FR to use the result of CFirst as its seed
         RecoPulseName = "MaskedOfflinePulses",  #Tell this module to use the feature extracted pulses
         ResultName = "FillRatioInfo",   #Now, give the result a name
         SphericalRadiusRMS = 3.5,  #set the radius of the rms-defined sphere (this is in units of the RMS)
         SphericalRadiusMean = 1.0,  #set the radius of the mean-defined sphere (this is in units of the Mean)
         AmplitudeWeightingPower = 1.0
         )

#---------------------------------------------------------------
# Can also do this as a tray segment. This is MUCH nicer, since you get an I3MapStringDouble out
#  instead of the silly I3FillRatioInfo object, which requires a bit more finagling to work well.
#  This is the PREFERRED way and will be the only supported way into the future!
#  The options match the normal fill-ratio module.
#---------------------------------------------------------------
tray.Add(fill_ratio.FillRatioModule,"segment",
         VertexName         = "CFirst",
         RecoPulseName      = "MaskedOfflinePulses",
         ResultName         = "FillRatioInfo2",
         MapName            = "FillRatioMap2",
         )

#---------------------------------------------------------------
# Write the output to an I3 file
#---------------------------------------------------------------
tray.AddModule( 'I3Writer', 'EventWriter',
                Filename          = options.outputfile,
                Streams           = [icetray.I3Frame.Physics, icetray.I3Frame.DAQ],
                )



tray.Execute()

