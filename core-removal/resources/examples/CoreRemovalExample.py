#!/usr/bin/env python

#
#   To run execute:  python CoreRemoval.py
#
# Description:  This example script takes in OfflinePulses from an IC86-11 
#   data file that has passed the cascade filter, uses the 
#   CscdL3_Credo_SpiceMie vertex finder and splits the OfflinePulses into
#   CorePulses (close to vertex) and CoronaPulses (away from vertex).
#   
#   The script then uses the I3CscdLlhModule to find a better vertex using only
#   the CorePulses and then only the CoronaPulses.
#
#   The output is an i3 file with the CorePulses, CoronaPulses, I3CoreCscdLlh
#   and I3CoronaCscdLlh saved.
#


import sys
from I3Tray import *
from icecube import dataio, tableio, phys_services
from icecube import icetray, dataclasses, simclasses
from icecube import cscd_llh, core_removal, clast

#   Input files
gcdfile = ["/data/exp/IceCube/2011/filtered/level2/0513/Level2_IC86.2011_data_Run00118177_0513_GCD.i3.gz"]
infile  = ["/data/ana/Cscd/IC86-1/level3a/exp/2011/0513/Level3a_Cscd_IC86.2011_data_Run00118175_000000.i3.bz2"]

files = gcdfile + infile 

tray = I3Tray( )

tray.AddModule( "I3Reader", "reader", FileNameList = files ) 

# First guess vertex/direction finding algorithm for cascades.
tray.AddModule( "I3CLastModule", "CLast", InputReadout = "OfflinePulses",
                                          Name         = "CFirst" )

#   I3CascadeFitCoreRemoval takes in pulse series (OfflinePulses) and a vertex 
#   fitter (CscdL3_Credo_SpiceMie) and splits the pulse series into CorePulses
#   and CoronaPulses.  CorePulses constitute pulse series of DOMs close to the
#   reconstructed vertex.  The pulses not part of the core are stored in 
#   CoronaPulses.
#
#   Note that the vertex fitter is important.  If one uses CLast, to get a 
#   rough vertex reconstruction, many events will not have any pulses at a
#   distance shorter than the radius (SPERadius) that that determines if a 
#   pulse is part of the CorePulses.  If an event doesn't have CorePulses,
#   I3CscdLlhModule which uses RecoSeries = "CorePulses"  will throw a warming 
#   saying:
#       I3CoreCscdLlh: Event# xxx: Unable to add RecoPulses!
#
#   Use CscdL3_Credo_SpiceMie vertex fitter, if available.
#
#   Also note setting VertexName to a track fitter (e.g.: LineFit, SPEFit, etc)
#   will produce similar warnings by I3CoreCscdLlh.


tray.AddModule( "I3CascadeFitCoreRemoval", "CoreRemoval",
    InputRecoPulseSeries    = "OfflinePulses", 
    SPERadiusName           = "I3SPERadius",
    VertexName              = "CscdL3_Credo_SpiceMie",
    #VertexName              = "CFirst", # Doesn't perform as well as above.
    CorePulsesName          = "CorePulses",
    OutputRecoPulseSeries   = "CoronaPulses" )


# Calculates vertex location using CorePulses only
tray.AddModule( "I3CscdLlhModule", "CoreCscdLlh",
    InputType               = "RecoPulse",
    RecoSeries              = "CorePulses",
    SeedKey                 = "CFirst",
    ResultName              = "I3CoreCscdLlh",
    MinHits                 = 3,
    PDF                     = "UPandel",
    Minimizer               = "Powell",
    EnergySeed              = 2,
    ParamX                  = "1., 0, 0, false",
    ParamY                  = "1., 0, 0, false",
    ParamZ                  = "1., 0, 0, false",
    ParamT                  = "1., 0, 0, false" )
    

# Calculates vertex location using CoronaPulses only.
tray.AddModule( "I3CscdLlhModule", "CoronaCscdLlh",
    InputType               = "RecoPulse",
    RecoSeries              = "CoronaPulses",
    SeedKey                 = "CFirst",
    ResultName              = "I3CoronaCscdLlh",
    MinHits                 = 3,
    PDF                     = "UPandel",
    Minimizer               = "Powell",
    EnergySeed              = 2,
    ParamX                  = "1., 0, 0, false",
    ParamY                  = "1., 0, 0, false",
    ParamZ                  = "1., 0, 0, false",
    ParamT                  = "1., 0, 0, false" )

# Output to save.
keep_keys = [ "I3CoreCscdLlh", "I3CoronaCscdLlh", "CoronaPulses", "CorePulses" ]

tray.AddModule( "Keep", "KeepKeys", keys = keep_keys )
tray.AddModule( "I3Writer", "Writer", FileName = "test.i3.bz2" )

tray.Execute( )
