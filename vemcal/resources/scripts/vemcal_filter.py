#!/usr/bin/env python

from I3Tray import *

from os.path import expandvars, dirname, basename   # needed for path- and filename manipulations
from string  import *                               # needed for string manipulations i.e replace

import glob


load("libdataclasses")
load("libpfclasses")
load("libdataio")
load("libicepick")

tray = I3Tray()

in_dir = "/mnt/fpmaster/pnflocal2/tilo"

# Get data files
file_pattern = '%s/PFFilt_PhysicsTrig_PhysicsFiltering_Run[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9]_Subrun00000000_[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9].i3' % (in_dir)

file_list = glob.glob(file_pattern)
file_list.sort()

vemcal_outdir = './'
vemcal_file   = '%s/IceTop_VEMCal_Run%8.8d.i3.gz' % (vemcal_outdir, run_number)


tray.AddService("I3Reader","reader")(
    ("FileNameList",    file_list),
    ("OmitGeometry",    True),
    ("OmitCalibration", True),
    ("OmitStatus",      True),
    ("SkipMissingDrivingTime", True),
    )

tray.AddModule("I3IcePickModule<I3FrameObjectFilter>", "vemcal_filter")(
    ("FrameObjectKey","I3VEMCalData"),
    ("DiscardEvents", True),
    )

tray.AddModule("Keep","keep_objects")(
    ("Keys",["DrivingTime",
             "I3VEMCalData"]),
    )

tray.AddModule("I3Writer","write_file")(
    ("FileName", vemcal_file),
    ("Streams", ["Physics"]),
    )

tray.AddModule("Dump","dump")



tray.Execute()


