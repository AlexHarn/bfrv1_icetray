#!/usr/bin/env python

from I3Tray import *
import sys, os, glob
import subprocess
from icecube import icetray, dataio, dataclasses, hdfwriter, phys_services, lilliput, gulliver, gulliver_modules, linefit, rootwriter

from icecube.level3_filter_muon.MuonL3TraySegment import MuonL3


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
    
    parser.add_option("--hd5output", action="store",
        type="string", default="", dest="hd5output",
        help="hd5 Output i3 file")
    
    parser.add_option("--rootoutput", action="store",
        type="string", default="", dest="rootoutput",
        help="root Output i3 file")
    
    parser.add_option("-n", "--num", action="store",
        type="int", default=-1, dest="num",
        help="Number of frames to process")

    parser.add_option("--photonicsdir", action="store",
        type="string", default="/cvmfs/icecube.opensciencegrid.org/data/photon-tables/SPICEMie/",
        dest="photonicsdir", help="Directory with photonics tables")
    
    parser.add_option("--photonicsdriverdir", action="store",
        type="string", default="/cvmfs/icecube.opensciencegrid.org/data/photon-tables/SPICEMie/driverfiles",
        dest="photonicsdriverdir", help="Directory with photonics tables driver files")
    
    parser.add_option("--photonicsdriverfile", action="store",
        type="string", default="mu_photorec.list",
        dest="photonicsdriverfile", help="photonics driver file")
    
    parser.add_option("--infmuonampsplinepath", action="store",
        type="string", default="/cvmfs/icecube.opensciencegrid.org/data/photon-tables/splines/InfBareMu_mie_abs_z20a10_V2.fits",
        dest="infmuonampsplinepath", help="InfMuonAmpSplinePath")
    
    parser.add_option("--infmuonprobsplinepath", action="store",
        type="string", default="/cvmfs/icecube.opensciencegrid.org/data/photon-tables/splines/InfBareMu_mie_prob_z20a10_V2.fits",
        dest="infmuonprobsplinepath", help="InfMuonProbSplinePath")
    
    parser.add_option("--cascadeampsplinepath", action="store",
        type="string", default="/cvmfs/icecube.opensciencegrid.org/data/photon-tables/splines/ems_mie_z20_a10.abs.fits",
        dest="cascadeampsplinepath", help="CascadeAmpSplinePath")

    parser.add_option("--cascadeprobsplinepath", action="store",
        type="string", default="/cvmfs/icecube.opensciencegrid.org/data/photon-tables/splines/ems_mie_z20_a10.prob.fits",
        dest="cascadeprobsplinepath", help="CascadeProbSplinePath")
    
    parser.add_option("-r", "--restoretwformc", action="store_true",default=False,
        dest="restoretwformc", help="Restore TimeWindow for MC?")

    parser.add_option("--gsiftp", action="store", type="string", default="",
        dest="gsiftp", help="url for gsiftp for file transfer")

    
    return parser    

def main(options, stats={}):
    icetray.logging.set_level("WARN")

    tray = I3Tray()

    tray.AddSegment(MuonL3,
                    gcdfile=options['gcdfile'],
                    infiles=options['infile'],
                    output_i3=options['outfile'],
                    output_hd5=options['hd5output'],
                    output_root=options['rootoutput'],
                    photonicsdir=options['photonicsdir'],
                    photonicsdriverdir=options['photonicsdriverdir'],
                    photonicsdriverfile=options['photonicsdriverfile'],
                    infmuonampsplinepath=options['infmuonampsplinepath'],
                    infmuonprobsplinepath=options['infmuonprobsplinepath'],
                    cascadeampsplinepath=options['cascadeampsplinepath'],
                    cascadeprobsplinepath=options['cascadeprobsplinepath'],
                    restore_timewindow_forMC = options['restoretwformc']
                    )

    

    # make it go
    if options['num'] >= 0:
        tray.Execute(options['num'])
    else:
        tray.Execute()
    

    # print(timers)

    usagemap = tray.Usage()

    for mod in usagemap:
        print(mod)

    
### iceprod stuff ###
iceprod_available = False
try:
    from simprod.modules import ipmodule
except ImportError:
    try:
        from iceprod.modules import ipmodule
    except ImportError:
        print('Module iceprod.modules not found. Will not define IceProd Class')
    else:
        iceprod_available = True
else:
    iceprod_available = True

if iceprod_available:
    Level2Filter = ipmodule.FromOptionParser(make_parser(),main)
### end iceprod stuff ###

# the business!
if (__name__=="__main__"):

    from optparse import OptionParser

    parser = make_parser()
    
    (options,args) = parser.parse_args()

    #------------------------------
    # check paths 1
    #------------------------------
    if options.gcdfile == "" :
        print("gcdfile is empty. exit now.")
        sys.exit(0)
    if options.infile == "" :
        print("infile is empty. exit now.")
        sys.exit(0)

    #-------------------------------
    # convert options to dictionary
    #-------------------------------
    opts = {}
    for name in parser.defaults:
        value = getattr(options,name)

        if name == 'infile' :
            values = []
            value = value.replace(" ","")
            if ',' in value :
                values = value.split(',') # split into multiple inputs
            else :
                values.append(value)
            opts[name] = values
        else :
            opts[name] = value

    #------------------------------
    # check table paths
    #------------------------------
    # list of table paths
    table_paths = ['photonicsdir', 
                  'photonicsdriverdir',
                  'infmuonampsplinepath', 
                  'infmuonprobsplinepath',
                  'cascadeampsplinepath', 
                  'cascadeprobsplinepath']

    for key in table_paths:
        path = opts[key]
        if not os.path.exists(path) :
            print("option %s=%s is not accessible.return now." % (key, path))
            sys.exit(0)

    path = opts['photonicsdriverdir']+"/"+opts['photonicsdriverfile']
    if not os.path.exists(path) :
            print("option photonicsdriverfile=%s is not accessible.return now." % (path))
            sys.exit(0)
 
    #------------------------------
    # append gsiftp path if needed 
    #------------------------------
    gsiftp = options.gsiftp
    need_to_modify = []
    if gsiftp != "" :
        need_to_modify = ['gcdfile','infile','outfile',
                          'hd5output','rootoutput'] 

    for key in need_to_modify :
        value = opts[key]
        if value == "" :
            # if value is empty don't add gsiftp path
            continue
        elif key == 'infile' :
            mod_infiles = []
            for infilename in value :
                infilepath = gsiftp + infilename
                mod_infiles.append(infilepath)
            opts[key] = mod_infiles
        else :
            newpath = gsiftp + value
            opts[key] = newpath

    # write options (for debug)
    print("===== option list start =====")
    for key, value in opts.items() :
        print "%s is set to : " %(key), value
    print("===== option list end =====")

    #------------------------------
    # Done. call main function.
    #------------------------------
    main(opts)
