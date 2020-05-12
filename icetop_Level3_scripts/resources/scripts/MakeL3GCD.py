'''
This script creates a the Level3 GCD diff to be used in data. 
It needs to be ran in Madison since it needs the GCD and VEMCal tabulated values. 
It takes the Level2 GCD, then modifies the snow and the vemcal values.
Then it creates a diff with the MC GCD as a base! Such that you only need to copy the diff and the MC GCD to your local computing system.
'''

import argparse
import os, glob

parser = argparse.ArgumentParser()
parser.add_argument('--runnumber', type=int, metavar='NUM', help='Number of the run to be processed')
parser.add_argument('--day', type=int, metavar='NUM', help='Day for snow height interpolation')
parser.add_argument('--month', type=int, metavar='NUM', help='Month for snow height interpolation')
parser.add_argument('--year', type=int, metavar='NUM', help='Year for snow height interpolation')
parser.add_argument('--output', type=str, metavar='FILE', help='Shiny new Level 3 GCD-File')
parser.add_argument('--MCgcd',type=str,metavar='FILE',help="GCD used in MC to use as a base.")
parser.add_argument('--L2-gcdfile', dest='L2_gcdfile', help='Manually specify the L2 GCD file to be used. When you run in Madison with the standard L3 GCD diff, this is not needed.')
parser.add_argument('--VEMCal-dir', dest='VEMCal_dir', help='Manually specify the directory holding VEMCal (xml) files.  When you run in Madison, this is not needed.')
parser.add_argument('--pass2a', action="store_true", help='Look for the L2GCD in the "pass2a" location, as opposed to regular level2?')

args = parser.parse_args()

if not (args.runnumber and args.day and args.month and args.year and args.output and args.MCgcd):
    parser.print_help()
    exit(0)

from icecube import dataio, icetray, icetop_Level3_scripts
from I3Tray import I3Tray

icetray.I3Logger.global_logger.set_level(icetray.I3LogLevel.LOG_INFO)

tray = I3Tray()

if not args.L2_gcdfile:
    gcd = icetop_Level3_scripts.functions.find_L2_GCD_from_date(args.runnumber, args.day, args.month, args.year, args.pass2a)
    icetray.logging.log_info('Using found L2 GCD: {0}'.format(gcd))
else:
    icetray.logging.log_info('Using user-defined L2 GCD: %s'%args.L2_gcdfile)
    gcd=args.L2_gcdfile

icetray.logging.log_info('Base will be {0}'.format(args.MCgcd))

tray.Add('I3Reader', 'reader',
    Filename = gcd
    )

from icecube.icetop_Level3_scripts.modules import ChangeSnowHeight_interpolated
tray.Add(ChangeSnowHeight_interpolated, 'updateSnow',
    Filename = os.path.expandvars('${I3_BUILD}/icetop_Level3_scripts/resources/data/IT81-MasterwithSnowMeasurements-Mar2019.csv'),
    Day      = args.day,
    Month    = args.month,
    Year     = args.year
    )

from icecube.icetop_Level3_scripts.modules import UpdateVEMCal
tray.Add(UpdateVEMCal, 'updateVEMCal',
    Runnumber = args.runnumber,
    Day       = args.day,
    Month     = args.month,
    Year      = args.year,
    XMLDir    = args.VEMCal_dir
    )

from icecube.frame_object_diff.segments import compress
tray.Add(compress, 'compress',
    base_filename = args.MCgcd
    )

tray.Add('I3Writer', 'writer',
    Filename = args.output,
    Streams  = [icetray.I3Frame.Geometry,
                icetray.I3Frame.Calibration,
                icetray.I3Frame.DetectorStatus]
    )

tray.Execute()

