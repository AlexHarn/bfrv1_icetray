#!/usr/bin/env python
from icecube import icetray
import re, glob, os, sys

def make_parser():
    from optparse import OptionParser
    parser = OptionParser(usage='%s [options] {N}'%os.path.basename(sys.argv[0]))
    parser.add_option("-o", "--output", action="store", type=str, dest="outfile", help="Output file name", metavar="BASENAME")
    parser.add_option("-n", action="store", type=int, dest="n", help="number of frames to process")
    parser.add_option("-d", "--det", dest="detector",
                      help="Detector configuration name, eg: IC79, IC86.2011, IC86.2012. Auto-detected if filename has standard formatting.")
    parser.add_option("--waveforms", action="store_true", dest="waveforms", help="Extract waveforms (only if more than 5 stations in HLC VEM pulses)")
    parser.add_option("--select", action="store_true", dest="select", help="Apply selection (containment, max. signal and min. stations). Default is to calculate variables but not filter", default=False)
    parser.add_option("--print-usage", action="store_true", dest="print_usage", help="Print CPU time usage at the end")
    parser.add_option("--debug", action="store_const", dest="log_level", help="Trace log-level", const=1, metavar="N", default=2)
    parser.add_option("--trace", action="store_const", dest="log_level", help="Trace log-level", const=0, metavar="N", default=2)
    parser.add_option('--L3-gcdfile', dest='L3_gcdfile', help='Manually specify the L3 (diff) GCD file to be used. When you run in Madison, this is not needed.')
    parser.add_option('--L2-gcdfile', dest='L2_gcdfile', help='Manually specify the L2 GCD file to be used. When you run in Madison with the standard L3 GCD diff, this is not needed.')
    parser.add_option("-m","--isMC", action="store_true",dest="isMC", help= "Is this data or MC?")
    parser.add_option("--dataset",dest="dataset", type=int,help= "Dataset number for MC. Needed when using default GCD, to look for it..")
    parser.add_option("--run",dest="run", type=int,help= "Runnumber, needed for data. Needed when using default GCD, to look for it..")
    parser.add_option("--add-jitter",action="store_true",dest="add_jitter",help="Do we add extra jitter on the IT pulses?")
    parser.add_option("--do-inice", action="store_true",dest="do_inice",help= "Also do in-ice reco?")
    parser.add_option("--histos", action="store", type=str, dest="histos", help="Histograms file name. Needs to be pickle file", metavar="BASENAME")
    parser.add_option("--livetime", action="store", type=str, dest="livetime", help="Livetime file name, only needed for data. Needs to be pickle file", metavar="BASENAME")
    parser.add_option("--spe-corr",action="store_true",dest="spe_corr",help="Should we do the SPE correction Level3?")
    parser.add_option("--snow-lambda", action="store", type=float, dest="snowLambda", help="Attenuation factor for snow. If not defined, use the one defined for every year.")
    parser.add_option("-i","--infile",action="store",dest="infile",type=str,help="List of infiles")
    #parser.add_option('inputFiles',help="Input file(s)",type=str,nargs="*")
    
    return parser

def get_detector_from_filename(input_file):
    m = re.search("Level[^_]+_(IC[^_]+)_", input_file)
    if not m:
        raise ValueError("cannot parse %s for detector config" % input_file)
    return m.group(1)


def get_run_from_filename(input_file):
    result = None
    m = re.search("Run([0-9]+)", input_file)
    if not m:
        raise ValueError("cannot parse %s for Run number" % input_file)
    return int(m.group(1))

def get_dataset_from_filename(input_file):
    result = None
    m = re.search("icetop.00([0-9]+).", input_file) # Works for IC79 MC 
    if not m:
        raise ValueError("cannot parse %s for dataset" % input_file)
    return int(m.group(1))

