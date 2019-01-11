from icecube import icetray

@icetray.traysegment
def EnergylossReco(tray, name,
                   InIcePulses='CleanCoincPulses',
                   dom_eff=0.99,
                   IceTopTrack='Laputop',
                   splinedir='/data/sim/sim-new/downloads/spline-tables/',
                   If= lambda fr: True,
                   ):

    from icecube import icetop_Level3_scripts, stochastics, dataclasses, millipede, photonics_service, ddddr

    cascade_service = photonics_service.I3PhotoSplineService(splinedir+'ems_mie_z20_a10.abs.fits', splinedir+'ems_mie_z20_a10.prob.fits', 30)

    tray.AddModule('MuMillipede', name+'_millipede_highenergy',
                   # MuonPhotonicsService=muon_service,                                                                                                                                        
                   CascadePhotonicsService=cascade_service,
                   PhotonsPerBin=600,
                   UseUnhitDOMs= True,
                   MuonSpacing=0,
                   ShowerSpacing=20,
                   StartSlantDepth=1300,
                   EndSlantDepth=2600,
                   SeedTrack=IceTopTrack,
                   ExcludedDOMs=[InIcePulses+'_Balloon','BadDomsList', 'CalibrationErrata', 'SaturationWindows'],
                   DOMEfficiency=dom_eff,  #fraction of DOM surface unshadowed by cable and active                                                                                                          
                   PartialExclusion=True, #exclude only marked time ranges from ExcludedDOMs                                                                                                              
                   #ReadoutWindow='', #(the default) will use the name of the pulse series with "TimeRange" appended to the end. As output by AddMissingTimeWindow!!                                        
                   Output='Millipede',
                   Pulses=InIcePulses,
                   If= If)

        #optimized in tuneStoch                                                                                                                                                                            
    tray.AddModule(icetop_Level3_scripts.modules.MassageMillipedeOutput,name+'_massage',
                   ShowerSpacing = 20, # Should be the same as above I think.
                   min_energy = -1., #0.,  #v2 : NEW : TAKE INTO ACCOUNT zero bins !!                                                                                                                      
                   inputname = 'Millipede',
                   outputname = 'Millipede_dEdX',
                   min_dust = -140,
                   max_dust = -30,
                   fill_dust = False,
                   If= If)
    
        #optimized in tuneStoch                                                                                                                                                                            
     # This one works well up to 100 PeV                                                                                                                                                                  
    tray.AddModule('I3Stochastics',name+'_stochme',
                   A_Param = 0,
                   B_Param = 5,
                   C_Param = 0.8,
                   FreeParams = 1,
                   InputParticleVector='Millipede_dEdX',
                   Minimizer = 'MIGRAD',
                   OutputName = 'Stoch_Reco',
                   OutputName_red = 'Stoch_Reco_red',
                   Verbose = False,
                   SelectionType = 'Type2',
                   If= If)

    # This one is a stronger selection that works well from 30 PeV onwards                                                                                                                                 
    tray.AddModule('I3Stochastics',name+'_stochme2',
                   A_Param = 0,
                   B_Param = 7,
                   C_Param = 0.9,
                   FreeParams = 1,
                   InputParticleVector='Millipede_dEdX',
                   Minimizer = 'MIGRAD',
                   OutputName = 'Stoch_Reco2',
                   OutputName_red = 'Stoch_Reco2_red',
                   Verbose = False,
                   SelectionType = 'Type2',
                   If= If)

    tray.AddModule("I3MuonEnergy",name+"_d4r",
                   Seed=IceTopTrack,
                   BinWidth=50,    #Default
                   MaxImpact= 100.0, #Default
                   InputPulses=InIcePulses,
                   If=If
                   )

