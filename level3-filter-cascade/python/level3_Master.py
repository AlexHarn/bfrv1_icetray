#!/usr/bin/env python

from I3Tray import *
import sys, os, glob

#
## load everything

'''
def try_icecube_import(name):
    full_name = 'icecube.' + name
    try:
        __import__(full_name)
        globals()[name] = sys.modules[full_name]
    except ImportError:
        pass
lib_dir = os.path.join(os.getenv('I3_BUILD'),'lib','icecube')
libs = map(os.path.basename,glob.glob(lib_dir + os.path.sep + '*.so'))
for lib in libs:
    try_icecube_import(os.path.splitext(lib)[0])
'''

from icecube.level3_filter_cascade.CascadeL3TraySegment import CascadeL3



def make_parser():
    """Make the argument parser"""
    from optparse import OptionParser
    parser = OptionParser()
    
    parser.add_option("-i", "--input", action="store",
        type="string", default="", dest="infile",
        help="Input i3 file(s)  (use comma separated list for multiple files)")
    
    parser.add_option("-g", "--gcd", action="store",
        type="string", default="", dest="gcdfile",
        help="GCD file for input i3 file")
    
    parser.add_option("-o", "--output", action="store",
        type="string", default="", dest="outfile",
        help="Main i3 output file")
    
    
    parser.add_option ("-m", "--MC", default=False, action="store_true",
                       dest="isMC", 
                       help="The data is MC (different behaivior for TWR)")
    parser.add_option ("-t", "--MT", default="corsika",
                       dest="MCType", 
                       help="MCType (to book correctly the MCTruth)")
    
    
    parser.add_option("--amplitudetablepath", action="store",
        type="string", default="/cvmfs/icecube.opensciencegrid.org/data/photon-tables/splines/ems_mie_z20_a10.abs.fits",
        dest="AmplitudeTable", help="Path to AmplitudeTable")
    

    parser.add_option("--timingtablepath", action="store",
        type="string", default="/cvmfs/icecube.opensciencegrid.org/data/photon-tables/splines/ems_mie_z20_a10.prob.fits",
        dest="TimingTable", help="Path to TimingTable")
    

    parser.add_option("-n", "--num", action="store",
        type="int", default=-1, dest="num",
        help="Number of frames to process")

    parser.add_option("-y", "--year", action="store",
        type="string", default="2015", dest="year",
        help="The year of the data/mc being processed")
    
    
    return parser    


def main(options, stats={}):
    icetray.logging.set_level("WARN")

    tray = I3Tray()
    
    tray.AddSegment(CascadeL3,
                    gcdfile=options['gcdfile'],
                    infiles=options['infile'],
                    output_i3=options['outfile'],
                    AmplitudeTable=options['AmplitudeTable'],
                    TimingTable=options['TimingTable'],
                    MCbool=options['isMC'],
                    MCtype=options['MCType'],
		    Year=options['year'])

    # make it go
    if options['num'] >= 0:
        tray.Execute(options['num'])
    else:
        tray.Execute()
    

    # print timers
    usagemap = tray.Usage()

    for mod in usagemap:
        print (mod)


### iceprod stuff ###
try:
    from iceprod.modules import ipmodule
except ImportError as e:
    print ('Module iceprod.modules not found. Will not define IceProd Class')
else:
    Level2Filter = ipmodule.FromOptionParser(make_parser(),main)
### end iceprod stuff ###


# the business!
if (__name__=="__main__"):

    from optparse import OptionParser

    parser = make_parser()

    
    (options,args) = parser.parse_args()
    opts = {}
    # convert to dictionary
    for name in parser.defaults:
        value = getattr(options,name)
        if name == 'infile' and ',' in value:
            value = value.split(',') # split into multiple inputs
        opts[name] = value


    main(opts)
