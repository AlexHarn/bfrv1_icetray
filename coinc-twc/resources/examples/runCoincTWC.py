#!/usr/bin/env python
from I3Tray import *

from icecube import dataio, coinc_twc

import math
import os.path
from optparse import OptionParser

parser = OptionParser(usage='%s [options] -o <filename>.i3[.bz2|.gz] {i3 file list}'%os.path.basename(sys.argv[0]))
parser.add_option("-o", "--output", action="store", type="string", dest="output", help="Output file name", metavar="BASENAME")
parser.add_option("-n", action="store", type="int", dest="n", help="number of frames to process", metavar="N")
parser.add_option('-g', '--gcdfile', dest='gcdfile', help='Manually specify the GCD file to be used')

(options, inputFiles) = parser.parse_args()

ok=True
if not (options.gcdfile and os.path.exists(options.gcdfile)):
    print " - GCD file %s not found!"%options.gcdfile    
    ok=False

if not options.output:
    print " - Output file not specified!"
    ok=False

if not ok:
    print ''
    parser.print_help()
    exit(0)

tray = I3Tray()

#Read in an i3-file
gcdFile=[options.gcdfile]

print inputFiles

tray.AddModule("I3Reader","readme")(
    ("FileNameList",gcdFile+inputFiles)
    )

# Settings used in Tom Feusels' IT73-IC79 analysis, more or less
tray.AddModule("I3CoincTWC<I3RecoPulseSeries>","coincTWC",
               CheckSingleSMTs = True,          # default                                                                                                                                               
               cleanWindowMaxLength = 6500.,    # default                                                                                                                                               
               cleanWindowMinus = 300.,         # default                                                                                                                                               
               cleanWindowPlus = 400.,          # default                                                                                                                                               
               IceTopVEMPulsesName = 'CleanedHLCTankPulses',  # just be safe to select IT SMT, if random coinc, will NOT pass the whole chain of cuts!                                                  
               InputResponse = 'OfflinePulses', # Pulses present in L2. Renamed to InIcePulses in IC86.2012 and in L3. 
               KeepLookingForCausalTrigger = False,   # default                                                                                                                                         
               KeepMultipleCausalTriggers = True,   # default                                                                                                                                           
               OutputResponse ='CoincPulses',
               UseSMT3 = False,
               UseSMT8 = True,
               MaxTimeDiff = 1500.,         # should be sharper? NO, just need to find A IT trigger, good reco + InIce size will clean out remaining random coinc!                                      
               WindowMax = 7000.,
               WindowMin = 4780.,          # tuned on MC, but double coinc II SMT might make this earlier (in data)                                                                                     
               Strategy = 'method2',       # 99.6% efficient!! vs 98% of method1                                                                                                                        
               Stream = icetray.I3Frame.Physics
               )


#Writer
outfile=options.output
if outfile.replace('.bz2', '').replace('.gz','')[-3:] == '.i3':
    tray.AddModule("I3Writer", "i3-writer",
                   Filename = outfile,
                   DropOrphanStreams = [ icetray.I3Frame.DAQ ],
                   streams = [icetray.I3Frame.DAQ, icetray.I3Frame.Physics],
                   )
else:
    raise Exception('I do not know how to handle files with extension %s'%outfile.replace('.bz2', '').replace('.gz','')[-3:])



if options.n is None:
    tray.Execute()
else:
    tray.Execute(options.n)

    
