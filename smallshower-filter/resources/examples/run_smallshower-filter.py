#!/bin/env python

# Usage example for the project smallshower-filter
#
# This example demonstrates how to use this module to reconstruct small air
# showers. The input is IceTop raw data, the output are showers reconstructed
# with the I3TopLateralFit implemented in toprec.
#
# WARNING: This example is outdated. We now use WaveCalibrator, tpx and
#          topeventcleaning for pulse processing.
#
# Copyright  (C) 2009
# the IceCube Collaboration
# $Id: run_smallshower-filter.py 84525 2012-01-31 14:32:14Z kislat $
#
# $Revision: 84525 $
# $Date: 2012-01-31 09:32:14 -0500 (Tue, 31 Jan 2012) $
# Author: Fabian Kislat <fabian.kislat@desy.de>
#

# Use Pythons option parser to parse the command line options
from optparse import OptionParser

parser.add_option("-i", "--input", help="Input file")
parser.add_option("-g", "--gcd", help="GCD file")
parser.add_option("-o", "--output", help="Output file")
(options, args) = parser.parse_args(sys.argv)

# Load icetray modules
from I3Tray import *
load("libdataio")
load("libDOMcalibrator")
load("libtopwaveprocessor")
load("libtopeventbuilder")
load("libsmallshower-filter")
load("libtoprec")

# Create an IceTray
tray = I3Tray()

# Read the data file using I3Reader
tray.AddModule("I3Reader", "read")(
    ("FileNameList",                [ options.gcd, options.input ])  # First read GCD, then input file
    )

# Use DOMcalibrator to calibrate the raw waveform
tray.AddModule("DOMcalibrator", "calibrate")(
    ("InputRawDataName",            "IceTopRawData"),          # Use IceTopRawData as input
    ("OutputATWDDataName",          "CalibratedIceTopATWD"),   # Name of calibrated IT ATWD waveform
    ("OutputFADCDataName",          "CalibratedIceTopFADC"),   # Name of calibrated IT FADC waveform
    ("OutputToFile",                False),                    # Default
    ("CalibrationMode",             1),                        # IceTop mode
    ("SubtractBaseline",            False),                    # Done in I3TopWaveProcessor
    ("SubtractTransitTime",         True),                     # Transit time handled here
    ("CorrectPedestalDroop",        False),                    # This is not done in IceTop at all
    ("CorrectPedestalDroopDualTau", False),                    # Not done in IceTop
    ("ATWDSaturationLevel",         900),                      # Switch to next ATWD at lower saturation
    ("FADCSaturationLevel",         1022),                     # Default: There is anyway only one channel
    ("CalibrateDataWithSLC",        True)                      # Keep SLC in the data
    )

# Use topwaveprocessor to extract IceTop pulses, splits SLC and HLC
tray.AddModule("I3TopWaveProcessor", "itwp")(
    ("InputWaveforms",              "CalibratedIceTopATWD"),   # Use calibrated IT waveforms, ATWD only
    ("OutputHLCPulses",             "IceTopHLCPulses"),        # Name of HLC pulses
    ("OutputSLCPulses",             "IceTopSLCPulses"),        # Name of SLC pulses
    ("OutputWaveforms",             ""),                       # Default: Do not write waveforms to frames
    ("PreventSaturation",           False),                    # Default
    ("CorrectTransitTime",          False),                    # Default: already done in DOMcalibrator
    ("BaselineMethod",              0),                        # Default baseline subtraction method
    ("BaselineFirstNBins",          3),                        # Default, configuration of baseline method
    ("BaselineLastNbins",           8),                        # Default, configuration of baseline method
    ("BaselineFirstBin",            -45),                      # Default, configuration of baseline method
    ("BaselineNBins",               40)                        # Default, configuration of baseline method
    )

