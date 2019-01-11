#!/usr/bin/env python
#----------------------------------------------------------------------
# The following code is an end-to-end test of the DeepCore_Filter
# modules through the tray segment. The I3_TESTDATA is used as standard
# testing data.
#
# $Id$
# $Date$ $Revision$ $Author$
#----------------------------------------------------------------------
import sys
from os.path import expandvars
from optparse import OptionParser

from icecube import icetray, dataclasses, dataio, DomTools, DeepCore_Filter
from I3Tray import *

tray = I3Tray()

parser = OptionParser()

parser.add_option( "-g", "--geo", action="store", type="string", dest="geoFile",
                   metavar="<geo file>", default=expandvars("$I3_TESTDATA/sim/GeoCalibDetectorStatus_2013.56429_V1.i3.gz"),
                   help="Name of GCD file")

parser.add_option( "-d", "--data_file",
                   dest = "data_file",
                   default = expandvars("$I3_TESTDATA/sim/Level2_IC86.2011_corsika.010281.001664.00.i3.bz2"),
                   help = "Test .i3.gz file")

(options, args) = parser.parse_args()

tray.AddModule("I3Reader", "reader") (
    ("filenamelist", [options.geoFile, options.data_file])
    )

tray.AddModule("I3LCCleaning", "get_hlc_launches",
               InIceInput="InIceRawData",
               InIceOutput="HLCInIceRawData")

from icecube.DeepCore_Filter import DOMS
dlist = DOMS.DOMS("IC86")

tray.AddModule("I3OMSelection<I3DOMLaunchSeries>",'selectICVetoDOMS',
               OmittedKeys = dlist.DeepCoreVetoDOMs,
               OutputOMSelection = 'GoodSelection_ICVeto',
               InputResponse = "HLCInIceRawData",
               OutputResponse = 'HLCInIceRawData_ICVeto',
               SelectInverse = True,
               )

tray.AddModule("I3OMSelection<I3DOMLaunchSeries>",'selectDCFidDOMs',
               OmittedKeys= dlist.DeepCoreFiducialDOMs,
               SelectInverse = True,
               InputResponse = "HLCInIceRawData",
               OutputResponse = 'HLCInIceRawData_DCFid',
               OutputOMSelection = 'GoodSelection_DCFid',
               )

tray.AddModule("I3DeepCoreVeto<I3DOMLaunch>",'deepcore_filter',
               ChargeWeightCoG = False,
               DecisionName = "DCVeto",
               FirstHitOnly = False,
               InputFiducialHitSeries = 'HLCInIceRawData_DCFid',
               InputVetoHitSeries = 'HLCInIceRawData_ICVeto',
               )

tray.AddModule("I3DeepCoreTimeVeto<I3DOMLaunch>",'deepcore_timeveto',
               ChargeWeightCoG = True,
               DecisionName = "DCTimeVeto",
               FirstHitOnly = True,
               InputFiducialHitSeries = 'HLCInIceRawData_DCFid',
               InputVetoHitSeries = 'HLCInIceRawData_ICVeto',
               )

npassed_veto = 0
npassed_timeveto = 0
def counter(frame):
    global npassed_veto, npassed_timeveto
    if not frame.Has("DCVeto"): return 
    npassed_veto     += frame["DCVeto"].value
    npassed_timeveto += frame["DCTimeVeto"].value

tray.AddModule(counter, "count_frames")



tray.Execute(1000)




# This file should have 8 DCVeto and 4 DCTimeVeto events passing of the first 1000
if not npassed_veto == 8 or not npassed_timeveto == 4:
    sys.exit(1)
else:
    sys.exit(0)
