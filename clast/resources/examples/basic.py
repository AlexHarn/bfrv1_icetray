#a slightly modified version of .../resources/test/ClastTest.py

from icecube import clast
from I3Tray import I3Tray
import sys

#here take a data file including gcd information
dataf = sys.argv[1]

#here generic outfile name
outf = "clast.i3"

#specify pulses to be used
pulses = "InIcePulses"

tray = I3Tray()

#read in file
tray.Add("I3Reader", Filename=dataf)

#run clast module. Defaults can be found in sphinx docs or using "icetray-inspect clast"
tray.Add("I3CLastModule", InputReadout=pulses, Name="clast") #[defaults: InputReadout=RecoPulses, Name=clast]

#notes: clast (type I3Particle) and clastParams (type I3CLastFitParams) will be written to the frame.

#write file
tray.Add("I3Writer", filename=outf)

tray.Execute(100)
