#!/usr/bin/env python

from I3Tray import *
from icecube import dataio, dataclasses, icetray 
from icecube.finallevel_filter_diffusenumu import level4, level5
from icecube.finallevel_filter_diffusenumu.write_hdf import write_hdf
from ConfigParser import ConfigParser

def make_parser_for_iceprod():
    """
    IceProd 1 cannot handle argparse. Therefore, we need to provide an OptionParser
    for it. Since the argparse is way nicer, we leave the argparse for the command line.
    """

    from optparse import OptionParser
    parser = OptionParser()

    parser.add_option("-i",
                        "--input",
                        dest="infile",
                        help="Input i3 file(s)")

    parser.add_option("-g",
                        "--gcd",
                        dest="gcdfile",
                        help="GCD file")

    parser.add_option("-o",
                        "--output",
                        dest="outfile",
                        help="Main i3 output file")

    parser.add_option("--hdf-out",
                        dest="hd5output",
                        help="HDF output File")

    parser.add_option("--isNuMu",
                        action="store_true",
                        dest="is_numu",
                        help="Remove bug events in nugen simulation")

    return parser

def main(options = None, stats = {}):
    icetray.logging.set_level("WARN")

    args = None
    if options is None:
        # No iceprod
        from argparse import ArgumentParser

        parser=ArgumentParser()
    
        parser.add_argument("-i",
                            "--input",
                            dest="infile",
                            required=True,
                            nargs="+",
                            help="Input i3 file(s)")
    
        parser.add_argument("-g",
                            "--gcd",
                            dest="gcdfile",
                            required=True,
                            help="GCD file")
    
        parser.add_argument("-o",
                            "--output",
                            dest="outfile",
                            required=True,
                            help="Main i3 output file")
    
        parser.add_argument("--hdf-out",
                            dest="hd5output",
                            required=True,
                            help="HDF output File")
    
        parser.add_argument("--isNuMu",
                            action="store_true",
                            dest="is_numu",
                            help="Remove bug events in nugen simulation")
    
        args = parser.parse_args()
    else:
        # Iceprod is running this script. Need to translate the dict
        from collections import namedtuple
        ArgumentParserDummy = namedtuple('ArgumentParserDummy', options.keys())
        args = ArgumentParserDummy(**options)

    parser = ConfigParser()
    parser.readfp(open(os.path.join(os.path.expandvars('$I3_BUILD'),
                                    'lib/icecube/finallevel_filter_diffusenumu',
                                    'paths.cfg')))
    paths = dict(parser.items("main"))

    # Check if somebody messed with the tables
    ret = os.system("md5sum -c {}".format(
        os.path.join(os.path.expandvars('$I3_BUILD'),
                     'lib/icecube/finallevel_filter_diffusenumu',
                     'checksums')))
    if ret != 0:
        raise RuntimeError("Tables are corrupt")
    tray = I3Tray()

    tray.Add(level4.IC12L4,
             gcdfile=args.gcdfile,
             infiles=args.infile,
             table_paths=paths,
             is_numu=args.is_numu)

    tray.Add(level5.segments.Scorer, "doLevel5",
        CutFunc=level5.segments.CutFunc,
        CascCut=0.5)

    tray.Add(level5.segments.millipede_segment, "MillipedeLosses", table_paths=paths)
    tray.Add(level5.segments.paraboloid_segment, "Paraboloid", table_paths=paths)
    tray.Add(write_hdf, hdf_out=args.hd5output)


    # write output
    tray.Add("I3Writer",
        Filename = args.outfile,
        DropOrphanStreams=[icetray.I3Frame.DAQ],
        Streams=[icetray.I3Frame.DAQ, icetray.I3Frame.Physics])

    tray.Execute()

    usagemap = tray.Usage()

    for mod in usagemap:
        print(mod)

### iceprod stuff ###
try:
    from iceprod.modules import ipmodule
except ImportError:
    print('Module iceprod.modules not found. Will not define IceProd Class')
else:
    Level2Filter = ipmodule.FromOptionParser(make_parser_for_iceprod(),main)
### end iceprod stuff ###

if (__name__)=="__main__":
    main()
