from icecube import icetray, dataclasses, dataio
from icecube import common_variables, paraboloid, gulliver, lilliput
from icecube.icetray import I3Units
from icecube.photonics_service import I3PhotoSplineService
from icecube import photonics_service, millipede

import pickle
import sklearn
import numpy as np
import math
import string
import os

# Check for right sklearn version
assert (sklearn.__version__=="0.15.0"), "You must install scikit-learn version 0.15.0 to obtain correct BDT results"

# Define cut function
def CutFunc(cos_zen):
    return 0.9

def load_pickle(pickle_file):
    try:
        with open(pickle_file, 'rb') as f:
            pickle_data = pickle.load(f)
    except UnicodeDecodeError as e:
        with open(pickle_file, 'rb') as f:
            pickle_data = pickle.load(f, encoding='latin1')
    except Exception as e:
        print('Unable to load data ', pickle_file, ':', e)
        raise
    return pickle_data


class Scorer(icetray.I3ConditionalModule):
    def __init__(self, ctx):
        icetray.I3ConditionalModule.__init__(self, ctx)
        self.AddOutBox("OutBox")
        self.AddParameter("CutFunc", "value for score cut")
        self.AddParameter("CascCut", "value for score cut")
        self.AddParameter("ClfPath", "path to the classifier",
                          os.path.expandvars("$I3_BUILD")+"/finallevel_filter_diffusenumu/resources/bdt/FULL_MAX_FEATURES_sqrt_MAX_DEPTH_None_MIN_SAMPLES_SPLIT_30000_MIN_SAMPLES_LEAF_15000_N_ESTIMATORS_50_LEARNING_RATE_0.05_PurePicklePy2.pkl")
        self.AddParameter("CascClfPath", "path to the cascade classifier",
                          os.path.expandvars("$I3_BUILD")+"/finallevel_filter_diffusenumu/resources/bdt/FULL_NUE_MAX_FEATURES_sqrt_MAX_DEPTH_None_MIN_SAMPLES_SPLIT_30000_MIN_SAMPLES_LEAF_15000_N_ESTIMATORS_50_LEARNING_RATE_0.05_PurePicklePy2.pkl")
        self.clf = None
        self.clfCasc = None


    def Configure(self):
        self.cut_func = self.GetParameter("CutFunc")
        self.casc_cut = self.GetParameter("CascCut")
        self.clf_name = self.GetParameter("ClfPath")
        self.clfCasc_name = self.GetParameter("CascClfPath")

        self.clf = load_pickle(self.clf_name)["clf"]
        self.clfCasc = load_pickle(self.clfCasc_name)["clf"]

    def Physics(self, frame):
        lempty = frame["SplineMPEICCharacteristicsIC"].empty_hits_track_length
        split_geo2 = math.cos(frame["SPEFit2GeoSplit2"].dir.zenith)
        if lempty>400. or split_geo2>0.2:
            return False

        features = {"cog_rho"     : math.sqrt(frame["HitStatisticsValuesIC"].cog.x**2. + frame["HitStatisticsValuesIC"].cog.y**2.),
            "cog_z"               : frame["HitStatisticsValuesIC"].cog.z,
            "lseparation"         : frame["SplineMPEICCharacteristicsIC"].track_hits_separation_length,
            "nch"                 : frame["HitMultiplicityValuesIC"].n_hit_doms,
            "bayes_llh_diff"      : frame["SPEFit2BayesianICFitParams"].logl-frame["SPEFit2ICFitParams"].logl,
            "cos_zenith"          : math.cos(frame["SplineMPEIC"].dir.zenith),
            "rlogl"               : frame["SplineMPEICFitParams"].rlogl,
            "ldir_c"              : frame["SplineMPEICDirectHitsICC"].dir_track_length,
            "ndir_c"              : frame["SplineMPEICDirectHitsICC"].n_dir_doms,
            "sigma_paraboloid"    : math.sqrt(frame["MPEFitParaboloidFitParams"].pbfErr1**2. + frame["MPEFitParaboloidFitParams"].pbfErr2**2.)/math.sqrt(2.) / I3Units.degree,
            "sdir_e"              : frame["SplineMPEICDirectHitsICE"].dir_track_hit_distribution_smoothness}
        feature_names = [f for f in sorted(features.keys())]

        X = []
        for feature in feature_names:
            X.append(features[feature])

        if not np.isfinite(X).all():
            return False

        score = self.clf.predict_proba(X)[:, 1][0]

        Xcasc = []
        for feature in sorted(["cog_z", "cog_rho", "ldir_c", "cos_zenith", "lseparation", "ndir_c", "rlogl", "sigma_paraboloid"]):
            Xcasc.append(features[feature])

        if not np.isfinite(Xcasc).all():
            return False

        cascscore = self.clfCasc.predict_proba(Xcasc)[:, 1][0]

        if score>self.cut_func(features["cos_zenith"]) and cascscore>self.casc_cut:
            for feature in features.keys():
                frame.Put("L5_"+feature, dataclasses.I3Double(features[feature]))
            frame.Put("Score", dataclasses.I3Double(score))
            frame.Put("CascScore", dataclasses.I3Double(cascscore))
            self.PushFrame(frame)
            return True
        else:
            return False

