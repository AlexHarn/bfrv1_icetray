'''
Example booking script for level3 IT files.
Not all keys are listed here, you should add your own selection...
'''
import glob, re
from argparse import ArgumentParser
from icecube import icetray, dataio, dataclasses, phys_services
from icecube.tableio import I3TableWriter
from icecube.rootwriter import I3ROOTTableService
from icecube.icetop_Level3_scripts import icetop_globals
from icecube.frame_object_diff.segments import uncompress
from I3Tray import *


def get_detector_from_filename(input_file):
    m = re.search("Level[^_]+_(IC[^_]+)_", input_file)
    if not m:
        raise ValueError("cannot parse %s for detector config" % input_file)
    print m
    return m.group(1)


def get_run_from_filename(input_file):
    result = None
    m = re.search("Run([0-9]+)", input_file)
    if not m:
        raise ValueError("cannot parse %s for Run number" % input_file)
    return int(m.group(1))

def get_dataset_from_filename(input_file):
    result = None
    m = re.search("_([0-9]+)_", input_file)
    if not m:
        raise ValueError("cannot parse %s for dataset" % input_file)
    return int(m.group(1))

parser = ArgumentParser(usage='%s [options] -o <filename>.i3[.bz2|.gz] {i3 file list}'%os.path.basename(sys.argv[0]))
parser.add_argument("-o", "--output", action="store", type=str, dest="output", help="Output file name", metavar="BASENAME")
parser.add_argument("--do-inice", action="store_true",dest="do_inice",help= "Also do in-ice reco?")
parser.add_argument("-n", action="store", type=int, dest="n", help="number of frames to process", metavar="N")
parser.add_argument("-m","--isMC", action="store_true",dest="isMC", help= "Is this data or MC?")
parser.add_argument('--L3-gcdfile', dest='L3_gcdfile', type=str, help='Manually specify the L3 (diff) GCD file to be used. When you run in Madison, this is not needed.')
parser.add_argument('--L2-gcdfile', dest='L2_gcdfile', type=str, help='Manually specify the L2 GCD file to be used. When you run in Madison with the standard L3 GCD diff, this is not needed.')
parser.add_argument("--dataset",dest="dataset", type=int,help= "Dataset number for MC. Needed when using default GCD, to look for it..")
parser.add_argument("--run",dest="run", type=int,help= "Runnumber, needed for data. Needed when using default GCD, to look for it..")
parser.add_argument("-d", "--det", dest="detector", help="Detector configuration name, eg: IC79, IC86.2011, IC86.2012. Auto-detected if filename has standard formatting.")
     
parser.add_argument('inputFiles',help="Input file(s)",nargs="*")

(args) = parser.parse_args()

ok=True
if not len(args.inputFiles)>0:
    icetray.logging.log_error("No input files found!")
    ok=False
else:
    # try to supply some args by parsing the input filename                                                                                                                                                 
    if args.detector is None:
        args.detector = get_detector_from_filename(args.inputFiles[0])
        icetray.logging.log_info("Auto-detected detector %s" % args.detector)
    if not args.isMC:  # Filename is only really needed for data...                                                                                                                                        
        if args.run is None:
            args.run = get_run_from_filename(args.inputFiles[0])
            icetray.logging.log_info("Auto-detected run %i" % args.run)
    else:
        if args.dataset is None:
            args.dataset=get_dataset_from_filename(args.inputFiles[0])
            icetray.logging.log_info("Auto-detected dataset %i" %args.dataset)

# Check whether output is okay.
if not args.output:
    icetray.logging.log_error("Output file not specified!")
    ok=False
else:
    if args.output[-5:]==".root":
        table_service = I3ROOTTableService(args.output)
    elif args.output[-3:]==".h5":
        icetray.logging.log_error("I do not know how to handle h5 files yet.")
        ok=False
    else:
        icetray.logging.log_error("Wrong extension for booking.")
        ok=False

# Check L3 GCD file. If not supplied, find it in the default directory.
if not args.L3_gcdfile:
    icetray.logging.log_info("Using the default L3 GCD.")
    if args.isMC:
        if not args.dataset:
            icetray.logging.log_error("When using the default L3 GCD for MC, you need to specify the dataset!")
            ok=False
        else:
            if not os.path.exists("/data/ana/CosmicRay/IceTop_level3/sim/%s/GCD/Level3_%i_GCD.i3.gz"%(args.detector, args.dataset)):
                icetray.logging.log_error("The default L3 GCD file is not found.")
                ok=False
    else:
        if not args.run:
            icetray.logging.log_error("When using the default L3 GCD for data, you need to specify the run number such that we can look for the correct GCD!")
            ok=False
        else:
            if not len(glob.glob("/data/ana/CosmicRay/IceTop_level3/exp/%s/GCD/Level3_%s_data_Run00%i_????_GCD.i3.gz"%(args.detector, args.detector, args.run)))==1:
                icetray.logging.log_error("Default L3 GCD file not found.")
                ok=False
