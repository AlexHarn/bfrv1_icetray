#!/usr/bin/env python

def main(inputFiles, options):
    import math
    import numpy
    import os

    from I3Tray import I3Tray
    from icecube import icetray, dataclasses, dataio, tpx

    if options.gcd:
        inputFiles = [options.gcd] + inputFiles
    outfile = options.output
    n = options.n
    outputLevel = options.log_level

    icetray.set_log_level_for_unit('tpx', icetray.I3LogLevel(outputLevel))

    # Instantiate a tray
    tray = I3Tray()
    print(inputFiles)

    tray.AddModule( 'I3Reader', 'Reader', FilenameList = inputFiles)

    tray.AddModule(lambda frame: bool(frame['QFilterMask']['IceTopSTA5_12']), 'q_filter',
                   Streams=[icetray.I3Frame.DAQ])

    data_file = os.environ['I3_BUILD'] + '/tpx/slc_calib_parameters.pcl'
    if not os.path.exists(data_file):
        raise Exception('IceTop SLC calibration constants were not found (%s). To fetch them, pass the option -DFETCH_ICETOP_SLC_CALIB=True to cmake'%data_file)

    tray.AddSegment(tpx.segments.CalibrateSLCs, 'CalibrateSLCs',
                    Config = data_file)

    tray.AddModule('I3Writer', 'Writer',
                   Filename = outfile,
                   Streams=[icetray.I3Frame.DAQ])

    

    # Execute the Tray
    if n is None:
        tray.Execute()
    else:
        tray.Execute(n)
    


if __name__ == "__main__":
    loglevel = 3
    import sys
    import os.path
    import argparse
    parser = argparse.ArgumentParser(description='Run TPX.')
    # these files are just a default where it works
    parser.add_argument('inputfiles', metavar='i3_file', type=str, nargs='?', help='Input files',
                        default=['/data/exp/IceCube/2012/filtered/level2/1130/Level2_IC86.2012_data_Run00121209_1130_GCD.i3.gz',
                                 '/data/exp/IceCube/2012/filtered/level2/1130/Level2_IC86.2012_data_Run00121209_Subrun00000208_IT.i3.bz2'])
    parser.add_argument("-o", "--output", action="store", type=str, dest="output", help="Output file name", metavar="FILE", default='icetop_slc_calib.i3')
    parser.add_argument("-n", action="store", type=int, dest="n", help="number of frames to process", metavar="N")
    parser.add_argument("--gcd", action="store", type=str, dest="gcd", help="GCD file name", metavar="FILE")
    parser.add_argument("--info", action="store_const", dest="log_level", help="Info log-level", const=2, metavar="N", default=loglevel)
    parser.add_argument("--debug", action="store_const", dest="log_level", help="Trace log-level", const=1, metavar="N", default=loglevel)
    parser.add_argument("--trace", action="store_const", dest="log_level", help="Trace log-level", const=0, metavar="N", default=loglevel)
    args = parser.parse_args()

    if not args.gcd:
        print('WARNING: you are not explicitly providing GCD file name. There better be GCD frames.')
    else:
        args.gcd = os.path.expandvars(os.path.expanduser(args.gcd))

    if args.gcd and not os.path.exists(args.gcd):
        print('GCD file %s does not exist'%args.gcd)
        parser.print_help()
        parser.exit(1)


    if not args.output:
        print('Need to provide output file name')
        parser.print_help()
        parser.exit(1)

    if len(args.inputfiles) > 0:
        main(args.inputfiles, args)
