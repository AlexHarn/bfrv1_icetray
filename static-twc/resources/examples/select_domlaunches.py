#! /usr/bin/env python
"""
Selects DOMLaunches based on a time window around the trigger time.

:Input: The input file should contain these keys
InIceRawData
OfflinePulses
I3TriggerHierarchy (with TriggerID 1010)

:Output:
No output it produced (no I3Writer), but this key is generated
  STWCInIceRawData
"""

import os
import sys
from I3Tray import I3Tray
from icecube import icetray, dataio, static_twc

infile = sys.argv[1]

# icetray
tray = I3Tray()

# Reader
tray.AddModule("I3Reader","reader")(
    ("filenamelist",[infile]),
)

# DOMLaunch Version
tray.AddModule("I3StaticTWC<I3DOMLaunchSeries>","domlaunch_twc")(
    ("InputResponse","InIceRawData"),
    ("OutputResponse","STWCInIceRawData"),
    ("TriggerName","I3TriggerHierarchy"),
    ("TriggerConfigIDs", [1007]),
    ("WindowMinus",  3500.0),
    ("WindowPlus",  4000.0),
)


tray.Execute()

