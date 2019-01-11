'''
This script creates the L3 GCD file for MC.
It cuts the snow height and creates a diff with the L1/L2 GCD file (for MC) as base.
One GCD file will exist per dataset, which is not needed but easy...
'''

import argparse
import os

parser = argparse.ArgumentParser()
parser.add_argument('--MCgcd', default='', dest='gcdfile', metavar='FILE',help='Manually specify the GCD file to be used.')
parser.add_argument("--obsLevel", dest="obsLevel",type=float, default=2834., help= "ObsLevel wrong. Put used observation level.")
parser.add_argument('--output', default='',dest='output', metavar='FILE', help='Shiny new Level 3 GCD-File')
args = parser.parse_args()

if not (args.gcdfile and args.obsLevel and args.output):
    parser.print_help()
    exit(0)

from icecube import dataio, icetray, dataclasses
from I3Tray import I3Tray

tray = I3Tray()


tray.Add('I3Reader',
    Filename = args.gcdfile
    )

from icecube.icetop_Level3_scripts.modules import CutSnowHeight
tray.AddModule(CutSnowHeight,'cutHeight',
               ObservationHeight=args.obsLevel,
               )

from icecube.frame_object_diff.segments import compress
tray.AddSegment(compress,
                base_filename = args.gcdfile,
                )

tray.AddModule('I3Writer',
    Filename = args.output,
    Streams  = [icetray.I3Frame.Geometry,
                icetray.I3Frame.Calibration,
                icetray.I3Frame.DetectorStatus]
    )



tray.Execute()