else:
    icetray.logging.log_info("Using a user specified L3 GCD file.")
    if not os.path.exists(args.L3_gcdfile):
        icetray.logging.log_error(" L3 GCD file not found")
        ok=False

if not args.L2_gcdfile:
    icetray.logging.log_info("Using the default L2 GCD.")
else:
    icetray.logging.log_info("Using a user specified L2 GCD file. This should be the same as the one where the diff is create against!")
    if not os.path.exists(args.L2_gcdfile):
        icetray.logging.log_error(" L2 GCD file not found.")
        ok=False

if not ok:
    parser.print_help()
    exit(0)
 
if not args.L3_gcdfile:
        if args.isMC:
            gcdfile=["/data/ana/CosmicRay/IceTop_level3/sim/%s/GCD/Level3_%i_GCD.i3.gz"%(args.detector, args.dataset)]
        else:
            gcdfile=glob.glob("/data/ana/CosmicRay/IceTop_level3/exp/%s/GCD/Level3_%s_data_Run00%i_????_GCD.i3.gz"%(args.detector, args.detector, args.run))
else:
        gcdfile = [args.L3_gcdfile]

tray=I3Tray()
tray.AddModule("I3Reader","reader", FilenameList=gcdfile+args.inputFiles)

# If the L2 gcd file is not specified, use the base_filename which is used for compressing. Check First whether it exists.                                                                               
# If the L2 gcd file is provided (probably in the case when running on your own cluster and when you copied the diff and L2 GCDs there),                                                                 
# then you use this, but you check first whether the filename makes sense (is the same as the base_filename used for compression).                                                                       
def CheckL2GCD(frame):
    geodiff=frame["I3GeometryDiff"]
    if args.L2_gcdfile:
        L2_GCD = args.L2_gcdfile
        if os.path.basename(L2_GCD) != os.path.basename(geodiff.base_filename):
            icetray.logging.log_fatal("The provided L2 GCD seems not suited to use for uncompressing the L3 GCD. It needs to have the same filename as the L2 GCD used to create the diff.")
    else:
        L2_GCD = geodiff.base_filename
    if not os.path.exists(L2_GCD):
        icetray.logging.log_fatal("L2 GCD file %s not found" % L2_GCD)

tray.AddModule(CheckL2GCD,'CheckL2CD',
                   Streams=[icetray.I3Frame.Geometry])

tray.Add(uncompress,
         base_filename=args.L2_gcdfile) # works correctly if L2_gcdfile is None                    


it_gen=['I3EventHeader']
if args.isMC:
    it_gen=it_gen+['MCPrimary','MCPrimaryInfo']

it_filter=['IceTop_EventPrescale',
           'IceTop_StandardFilter',
           'IceTop_InFillFilter']

## The two-station filter: only exists 2016 and onward
if not (args.detector=="IC79" or args.detector=="IC86.2011" or args.detector=="IC86.2012" or args.detector=="IC86.2013" or args.detector=="IC86.2014" or args.detector=="IC86.2015"):
    it_filter += ['IceTop_TwoStationFilter']

it_pulses=[dict(key =icetop_globals.icetop_clean_hlc_pulses,
                converter = dataclasses.converters.I3RecoPulseSeriesMapConverter(bookGeometry=True),
                name = icetop_globals.icetop_clean_hlc_pulses),
           dict(key =icetop_globals.icetop_clean_hlc_pulses,
                converter = phys_services.converters.I3EventInfoConverterFromRecoPulses(),
                name = icetop_globals.icetop_clean_hlc_pulses+'_info'),
           dict(key =icetop_globals.icetop_HLCseed_clean_hlc_pulses,
                converter = dataclasses.converters.I3RecoPulseSeriesMapConverter(bookGeometry=True),
                name = icetop_globals.icetop_HLCseed_clean_hlc_pulses),
           dict(key =icetop_globals.icetop_HLCseed_clean_hlc_pulses,
                converter = phys_services.converters.I3EventInfoConverterFromRecoPulses(),
                name = icetop_globals.icetop_HLCseed_clean_hlc_pulses+'_info'),
           dict(key =icetop_globals.icetop_HLCseed_clean_hlc_pulses+"_SnowCorrected",
                converter = dataclasses.converters.I3RecoPulseSeriesMapConverter(bookGeometry=True),
                name = icetop_globals.icetop_HLCseed_clean_hlc_pulses+"_SnowCorrected"),
           dict(key =icetop_globals.icetop_HLCseed_clean_hlc_pulses+"_SnowCorrected",
                converter = phys_services.converters.I3EventInfoConverterFromRecoPulses(),
                name = icetop_globals.icetop_HLCseed_clean_hlc_pulses+'_SnowCorrected_info')
           ]


