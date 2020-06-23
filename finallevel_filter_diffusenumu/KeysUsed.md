Summary
=======
Needed from before L4: "TWSRTHVInIcePulsesIC"
                       "SplineMPE"
                       "I3MCTree"
                       "SplineMPETruncatedEnergy_SPICEMie_AllDOMS_Muon"
                       "SplineMPETruncatedEnergy_SPICEMie_DOMS_Muon"
                       "SplineMPETruncatedEnergy_SPICEMie_AllBINS_Muon"
                       "SplineMPETruncatedEnergy_SPICEMie_BINS_Muon"
                       "SplineMPETruncatedEnergy_SPICEMie_ORIG_Muon"
                       "SPEFit2GeoSplit2"
                       "MPEFitParaboloidFitParams"
                       "DeepCoreDOMs"
                       "BrightDOMs"
                       "SaturationWindows"
                       "BadDomsList"
                       "CalibrationErrata"
                       "MillipedeHVSplitPulsesExcludedTimeRange"
                       "MillipedeHVSplitPulsesReadoutWindow"
                       "MillipedeHVSplitPulses"
Level4
======
   
CutZenith
---------
Needs: "SplineMPE"

removeBugEvents( MC only)
-------------------------
Needs: "I3MCTree"
       
CutNCh
------
Needs: "TWSRTHVInIcePulsesIC"

SplineMPEIC
-----------
Generates: SplineMPEIC
Needs : "SplineMPETruncatedEnergy_SPICEMie_AllDOMS_Muon"
        "SplineMPETruncatedEnergy_SPICEMie_DOMS_Muon"
        "SplineMPETruncatedEnergy_SPICEMie_AllBINS_Muon"
        "SplineMPETruncatedEnergy_SPICEMie_BINS_Muon"
        "SplineMPETruncatedEnergy_SPICEMie_ORIG_Muon"
        "TWSRTHVInIcePulsesIC"
        "SplineMPE"

CommonVarables
--------------
Generates: "SplineMPEICDirectHitsIC"
Generates: "SplineMPEICCharacteristicsIC"
Needs: "SplineMPEIC"
       "TWSRTHVInIcePulsesIC"

PrecutFilter
------------
Needs: "SplineMPEIC"
       "HitMultiplicityValuesIC"
       "SplineMPEICDirectHitsICC"
    
BasicRecos
----------
Generates: "LineFitIC"
           "SPEFitSingleIC"
           "SPEFit2IC"
           "MPEFitIC"
Needs: "TWSRTHVInIcePulsesIC"

BayesianRecos
-------------
Generates: "SPEFit2BayesianIC"
Needs: "TWSRTHVInIcePulsesIC"
       "SplineMPEIC"
    
EnergyRecos
-----------
Generates: "SplineMPEICTruncatedEnergySPICEMie"
           "SplineMPEICMuEXDifferential"
Needs: "TWSRTHVInIcePulsesIC"
       "SplineMPEIC"

Level5
======

Score-Cut
---------
Generates:
        "Score"
        "CascScore"
        "L5_cog_rho"
        "L5_cog_z"  
        "L5_lseparation" 
        "L5_nch" 
        "L5_bayes_llh_diff" 
        "L5_cos_zenith"
        "L5_rlogl"
        "L5_ldir_c"          
        "L5_ndir_c"          
        "L5_sigma_paraboloid"
        "L5_sdir_e"         
Needs: "SplineMPEICCharacteristicsIC"
        "SPEFit2GeoSplit2"
        "HitStatisticsValuesIC"
        "SplineMPEICCharacteristicsIC"
        "HitMultiplicityValuesIC"
        "SPEFit2BayesianICFitParams"
        "SPEFit2ICFitParams"
        "SplineMPEIC"
        "SplineMPEICFitParams"
        "SplineMPEICDirectHitsICC"
        "MPEFitParaboloidFitParams"
        "SplineMPEICDirectHitsICE"

Millipede
---------
Generates: "SplineMPEIC_MillipedeHighEnergyMIE"
Needs:  "DeepCoreDOMs"
        "BrightDOMs"
        "SaturationWindows"
        "BadDomsList"
        "CalibrationErrata"
        "MillipedeHVSplitPulsesExcludedTimeRange"
        "SplineMPEIC"
        "MillipedeHVSplitPulsesReadoutWindow"
        "MillipedeHVSplitPulses"


Paraboloid
----------
Generates: SplineMPEICParaboloid
Needs : "TWSRTHVInIcePulsesIC"
        "SplineMPEIC"
        "SplineMPETruncatedEnergy_SPICEMie_AllDOMS_Muon",
        "SplineMPETruncatedEnergy_SPICEMie_DOMS_Muon",
        "SplineMPETruncatedEnergy_SPICEMie_AllBINS_Muon",
        "SplineMPETruncatedEnergy_SPICEMie_BINS_Muon",
        "SplineMPETruncatedEnergy_SPICEMie_ORIG_Muon",



