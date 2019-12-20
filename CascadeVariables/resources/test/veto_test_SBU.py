#!/usr/bin/env python
#i###############################################################################
# test script that prints hit info and compares it to the results of I3VetoModule
# copied from veto_test.py 
# updated on Dec/18/2019 by Zelong Zhang <zelong.zhang.1@stonybrook.edu>
################################################################################

from __future__ import print_function
import sys, os, os.path, datetime
from os.path import join
from optparse import OptionParser

print("begin:", datetime.datetime.now())

from I3Tray import *

from icecube import CascadeVariables, dataio, icetray, phys_services, dataclasses
#from icecube import cscd_containment

from icecube.tableio import I3TableWriter
from icecube.hdfwriter import I3HDFTableService

#####################################################################
# DATAFILES
#####################################################################

parser = OptionParser()
usage = """%prog [options]"""
parser.set_usage(usage)
parser.add_option("-g", "--gcd", action="store", type="string", 
          default="/data/ana/Cscd/IC86-8/level3/exp/2018/0902/Run00131452/Level2_IC86.2018_data_Run00131452_74_411_GCD.i3.zst",
		  dest="GCD", help="GCD file for input i3 file")
parser.add_option("-i", "--input", action="store", type="string", 
          default="/data/ana/Cscd/IC86-8/level3/exp/2018/0902/Run00131452/Level3_IC86.2018_data_Run00131452_Subrun00000000_00000000.i3.zst", 
		  dest="INPUT", help="Input i3 file to process")
parser.add_option("-p", "--pulse-series", action="store", type="string", default="SplitInIcePulses",
		  dest="PULSES", help="Pulse series to use")
parser.add_option("-o", "--output", action="store", type="string", default="output.hd5",
		  dest="OUTPUT", help="Output HDF5 file")

# get parsed args
(options,args) = parser.parse_args()

#####################################################################
# DEFINE OWN MODULES
#####################################################################

def printDOMinfo(frame):
    if not options.PULSES in frame:
        return True
        
    print("\n===== STARTING NEW EVENT =======================")
    # somehow loop over DOMs in the pulseSeries
    recoPulseSeries = dataclasses.I3RecoPulseSeriesMap.from_frame(frame, options.PULSES)
    for omkey in recoPulseSeries.keys():
        charge = sum( i.charge for i in recoPulseSeries[omkey] )
        startTime = min( i.time for i in recoPulseSeries[omkey] )
        endTime = max( i.time for i in recoPulseSeries[omkey] )
        print("Hit on OM %2d on string %2d - Start Time: %5d - End Time: %5d - Total charge: %.2f" % (omkey.om, omkey.string, startTime, endTime, charge))
        
    return True


def printVeto(frame):
    if not "Veto" in frame:
        return True

    veto = frame["Veto"]
    print("Results of I3Veto ==============================")
    print("Earliest Hit")
    print("  Layer: %2d OM no.: %2d Containment variable: %2d" % (veto.earliestLayer,veto.earliestOM,veto.earliestContainment))
    print("Latest Hit")
    print("  Layer: %2d OM no.: %2d Containment variable: %2d" % (veto.latestLayer,veto.latestOM,veto.latestContainment))
    print("Max Charge Hit")
    print("  Layer: %2d OM no.: %2d Containment variable: %2d" % (veto.maxDomChargeLayer,veto.maxDomChargeOM,veto.maxDomChargeContainment))

    return True


#####################################################################
# BOOT INTO ICETRAY
#####################################################################

tray = I3Tray()

#####################################################################
# SERVICES
#####################################################################

hdfService = I3HDFTableService(options.OUTPUT)

hdfKeys = ["I3EventHeader",
           #
           # reco pulses
           options.PULSES,
           #
           # fits
           #
           # fit parameters
           #
           # doubles
           #
           # containment veto
           "Veto",
           #
           # bools
           #
           # feel free to extend this list!
           ]

#####################################################################
# MODULE CHAIN
#####################################################################

tray.AddModule('I3Reader','reader',
               filenamelist = [options.GCD, options.INPUT])


tray.AddModule("I3VetoModule", "veto", 
               HitmapName=options.PULSES,
               OutputName="Veto",
               DetectorGeometry=86, 
               useAMANDA=False,
               FullOutput=True)


tray.AddModule(printDOMinfo, 'DOMinfo')


tray.AddModule(printVeto, 'VetoInfo')


tray.AddModule(I3TableWriter,'writer',
               tableservice = hdfService,
               keys         = hdfKeys,
               SubEventStreams = ['BaseProc_splittrigs'])




# do it
tray.Execute(20)


print("end:", datetime.datetime.now())