@icetray.traysegment
def millipede_segment(tray, name, table_paths):

    # millipede - new year different pain
    cascade_service_mie = photonics_service.I3PhotoSplineService(table_paths["cascadeampsplinepath"],
                                                                 table_paths["cascadeprobsplinepath"],
                                                                 0)

    exclusionsHE = ["DeepCoreDOMs", "BrightDOMs",
                    "SaturationWindows", "BadDomsList",
                    "CalibrationErrata", "MillipedeHVSplitPulsesExcludedTimeRange"]

    tray.AddModule("MuMillipede", "millipede_highenergy_mie_level5",
        MuonPhotonicsService=None,
        CascadePhotonicsService=cascade_service_mie,
        BinSigma=3,
        MuonSpacing=0,
        ShowerSpacing=10,
        ShowerRegularization=1e-9,
        MuonRegularization=0,
        SeedTrack="SplineMPEIC",
        Output="SplineMPEIC_MillipedeHighEnergyMIE",
        ReadoutWindow="MillipedeHVSplitPulsesReadoutWindow",
        ExcludedDOMs=exclusionsHE,
        DOMEfficiency=0.99,
        Pulses="MillipedeHVSplitPulses")

@icetray.traysegment
def paraboloid_segment(tray, name, table_paths, pulses="TWSRTHVInIcePulsesIC"):
    # Seed service for Paraboloid
    tray.AddService("I3BasicSeedServiceFactory", name+"ParaboloidSeed",
            InputReadout=pulses,
            FirstGuesses=["SplineMPEIC"],
            TimeShiftType="TNone",
            PositionShiftType="None")

    EnEstis = ["SplineMPETruncatedEnergy_SPICEMie_AllDOMS_Muon",
               "SplineMPETruncatedEnergy_SPICEMie_DOMS_Muon",
               "SplineMPETruncatedEnergy_SPICEMie_AllBINS_Muon",
               "SplineMPETruncatedEnergy_SPICEMie_BINS_Muon",
               "SplineMPETruncatedEnergy_SPICEMie_ORIG_Muon"]
    PreJitter = 2

    BareMuTimingSpline = table_paths["infmuonprobsplinepath"]
    BareMuAmplitudeSpline = table_paths["infmuonampsplinepath"]
    StochTimingSpline = table_paths["stochprobsplinepath"]
    StochAmplitudeSpline = table_paths["stochampsplinepath"]

    # python2 python3 compatible version of removing all punctuation
    def str_remove(instring, removestr):
        return ''.join(s for s in instring if s not in removestr)

    BareMuSplineName = "BareMuSplineJitter" + str(PreJitter) +\
            str_remove(BareMuAmplitudeSpline, string.punctuation) +\
            str_remove(BareMuTimingSpline, string.punctuation)
    StochSplineName = "StochMuSplineJitter" + str(PreJitter) +\
            str_remove(StochAmplitudeSpline, string.punctuation) +\
            str_remove(StochTimingSpline, string.punctuation)
    NoiseSplineName = "NoiseSpline" + \
            str_remove(BareMuAmplitudeSpline, string.punctuation)+\
            str_remove(BareMuTimingSpline, string.punctuation) 

    ExistingServices = tray.context.keys()
    # bare muon spline
    if not BareMuSplineName in ExistingServices:
        tray.context[BareMuSplineName] = I3PhotoSplineService(BareMuAmplitudeSpline,
            BareMuTimingSpline,
            PreJitter)
    # stochasics spline
    if not StochSplineName in ExistingServices:
        tray.context[StochSplineName] = I3PhotoSplineService(StochAmplitudeSpline,
        StochTimingSpline,
        PreJitter)
            # noise spline 
    if not NoiseSplineName in ExistingServices:
        tray.context[NoiseSplineName] = I3PhotoSplineService(BareMuAmplitudeSpline,
        BareMuTimingSpline,
        1000)
    tray.AddService("I3GulliverMinuitFactory", name+"Minuit",
          Algorithm =                    "SIMPLEX",                # Default
          FlatnessCheck =                True,                     # Default
          MaxIterations =                1000,                     # ! Only need 1000 iterations
          MinuitPrintLevel =             -2,                       # Default
          MinuitStrategy =               2,                        # Default
          Tolerance =                    0.01                      # ! Set tolerance to 0.01
          )

    tray.AddService("I3SplineRecoLikelihoodFactory", name+"SplineMPEllh",
               PhotonicsService = BareMuSplineName,
               PhotonicsServiceStochastics = StochSplineName,
               PhotonicsServiceRandomNoise = NoiseSplineName,
               ModelStochastics = False,
               NoiseModel = "SRT",
               Pulses = pulses,
               E_Estimators=EnEstis,
               Likelihood = "MPE",
               NoiseRate = 10*I3Units.hertz,
               PreJitter = 0,
               PostJitter = 2,
               KSConfidenceLevel = 5,
               ChargeCalcStep = 0,
               CutMode = "late",
               EnergyDependentJitter = True,
               EnergyDependentMPE = True)

    tray.AddModule("I3ParaboloidFitter", "SplineMPEICParaboloid",
            SeedService                 = name+"ParaboloidSeed",
            LogLikelihood               = name+"SplineMPEllh",
            MaxMissingGridPoints        = 1,
            VertexStepSize              = 5.*I3Units.m,
            ZenithReach                 = 0.5*I3Units.deg,
            AzimuthReach                = 0.5*I3Units.deg,
            GridPointVertexCorrection   = name+"ParaboloidSeed",
            Minimizer                   = name+"Minuit",
            NumberOfSamplingPoints      = 8,
            NumberOfSteps               = 3,
            OutputName                  = "SplineMPEICParaboloid")
