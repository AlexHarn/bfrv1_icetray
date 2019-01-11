from icecube import icetray

@icetray.traysegment
def muonReconstructions(tray, name,
                        Pulses='CoincPulses',
                        prefix='CoincMuonReco_',
                        If= lambda f: True):
    
    # Let's first do SeededRT cleaning on the pulses, since there is still a lot of noise remaining after coinc-twc, which only removes pulses outside the time window. 
    # Taken all the settings from muon L2.
    srtPulses="SRT"+Pulses
    tray.AddModule( "I3SeededRTHitCleaningModule<I3RecoPulse>", name+"_coinc_srt",
                    InputResponse = Pulses ,    # ! Name of input                                                                                                                                        
                    OutputResponse = srtPulses,        # ! Name of output                                                                                                                          
                    #DiscardedHLCResponse = "SRTexcluded"+Pulses, # ! Name of HLC output that were cleaned out in SeededR. Let's see whether we need it.    
                    RTRadius = 150 ,                            # ! Radius for RT cleaning                                                                                                                  
                    RTTime = 1000 ,                              # ! Time for RT cleaning                                                                                                                   
                    MaxIterations = 3 ,              # 3 iterations are enough.                                                                                                                           
                    Seeds = "HLCcore" ,              # ! do not use all HLC hits as seed                                                                                                                    
                    HLCCoreThreshold = 2 ,           # Default                                                                   
                    If= If
                    )            

    # Create RecoPulseSeriesMapMask instead of RecoPulseSeriesMap                                                                                                                                    
    from icecube import icetop_Level3_scripts
    tray.AddModule(icetop_Level3_scripts.functions.makePulseMask,name+"_createSRTMask",
                   PrimaryPulsesName=Pulses,
                   SecondaryPulsesName=srtPulses,
                   SecondaryMaskName=srtPulses)

    # Creates 'CoincMuonReco_LineFit', 'CoincMuonReco_SPEFitSingle', 'CoincMuonReco_SPEFit2', 'CoincMuonReco_LineFitParams','CoincMuonReco_SPEFitSingleFitParams','CoincMuonReco_SPEFit2FitParams', 'CoincMuonReco_SPEFit2CramerRaoParams'
    tray.AddSegment(SPE,name+"_SPE",
                    Pulses=srtPulses,
                    prefix=prefix,
                    If= lambda fr: srtPulses in fr)
        
    # Creates  'CoincMuonReco_MPEFit','CoincMuonReco_MPEFitFitParams', 'CoincMuonReco_MPEFitCramerRaoParams',
    tray.AddSegment(MPE,name+"_MPE",
                    Pulses=srtPulses,
                    prefix=prefix,
                    Seed='SPEFit2',
                    If=lambda fr: srtPulses in fr)
    
    tray.AddSegment(add_hit_verification_info, 'CommonVariables', 
                    Pulses= srtPulses, 
                    OutputI3HitMultiplicityValuesName=  prefix+"CVMultiplicity",
                    OutputI3HitStatisticsValuesName= prefix+"CVStatistics",
                    prefix = prefix,
                    If = lambda fr: srtPulses in fr)

        
    tray.AddSegment(Truncated, name+'Truncated_SPICEMie', 
                    Pulses = srtPulses, 
                    Seed = prefix+'MPEFit', 
                    prefix = prefix,
                    PhotonicsService = 'PhotonicsServiceMu_SpiceMie',
                    Model = '_SPICEMie',
                    Truncated = 'Truncated',
                    If = lambda fr: srtPulses in fr
                    )

    tray.AddSegment(MuEX, name+'MuEX', 
                    Pulses = srtPulses, 
                    Seed = 'MPEFit', 
                    MuEX = 'MPEFitMuEX',
                    prefix = prefix,
                    If = lambda fr: srtPulses in fr 
                    )
                
    from icecube import ddddr
    tray.AddModule("I3MuonEnergy",name+"_d4r",
                   Seed=prefix+'SPEFit2',
                   Prefix=prefix+'SPEFit2_'+'D4R_',
                   BinWidth=50,    #Default                                                                                                                                                                 
                   MaxImpact= 100.0, #Default                                                                                                                                                               
                   InputPulses=srtPulses,
                   If=lambda fr: srtPulses in fr
                   )
    