# Use topeventbuilder to convert pe to VEM and for a simple cleaning
tray.AddModule("I3TopEventBuilder", "buildevent")(
    ("InputRecoPulses",             "IceTopHLCPulses"),        # Use HLC pulses from topwaveprocessor
    ("OutputVEMPulses",             "IceTopVEMPulses"),        # Name of IT event pulses
    ("OutputBadStations",           "IceTopExcludedStations"), # Write list of excluded stations
    ("MaxHGLGTimeDifference",       40),                       # Default
    ("IntraStationTimeTolerance",   200),                      # Default
    ("InterStationTimeTolerance",   200),                      # Default
    ("InhibitLowGain",              False),                    # Also use low gain pulses
    ("Verbose",                     False),                    # Default
    )

# I3SmallShowerFilter: filter on small showers, subsequent modules are run conditionally
tray.AddModule("I3IcePickModule<I3SmallShowerFilter>", "filter")(
    ("FilterGeometry",              "IC59"),                   # Specify input geometry type, mandatory
    ("TopPulseKey",                 "IceTopVEMPulses_0"),      # Use first IT event pulses
    ("NStationResultName",          "SmallShowerNStation"),    # Write NStation as an I3Int to the frame
    ("DecisionName",                "SmallShowerFilterPassed") # Write the filter decision to the frame
    )

# Center Of Gravity as a first guess for the core position
tray.AddModule("I3TopRecoCore", "cog")(
    ("DataReadout",                 "IceTopVEMPulses_0"),      # Use first IT event pulses
    ("ShowerCore",                  "ShowerCOG"),              # Default output name
    ("Weighting_Power",             0.5),                      # Weight tanks with sqrt(charge)
    ("Verbose",                     False),                    # Default
    ("IcePickServiceKey",           "SmallShowerFilterPassed") # Conditional execution key, small showers
    )

# Plane fit for the direction reconstruction
tray.AddModule("I3TopRecoPlane", "plane")(
    ("EventHeaderName",             "I3EventHeader"),          # Default
    ("DataReadout",                 "IceTopVEMPulses_0"),      # Use first IT event pulses
    ("ShowerPlane",                 "ShowerPlane"),            # Default output name
    ("Trigger",                     3),                        # Require at least three stations (default)
    ("Verbose",                     False),                    # Default
    ("IcePickServiceKey",           "SmallShowerFilterPassed") # Conditional execution key, small showers
    )

# Lateral fit for the likelihood reconstruction. Direction reco disabled.
tray.AddModule("I3TopLateralFit", "latfit")(
    ("DataReadout",                 "IceTopVEMPulses_0"),      # Use first IT event pulses
    ("OutShower",                   "ShowerCombined"),         # Name fit output
    ("InPlane",                     "ShowerPlane"),            # Shower Plane seed
    ("InCore",                      "ShowerCOG"),              # Shower Core seed
    ("Trigger",                     3),                        # Require at least three stations
    ("ResidualCut",                 3000.0),                   # Default: effectively disable this cut
    ("AsciiFile",                   ""),                       # Default: No ascii output
    ("CoreCut",                     11.0),                     # Default: Cut stations closer than 11m to core
    ("Ldf",                         "dlp"),                    # Default lateral distribution function
    ("Curvature",                   ""),                       # Do not fit curvature
    ("CutBad",                      0),                        # Default: disable this cut
    ("CutBadThreshold",             3.0),                      # Default
    ("Verbose",                     False),                    # Default
    ("BadStations",                 "IceTopExcludedStations"), # Correctly treat stations excluded by TopEventBuilder
    ("IcePickServiceKey",           "SmallShowerFilterPassed") # Conditional execution key, small showers
    )

# I3Writer to write the output file
tray.AddModule("I3Writer", "write")(
    ("CompressionLevel",            -2),                       # Default
    ("Filename",                    options.output),           # Output file name
    ("SkipKeys",                    []),                       # Write all keys
    ("Streams",                     [icetray.I3Frame.Physics]),# Only write Physics stream
    ("IcePickServiceKey",           "SmallShowerFilterPassed") # Conditional execution key, small showers
    )

# Use the trash can to clean up the frame at the end


# Execute and clean up the tray
tray.Execute()

