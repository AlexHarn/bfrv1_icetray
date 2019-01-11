#!/usr/bin/env python

#########################################################
# This script takes the LE/Osc L2 files as input and will
# apply the L3 cuts with the option to filter.
#########################################################

from optparse import OptionParser
from glob import glob
from os.path import expandvars
import time

from I3Tray import *
from icecube import icetray, dataclasses, dataio
from icecube import tableio,hdfwriter

from icecube.level3_filter_lowen import LowEnergyL3TraySegment

#icetray.set_log_level(icetray.I3LogLevel.LOG_WARN)

##----------------------------------------------------------------------
## Allows the parser to take lists as inputs. Cleans out any white
## spaces that may be included in the names of the Official or
## Personal scripts to be run. Also removes the .py which is required
## to load a python module.
#
#def list_callback(option, opt, value, parser):
#    noWhiteSpace = value.replace(' ', '')
#    noPyAppend   = noWhiteSpace.replace('.py', '')
#    cleanList    = noPyAppend.split(',')
#    setattr(parser.values, option.dest, cleanList)
#
#def string_callback(option, opt, value, parser):
#    lower_case   = value.lower()
#    no_hyphens   = lower_case.replace('-', '')
#    no_underscore= no_hyphens.replace('_', '')
#    setattr(parser.values, option.dest, no_underscore)

#----------------------------------------------------------------------
#----------------------------------------------------------------------
# The inputfile can be added singly using the -i option
# or many input files can be processed by adding them
# as arguments.

#usage = "%prog [options] <inputfiles>"
#parser = OptionParser(usage=usage)

def make_parser():
    """Make the argument parser"""
    from optparse import OptionParser
    parser = OptionParser()

    # i/o options
    parser.add_option("-i", "--inputfile", action="store",
	    type="string", default="", dest="infile",
	    help="Input i3 file(s)  (use comma separated list for multiple files)")
    
    parser.add_option("-g", "--gcdfile", action="store",
	    type="string", default="", dest="gcdfile",
	    help="GCD file for input i3 file")
    
    parser.add_option("-o", "--outputfile", action="store",
	    type="string", default="", dest="outfile",
	    help="Main i3 output file")
    
    parser.add_option("--hd5output", action="store",
	    type="string", default="", dest="hd5output",
	    help="hdf5 Output file")

    parser.add_option("--year", action="store",
                      type="string", default="12", dest="year",
                      help = "Processing years last two digits (i.e. 2012=12)")
#    
#    parser.add_option("-b", "--branches", type = "int", action = "store",
#		      default = 1, dest="branches",
#		      help = "Process which filter branches (1=Both,2=StdDCFilter,3=ExpFidFilter)")
    
    parser.add_option("-f","--filter", action="store_true",
		      default=True, dest='filter', 
		      help="If true, only events passing specified branch will be written.")
    
    parser.add_option("-n", "--num", action="store",
	    type="int", default=-1, dest="num",
	    help="Number of frames to process")
    
    return parser


#(options, args) = parser.parse_args()

#def main(params):
def main(options, stats={}):
    icetray.logging.set_level("WARN")

    tray = I3Tray()
    
    files = [options['gcdfile']]

    if isinstance(options['infile'],str):
        files.append(options['infile'])
    else:
        for f in options['infile']:files.append(f)


    tray.AddModule("I3Reader", "reader", filenamelist=files)

    # ##################################################################
    # Changing year to match files but still make sense to normal people
    # ##################################################################

    if int(options['year']) == 12:
        years=str(12)
    elif int(options['year']) < 12:
        raise Exception['This cannot be used for years before 2012'] 
    else:
        years=str(13)

    # ##############################
    # 2012 DeepCore L3 tray segment
    # ##############################
    
    tray.AddSegment(LowEnergyL3TraySegment.DCL3MasterSegment,"DCL3MasterSegment", Filter=options['filter'],year=years)
    
    # #########################################
    # List of objects to write to table
    # #########################################
    
    FrameObjectsToKeep = [ "CascadeLast_DC",
			   "CorsikaWeightMap",
			   "DecayParticle",
			   "iLinefit_LE_L3",                      
			   "I3EventHeader",
			   "I3MCTree",
			   'VertexGuessInsideExpDC',
			   "I3MCWeightDict",
			   "I3TriggerHierarchy",
			   "IC2012_LE_L3_Vars",
			   "IC2012_LE_L3",
                           "IC2012_LE_L3_No_RTVeto",
			   "IC2012_ExpLE_L3_Vars",
			   "IC2012_ExpLE_L3",
			   "IniceNu",
			   "InteractionVertex",
			   "InteractionParticle",
			   "FilterMask",
			   "LineFit_DC",
			   "NoiseEngine_bool",
			   "PrimaryNu",
			   "ToI_DC",
			   ]
    
    tray.AddModule( 'I3Writer', 'EventWriter',
		    Filename          = options['outfile'],
		    Streams           = [icetray.I3Frame.Physics, icetray.I3Frame.DAQ],
		    DropOrphanStreams = [icetray.I3Frame.DAQ]
		    )
    
    if len(options['hd5output']):
	    #taboutfile = options.outputfile.replace('.i3','.hdf5')
	    tabler = hdfwriter.I3HDFTableService(options['hd5output'],1)
	    tray.AddModule(tableio.I3TableWriter,'writer1',tableservice = tabler, SubEventStreams = ['InIceSplit',],keys=FrameObjectsToKeep)
    
    
    
#    if options.numevents < 0:
#	tray.Execute()
#    else:
#	tray.Execute(options.numevents)
	
    # make it go
    if options['num'] >= 0:
        tray.Execute(options['num'])
    else:
        tray.Execute()
	
    
    
    
    #del tray


### iceprod stuff ###
try:
    from iceprod.modules import ipmodule
except ImportError, e:
    print 'Module iceprod.modules not found. Will not define IceProd Class'
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

    if opts['infile']=='' or opts['gcdfile']=='' or opts['outfile']=='':
        print "you must specify the input, gcd and output file arguments"
        exit(1)

    print opts
    main(opts)