@icetray.traysegment
def SPE(tray, name, Pulses = '',  
        prefix = '',
        LineFit = 'LineFit',
        SPEFitSingle = 'SPEFitSingle',
        SPEFit = 'SPEFit2',
        SPEFitCramerRao = 'SPEFit2CramerRao',
        N_iter = 2,
        If = lambda f: True,
        ):

    from icecube import linefit, improvedLinefit, lilliput, cramer_rao
    import icecube.lilliput.segments

    # Creates Linefit and Linefit+params
    tray.AddSegment( improvedLinefit.simple, prefix+LineFit, inputResponse = Pulses, fitName = prefix+LineFit, If = If )
    
    # Creates SPEFitSingle + SPEFitSingleFitparams
    tray.AddSegment( lilliput.segments.I3SinglePandelFitter, prefix+SPEFitSingle, pulses = Pulses, seeds = [prefix+LineFit], If = If )
    
    # Creates SPEFit2+ SPEFit2FitParams
    if N_iter > 1:
        tray.AddSegment( lilliput.segments.I3IterativePandelFitter, prefix+SPEFit, pulses = Pulses, n_iterations = N_iter, seeds = [ prefix+SPEFitSingle ], If = If )

    
@icetray.traysegment
def MPE(tray, name, 
        Pulses = '', 
        Seed = '', 
        prefix = '',
        MPEFit = 'MPEFit',
        MPEFitCramerRao = 'MPEFitCramerRao',
        If = lambda f: True, 
        ):

    from icecube import lilliput, cramer_rao
    import icecube.lilliput.segments

    tray.AddSegment( lilliput.segments.I3SinglePandelFitter, prefix+MPEFit, pulses = Pulses, seeds = [ prefix+Seed ], domllh = 'MPE', If = If)

@icetray.traysegment
def MuEX(tray, name, 
         Pulses = '', 
         Seed = '', 
         prefix = '',
         MuEX = 'MPEFitMuEX',
         If = lambda f: True, 
         ):

    from icecube import mue
    tray.AddModule( 'muex', name + '_' + prefix+MuEX,
                    pulses = Pulses,  # ! Name of pulses (multi-pulses per DOM)                                                                                                              
                    rectrk = prefix+Seed, # ! Name of reconstruction seed used for energy reco                                                                                                              
                    result = prefix+MuEX, # ! Name output particle                                                                                                                                          
                    detail = False, # Default                                                                                                                                                               
                    lcspan = 0, # Default                                                                                                                                                                   
                    If = If,
                    )

@icetray.traysegment
def Truncated(tray, name,
              Pulses = '',
              Seed = '',
              prefix = '',
              PhotonicsService = 'PhotonicsServiceMu_SpiceMie',
              Model = '',
              Truncated = 'Truncated',
              If = lambda f: True,
              ):

    from icecube import photonics_service
    icetray.load('truncated_energy',False)

    TruncatedName = Seed+Truncated # ! base result name to put into frame                                                                                                                      

    '''                                                                                                                                                                                                     
    Truncated outputs several potential frames objects with the following extensions to the name:                                                                                                           
    _ORIG_Muon                                                                                                                                                                                              
    _ORIG_Neutrino                                                                                                                                                                                          
    _ORIG_dEdX                                                                                                                                                                                              
    _DOMS_MuEres                                                                                                                                                                                            
    _DOMS_Muon                                                                                                                                                                                              
    _DOMS_Neutrino                                                                                                                                                                                          
    _BINS_MuEres                                                                                                                                                                                          
    _BINS_Muon                                                                                                                                                                                              
    _BINS_Neutrino                                                                                                                                                                                          
    IceTop Level3 coinc reco will only use _BINS_Muon and _ORIG_Muon. And _AllBINS_Muon
    '''
    
   # Muons SPICEMie                                                                                                                                                                                         
    tray.AddService( 'I3PhotonicsServiceFactory',name+PhotonicsService,
                     #UseDummyService=True,
                     ServiceName=name+PhotonicsService,    
                     PhotonicsTopLevelDirectory="/cvmfs/icecube.opensciencegrid.org/data/photon-tables/SPICEMie", # ! Path to the directory containing subdirectory with tables       
                     DriverFileDirectory="/cvmfs/icecube.opensciencegrid.org/data/photon-tables/SPICEMie/driverfiles",               # ! Directory where the driver \files are located      
                     PhotonicsLevel2DriverFile="mu_photorec.list",              # ! Name of level 2 driver file                                                                   
                     PhotonicsTableSelection=2 ,                       # ! Use level2 tables                                                                                                     
                     )

    tray.AddModule( 'I3TruncatedEnergy', name + prefix+'_Truncated' + Model,
                    RecoPulsesName = Pulses, # ! Name of pulses                                                                                                                                             
                    RecoParticleName = Seed, # ! Name of input reconstructed Track                                                                                                                   
                    ResultParticleName = TruncatedName, # ! Name of result Particle                                                                                                                         
                    I3PhotonicsServiceName = name+PhotonicsService,  # ! Name of photonics service to use                                                                                                   
                    UseRDE = True, # ! Correct for HQE DOMs                                                                                                                                                 
                    If = If,
                    )