it_reco=["Laputop","LaputopParams",
         dict(key='Laputop',
              converter = phys_services.converters.I3RecoInfoConverter('IceTopHLCSeedRTPulses'),
              name = 'Laputop_info'),
         ]

it_cuts=["IT73AnalysisIceTopQualityCuts"]


book_keys=it_gen+it_filter+it_pulses+it_reco+it_cuts
if args.do_inice:
    from icecube import ddddr, common_variables
    '''
    ic_pulses=[dict(key =icetop_globals.inice_coinc_pulses,
                    converter = dataclasses.converters.I3RecoPulseSeriesMapConverter(bookGeometry=True),
                    name = icetop_globals.inice_coinc_pulses),
               dict(key =icetop_globals.inice_coinc_pulses,
                    converter = phys_services.converters.I3EventInfoConverterFromRecoPulses(),
                    name = icetop_globals.inice_coinc_pulses+'_info'),
               dict(key =icetop_globals.inice_clean_coinc_pulses,
                    converter = dataclasses.converters.I3RecoPulseSeriesMapConverter(bookGeometry=True),
                    name = icetop_globals.inice_clean_coinc_pulses),
               dict(key =icetop_globals.inice_clean_coinc_pulses,
                    converter = phys_services.converters.I3EventInfoConverterFromRecoPulses(),
                    name = icetop_globals.inice_clean_coinc_pulses+'_info'),
               dict(key ='SRT'+icetop_globals.inice_coinc_pulses,
                    converter = dataclasses.converters.I3RecoPulseSeriesMapConverter(bookGeometry=True),
                    name = 'SRT'+icetop_globals.inice_coinc_pulses),
               dict(key ='SRT'+icetop_globals.inice_coinc_pulses,
                    converter = phys_services.converters.I3EventInfoConverterFromRecoPulses(),
                    name = 'SRT'+icetop_globals.inice_coinc_pulses+'_info'),

        ]
    '''
    # Make some selection
    ic_pulses=[dict(key ='SRT'+icetop_globals.inice_coinc_pulses,
                    converter = phys_services.converters.I3EventInfoConverterFromRecoPulses(),
                    name = 'SRT'+icetop_globals.inice_coinc_pulses+'_info'),
               dict(key =icetop_globals.inice_clean_coinc_pulses,
                    converter = phys_services.converters.I3EventInfoConverterFromRecoPulses(),
                    name = icetop_globals.inice_clean_coinc_pulses+'_info')]
   
    '''
    ic_reco=["Millipede",
             "MillipedeFitParams",
             "Millipede_dEdX",
             "I3MuonEnergyLaputopParams",
             'CoincMuonReco_SPEFit2',
             'CoincMuonReco_SPEFit2FitParams',
             'CoincMuonReco_MPEFit',
             'CoincMuonReco_MPEFitFitParams',
             'CoincMuonReco_MPEFitMuEX',
             'CoincMuonReco_MPEFitCharacteristics',
             'CoincMuonReco_SPEFit2Characteristics',
             'CoincMuonReco_SPEFit2_D4R_Params',
             'CoincMuonReco_MPEFitTruncated_BINS_Muon',
             'CoincMuonReco_MPEFitTruncated_AllBINS_Muon',
             'CoincMuonReco_MPEFitTruncated_ORIG_Muon',
             ]
    '''
    # Also some selection
    ic_reco=["Millipede",
             "MillipedeFitParams",
             "Millipede_dEdX",
             "I3MuonEnergyLaputopParams",
             'CoincMuonReco_MPEFit',
             'CoincMuonReco_MPEFitFitParams',
             'CoincMuonReco_SPEFit2_D4R_Params',
             "Stoch_Reco",
             "Stoch_Reco2"]
    
    ic_cuts=['IT73AnalysisInIceQualityCuts']

    book_keys=book_keys+ic_pulses+ic_reco+ic_cuts
    
tray.AddModule(I3TableWriter, "table_writer",
               TableService = table_service,
               SubEventStreams = ['ice_top'],
               Keys = book_keys)

# Execute the Tray                                                                                                                                                                                       
if args.n is None:
    tray.Execute()
else:
    tray.Execute(args.n)
    