def main(options, stats={}):
    from I3Tray import I3Tray
    from icecube import dataio, icetop_Level3_scripts, dataclasses, phys_services, frame_object_diff
    from icecube.icetop_Level3_scripts import icetop_globals

    # Make options['infile'] a list
    if not isinstance(options['infile'], list):
        options['infile'] = [options['infile']]

    print "infiles: %s" % options['infile']

    ok=True
    # First go over all options and try to find what you need. If this fails, log_fatal.
    if not len(options['infile']) > 0:
        icetray.logging.log_error("No input files found!")
        ok=False
    else:
        # try to supply some args by parsing the input filename
        if options['detector'] is None:
            options['detector'] = get_detector_from_filename(options['infile'][0])
            icetray.logging.log_info("Auto-detected detector %s" % options['detector'])
        if not options['isMC']:  # Filename is only really needed for data...
            if options['run'] is None:
                options['run'] = get_run_from_filename(options['infile'][0])
                icetray.logging.log_info("Auto-detected run %i" % options['run'])
    if not options['outfile']:
        icetray.logging.log_error("Output file not specified!")
        ok=False
    else:
        if len(os.path.dirname(options['outfile']))>0 and not os.path.isdir(os.path.dirname(options['outfile'])):
            icetray.logging.log_error("Output directory not found.")
            ok=False

    if not options['L3_gcdfile']:
        icetray.logging.log_info("Using the default L3 GCD.")
        if options['isMC']:
            if not options['dataset']:
                options['dataset']=get_dataset_from_filename(options['infile'][0])
                icetray.logging.log_info("Auto-detected dataset %i" %options['dataset'])
            
            else:
                if not os.path.exists("/data/ana/CosmicRay/IceTop_level3/sim/%s/GCD/Level3_%i_GCD.i3.gz"%(options['detector'], options['dataset'])):
                    icetray.logging.log_error("The default L3 GCD file is not found.")
                    ok=False
        else:
            if not options['run']:
                icetray.logging.log_error("When using the default L3 GCD for data, you need to specify the run number such that we can look for the correct GCD!")
                ok=False
            else:
                if not os.path.exists(glob.glob("/data/ana/CosmicRay/IceTop_level3/exp/%s/GCD/Level3_%s_data_Run00%i_????_GCD.i3.gz"%(options['detector'], options['detector'], options['run']))[0]):
                    icetray.logging.log_error("Default L3 file not found.")
                    ok=False
    else:
        icetray.logging.log_info("Using a user specified L3 GCD file.")
        if not os.path.exists(options['L3_gcdfile']):
            icetray.logging.log_error(" L3 GCD file not found")
            ok=False

    if not options['L2_gcdfile']:
        icetray.logging.log_info("Using the default L2 GCD.")
    else:
        icetray.logging.log_info("Using a user specified L2 GCD file. This should be the same as the one where the diff is create against! If no error is showing up, this is fine!")
        if not os.path.exists(options['L2_gcdfile']):
            icetray.logging.log_error(" L2 GCD file not found.")
            ok=False

    if not options['isMC']:
        if options['livetime']:
            icetray.logging.log_info("Storing the livetime histo in %s."%options['livetime'])
            if len(os.path.dirname(options['livetime']))>0 and not os.path.isdir(os.path.dirname(options['livetime'])):
                icetray.logging.log_error("Directory to store livetime pickle file not found.")
                ok=False
            if not options['livetime'][-7:]==".pickle":
                icetray.logging.log_error("livetime parameter needs to be a .pickle file.")
                ok=False

    if options['histos']:
        icetray.logging.log_info("Storing the histos in %s."%options['histos'])
        if len(os.path.dirname(options['histos']))>0 and not os.path.isdir(os.path.dirname(options['histos'])):
            icetray.logging.log_error("Directory to store histos pickle file not found.")
            ok=False
        if not options['histos'][-7:]==".pickle":
            icetray.logging.log_error("histos parameter needs to be a .pickle file.")
            ok=False

    # Now check
    if not ok:
        #parser.print_help()
        icetray.logging.log_fatal("Exit.")
    
    # For running, we'll use these logging settings.
    icetray.I3Logger.global_logger.set_level(icetray.I3LogLevel.LOG_ERROR)
    icetray.I3Logger.global_logger.set_level_for_unit("MakeQualityCuts",icetray.I3LogLevel.LOG_INFO)
    
    if not options['L3_gcdfile']:
        if options['isMC']:
            gcdfile=["/data/ana/CosmicRay/IceTop_level3/sim/%s/GCD/Level3_%i_GCD.i3.gz"%(options['detector'], options['dataset'])]
        else:
            gcdfile=glob.glob("/data/ana/CosmicRay/IceTop_level3/exp/%s/GCD/Level3_%s_data_Run00%i_????_GCD.i3.gz"%(options['detector'], options['detector'], options['run']))
    else:
        gcdfile = [options['L3_gcdfile']]
        
    # Instantiate a tray
    tray = I3Tray()
    tray.AddModule( 'I3Reader', 'Reader', FilenameList = gcdfile + options['infile'])

    from icecube.frame_object_diff.segments import uncompress
    
    # If the L2 gcd file is not specified, use the base_filename which is used for compressing. Check First whether it exists.
    # If the L2 gcd file is provided (probably in the case when running on your own cluster and when you copied the diff and L2 GCDs there), 
    # then you use this, but you check first whether the filename makes sense (is the same as the base_filename used for compression). 
    def CheckL2GCD(frame):
        geodiff=frame["I3GeometryDiff"]
        if options['L2_gcdfile']:
            L2_GCD = options['L2_gcdfile']
            if os.path.basename(L2_GCD) != os.path.basename(geodiff.base_filename):
                icetray.logging.log_fatal("The provided L2 GCD seems not suited to use for uncompressing the L3 GCD. It needs to have the same filename as the L2 GCD used to create the diff.")
        else:
            L2_GCD = geodiff.base_filename
        if not os.path.exists(L2_GCD):
            icetray.logging.log_fatal("L2 GCD file %s not found" % L2_GCD)

    tray.AddModule(CheckL2GCD,'CheckL2CD',
                   Streams=[icetray.I3Frame.Geometry])

    tray.Add(uncompress,
             base_filename=options['L2_gcdfile']) # works correctly if L2_gcdfile is None
        
    tray.AddSegment(icetop_Level3_scripts.segments.level3_IceTop, "level3_IceTop",
                    detector=options['detector'],
                    do_select = options['select'],
                    isMC=options['isMC'],
                    add_jitter=options['add_jitter'],
                    snowLambda=options['snowLambda']
                    )
    
    if options['do_inice']:
        itpulses='IceTopHLCSeedRTPulses'
            
        if options['spe_corr']:
            def fix_spe(frame,pulses):
                if pulses in frame: # Although InIcePulses should always be there                                                                                                                            
                    corr = dataclasses.I3RecoPulseSeriesMapApplySPECorrection(pulses,"I3Calibration")
                    corrpulses = corr.apply(frame)
                    frame.Delete(pulses)
                    frame[pulses] = corrpulses
                return True

            tray.AddModule(fix_spe, "fixspe",pulses=icetop_globals.names["Level3"]["InIcePulses"],Streams=[icetray.I3Frame.DAQ, icetray.I3Frame.Physics])

        tray.AddSegment(icetop_Level3_scripts.segments.level3_Coinc,"level3_Coinc",
                        Detector=options['detector'],
                        isMC=options['isMC'],
                        do_select=options['select'],
                        IceTopTrack='Laputop',
                        IceTopPulses=itpulses,
                        )

    if options['waveforms']:
        from icecube.icetop_Level3_scripts.functions import count_stations
        tray.AddModule(icetop_Level3_scripts.modules.FilterWaveforms, 'FilterWaveforms',   #Puts IceTopWaveformWeight in the frame.
                       pulses=icetop_globals.icetop_hlc_pulses,
                       If = lambda frame: icetop_globals.icetop_hlc_pulses in frame and count_stations(dataclasses.I3RecoPulseSeriesMap.from_frame(frame, icetop_globals.icetop_hlc_pulses)) >= 5)
        tray.AddSegment(icetop_Level3_scripts.segments.ExtractWaveforms, 'IceTop',
                       If= lambda frame: "IceTopWaveformWeight" in frame and frame["IceTopWaveformWeight"].value!=0)    
                  
    ## Which keys to keep:
    wanted_general=['I3EventHeader',
                    icetop_globals.filtermask,
                    'I3TriggerHierarchy']

    if options['isMC']:
        wanted_general+=['MCPrimary',
                         'MCPrimaryInfo',
                         'AirShowerComponents',
                         'IceTopComponentPulses_Electron',
                         'IceTopComponentPulses_ElectronFromChargedMesons',
                         'IceTopComponentPulses_Gamma',
                         'IceTopComponentPulses_GammaFromChargedMesons',
                         'IceTopComponentPulses_Muon',
                         'IceTopComponentPulses_Hadron',
                         ]

    wanted_icetop_filter=['IceTop_EventPrescale',
                          'IceTop_StandardFilter',
                          'IceTop_InFillFilter',
                          'IceTop_TwoStationFilter']
 
    wanted_icetop_pulses=[icetop_globals.icetop_hlc_pulses,
                          icetop_globals.icetop_slc_pulses,
                          icetop_globals.icetop_clean_hlc_pulses,
                          icetop_globals.icetop_tank_pulse_merger_excluded_tanks,
                          icetop_globals.icetop_cluster_cleaning_excluded_tanks,
                          icetop_globals.icetop_HLCseed_clean_hlc_pulses,
                          icetop_globals.icetop_HLCseed_excluded_tanks,
                          icetop_globals.icetop_HLCseed_clean_hlc_pulses+'_SnowCorrected',
                          'TankPulseMergerExcludedSLCTanks',
                          'IceTopLaputopSeededSelectedHLC',  
                          'IceTopLaputopSeededSelectedSLC',                                                                                                                                               
                          'IceTopLaputopSmallSeededSelectedHLC',  
                          'IceTopLaputopSmallSeededSelectedSLC',                                                                                                                                         
                          ]

    wanted_icetop_waveforms=['IceTopVEMCalibratedWaveforms',
                             'IceTopWaveformWeight']

    wanted_icetop_reco=['ShowerCOG',
                        'ShowerPlane',
                        'ShowerPlaneParams',
                        'Laputop',
                        'LaputopParams',
                        'LaputopSnowDiagnostics',
                        'LaputopSmall',
                        'LaputopSmallParams',
                        'IsSmallShower'
                        ]
    
    wanted_icetop_cuts=['Laputop_FractionContainment',
                        'Laputop_OnionContainment',
                        'Laputop_NearestStationIsInfill',
                        'StationDensity',
                        'IceTopMaxSignal',
                        'IceTopMaxSignalInEdge',
                        'IceTopMaxSignalTank',
                        'IceTopMaxSignalString',
                        'IceTopNeighbourMaxSignal',
                        'IT73AnalysisIceTopQualityCuts',
                        ]

    wanted=wanted_general+wanted_icetop_filter+wanted_icetop_pulses+wanted_icetop_waveforms+wanted_icetop_reco+wanted_icetop_cuts
    
    wanted_sta2 = wanted_general + wanted_icetop_filter + \
                  [icetop_globals.icetop_hlc_pulses,
                   icetop_globals.icetop_slc_pulses,
                   icetop_globals.icetop_clean_hlc_pulses,
                   icetop_globals.icetop_cluster_cleaning_excluded_tanks,
                   icetop_globals.icetop_tank_pulse_merger_excluded_tanks,
                   'TankPulseMergerExcludedSLCTanks']

    if options['do_inice']:
        wanted_inice_pulses=[icetop_globals.inice_pulses,
                             icetop_globals.inice_coinc_pulses,
                             icetop_globals.inice_clean_coinc_pulses,
                             icetop_globals.inice_clean_coinc_pulses+"TimeRange",
                             icetop_globals.inice_clean_coinc_pulses+"_Balloon",
                             "SaturationWindows",
                             "CalibrationErrata",
                             'SRT'+icetop_globals.inice_coinc_pulses,
                             'NCh_'+icetop_globals.inice_clean_coinc_pulses]

        wanted_inice_reco=["Millipede",
                           "MillipedeFitParams",
                           "Millipede_dEdX",
                           "Stoch_Reco",
                           "Stoch_Reco2",
                           "I3MuonEnergyLaputopCascadeParams",
                           "I3MuonEnergyLaputopParams"
                           ]
   
        wanted_inice_cuts=['IT73AnalysisInIceQualityCuts']

        wanted_inice_muon=['CoincMuonReco_LineFit',
                           'CoincMuonReco_SPEFit2',
                           'CoincMuonReco_LineFitParams',
                           'CoincMuonReco_SPEFit2FitParams',
                           'CoincMuonReco_MPEFit',
                           'CoincMuonReco_MPEFitFitParams',
                           'CoincMuonReco_MPEFitMuEX',
                           'CoincMuonReco_CVMultiplicity',
                           'CoincMuonReco_CVStatistics',
                           'CoincMuonReco_MPEFitCharacteristics',
                           'CoincMuonReco_SPEFit2Characteristics',
                           'CoincMuonReco_MPEFitTruncated_BINS_Muon',
                           'CoincMuonReco_MPEFitTruncated_AllBINS_Muon',
                           'CoincMuonReco_MPEFitTruncated_ORIG_Muon',
                           'CoincMuonReco_SPEFit2_D4R_CascadeParams',
                           'CoincMuonReco_SPEFit2_D4R_Params',
                           'CoincMuonReco_MPEFitDirectHitsC'
                           ]

        wanted=wanted+wanted_inice_pulses+wanted_inice_reco+wanted_inice_cuts+ wanted_inice_muon

    tray.AddModule("Keep", 'DropObjects',
                   Keys = wanted
                   )

    tray.AddModule("Keep", 'DropObjects_STA2',
                   Keys = wanted_sta2,
                   If = lambda frame: ('IceTop_TwoStationFilter' in frame and frame['IceTop_TwoStationFilter'].value) \
                       and not ('IceTop_StandardFilter' in frame and frame['IceTop_StandardFilter'].value) \
                       and not ('IceTop_InFillFilter' in frame and frame['IceTop_InFillFilter'].value)
                   )

    if options['outfile'].replace('.bz2', '').replace('.gz','')[-3:] == '.i3':
        tray.AddModule("I3Writer", "i3-writer",
                       Filename = options['outfile'],
                       DropOrphanStreams = [ icetray.I3Frame.DAQ ],
                       streams = [icetray.I3Frame.DAQ, icetray.I3Frame.Physics],
                       )
    else:
        raise Exception('I do not know how to handle files with extension %s'%outfile.replace('.bz2', '').replace('.gz','')[-3:])

    if options['livetime'] or options['histos']:
        from icecube.production_histograms import ProductionHistogramModule
        
        if options['histos']:
            tray.AddSegment(icetop_Level3_scripts.segments.MakeHistograms, "makeHistos", OutputFilename=options['histos'], isMC=options['isMC'])

        if not options['isMC'] and options['livetime']:
            tray.Add(ProductionHistogramModule, "LivetimeHistogram",
                     Histograms = [icetop_Level3_scripts.histograms.Livetime],
                     OutputFilename = options['livetime']
                     )

    
    
    # Execute the Tray
    if options['n'] is None:
        tray.Execute()
    else:
        tray.Execute(options['n'])

    if options['print_usage']:
        tray.PrintUsage(fraction=1.0)
    
    
### iceprod stuff ###
try:
    from iceprod.modules import ipmodule
except ImportError, e:
    print 'Module iceprod.modules not found. Will not define IceProd Class'
else:
    Level2Filter = ipmodule.FromOptionParser(make_parser(),main)
### end iceprod stuff ###

if __name__ == "__main__":
    # run as script from the command line
    # get parsed args
    parser = make_parser()
    (options,args) = parser.parse_args()
    opts = {}
    # convert to dictionary
    for name in parser.defaults:
        value = getattr(options,name)
        if name == 'infile' and ',' in value:
            value = value.split(',') # split into multiple inputs
        opts[name] = value

    # call main function
    main(opts)