@icetray.traysegment
def add_hit_verification_info(tray, name, 
                              pulses = '', 
                              ConsiderOnlyFirstPulse= '',  
                              OutputI3HitMultiplicityValuesName= '',
                              OutputI3HitStatisticsValuesName= '',
                              prefix='',
                              If = lambda f: True,
                              ):

    """Adds hit information to the frame for verification purposes.                                                                                                                                         
    """

    from icecube.common_variables import hit_multiplicity, hit_statistics, track_characteristics, direct_hits

    dh_defs = direct_hits.get_default_definitions()

    tray.AddSegment(direct_hits.I3DirectHitsCalculatorSegment, name+'_dh',
                    DirectHitsDefinitionSeries       = dh_defs,
                    PulseSeriesMapName               = pulses,
                    ParticleName                     = prefix+'MPEFit',
                    OutputI3DirectHitsValuesBaseName = prefix+'MPEFit'+'DirectHits',
                    BookIt                           = False,
                    If                               = If
                    )
 
    tray.AddModule(hit_multiplicity.I3HitMultiplicityCalculator, name+'_I3HitMultiplicityCalculator',   
                   If= If,
                   PulseSeriesMapName = pulses,
                   OutputI3HitMultiplicityValuesName = OutputI3HitMultiplicityValuesName,
                   # ConsiderOnlyFirstPulse= ConsiderOnlyFirstPulse                                                                                                                                    
                   )

    tray.AddModule(hit_statistics.I3HitStatisticsCalculator, name+'_I3HitStatisticsCalculator',
                   If= If,
                   PulseSeriesMapName = pulses,
                   OutputI3HitStatisticsValuesName = OutputI3HitStatisticsValuesName,
                   #        ConsiderOnlyFirstPulse                 = ConsiderOnlyFirstPulse                                                                                                                                     
                   )

    track_cylinder_radius = 150
    
    tray.AddSegment(track_characteristics.I3TrackCharacteristicsCalculatorSegment, name+'_tc_mpe',
                    If = If,
                    PulseSeriesMapName                     = pulses,
                    ParticleName                           = prefix+'MPEFit',
                    OutputI3TrackCharacteristicsValuesName = prefix+'MPEFit'+'Characteristics',
                    TrackCylinderRadius                    = track_cylinder_radius,
                    BookIt                                 = True
                    )


    tray.AddSegment(track_characteristics.I3TrackCharacteristicsCalculatorSegment, name+'_tc_spe2',
                    If = If,
                    PulseSeriesMapName                     = pulses,
                    ParticleName                           = prefix+'SPEFit2',
                    OutputI3TrackCharacteristicsValuesName = prefix+"SPEFit2"+'Characteristics',
                    TrackCylinderRadius                    = track_cylinder_radius,
                    BookIt                                 = True
                    )


