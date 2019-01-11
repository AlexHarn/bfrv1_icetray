from icecube import icetray, dataclasses, dataio
from icecube import photonics_service, gulliver, lilliput
from icecube import linefit, paraboloid
from icecube import cramer_rao, spline_reco, simclasses, shield
from icecube.icetray import I3Units
import icecube.lilliput.segments
import math, os, numpy

from icecube.millipede import HighEnergyExclusions

from I3Tray import *

load("libmue")
load("libbayesian-priors")
load("libtruncated_energy")
load("libDomTools")
load("libdouble-muon")
load("libmillipede")

@icetray.traysegment
def Truncated(tray, Name, Pulses="", Seed="", Suffix="", If=lambda f: True,
    PhotonicsService="", Model=""):

    TruncatedName = Seed+"TruncatedEnergy"+Suffix+Model # ! base result Name to put into frame
    tray.AddModule("I3TruncatedEnergy",
        RecoPulsesName = Pulses, # ! Name of Pulses
        RecoParticleName = Seed,
        ResultParticleName = TruncatedName, # ! Name of result Particle
        I3PhotonicsServiceName = PhotonicsService,  # ! Name of photonics service to use
        UseRDE = True, # ! Correct for HQE DOMs
        If = If )

# high-noise MPE
@icetray.traysegment
def HighNoiseMPE(tray, Name, Pulses="", Seed="", If=lambda f: True, Suffix="", noiserate=10):
    tray.AddService("I3BasicSeedServiceFactory", "HighNoiseMPESeed"+Suffix,
        InputReadout=Pulses,
        FirstGuesses=[Seed],
        TimeShiftType="TFirst")

    tray.AddService("I3GulliverIPDFPandelFactory", "HighNoiseMPEPandel"+Suffix,
        InputReadout=Pulses,
        EventType="InfiniteMuon",
        Likelihood="MPE",
        PEProb="GaussConvolutedFastApproximation",
        JitterTime=4.0 * I3Units.ns,
        NoiseProbability=noiserate * I3Units.hertz)

    tray.AddModule("I3SimpleFitter", "MPEFitHighNoise"+Suffix,
        SeedService="HighNoiseMPESeed"+Suffix,
        Parametrization="SimpleTrack",
        LogLikelihood="HighNoiseMPEPandel"+Suffix,
        Minimizer="Minuit",
        OutputName="MPEFitHighNoise"+Suffix,
        If=If)

#paraboloid
@icetray.traysegment
def Paraboloid(tray, Name, Pulses="", Seed="", If=lambda f: True, Suffix="", noiserate=10):
    tray.AddService("I3BasicSeedServiceFactory", "ParaboloidSeed"+Suffix,
        InputReadout=Pulses,
        FirstGuesses=[Seed],
        TimeShiftType="TFirst",
        NChEnergyGuessPolynomial=[ 0.9789139, 1.173308, 0.3895591])

    tray.AddService("I3GulliverIPDFPandelFactory", "MPEParaboloidPandel"+Suffix,
        InputReadout=Pulses,
        EventType="InfiniteMuon",
        Likelihood="MPE",
        PEProb="GaussConvolutedFastApproximation",
        JitterTime=4.0 * I3Units.ns,
        NoiseProbability=noiserate * I3Units.hertz)

    tray.AddModule("I3ParaboloidFitter", "MPEFitParaboloid"+Suffix,
        SeedService="ParaboloidSeed"+Suffix,
        LogLikelihood="MPEParaboloidPandel"+Suffix,
        MaxMissingGridPoints=1,
        VertexStepSize=5.0 * I3Units.m,
        ZenithReach=2.0 * I3Units.degree,
        AzimuthReach=2.0 * I3Units.degree,
        GridpointVertexCorrection="ParaboloidSeed"+Suffix,
        Minimizer="Minuit",
        NumberOfSamplingPoints=8,
        NumberOfSteps=3,
        MCTruthName="",
        OutputName="MPEFitParaboloid"+Suffix,
        If = If)

@icetray.traysegment
def DoBayesianFit(tray, Name, Pulses, Seed, NIterations=4, noiserate=10, Suffix="", If=lambda frame: True):
    tray.AddService("I3BasicSeedServiceFactory", "BayesSeed"+Suffix,
        InputReadout=Pulses,
        FirstGuesses=[Seed],
        TimeShiftType="TFirst")

    tray.AddService("I3GulliverIPDFPandelFactory", "BayesSPEPandel"+Suffix,
        InputReadout=Pulses,
        Likelihood="SPE1st",
        PEProb="GaussConvoluted",
        NoiseProbability=noiserate* I3Units.hertz,
        JitterTime=15.0 * I3Units.ns)

    tray.AddService("I3EventLogLikelihoodCombinerFactory", "BayesZenithWeightPandel"+Suffix,
        InputLogLikelihoods=["BayesSPEPandel"+Suffix,"ZenithWeight"],
        Multiplicity="Max", # Default
        RelativeWeights=[]) # Default

    tray.AddModule("I3IterativeFitter","SPEFit"+Suffix+"Bayesian",
        RandomService="SOBOL",
        NIterations=NIterations,
        SeedService="BayesSeed"+Suffix,
        Parametrization="SimpleTrack",
        LogLikelihood="BayesZenithWeightPandel"+Suffix,
        CosZenithRange=[0, 1], # ! This is a downgoing hypothesis
        Minimizer="Minuit",
        OutputName="SPEFit"+Suffix+"Bayesian",
        If=If)

@icetray.traysegment
def SplitRecos(tray, Name, Pulses, NIterations=2, Suffix="", If = lambda frame: True):
    tray.AddSegment(linefit.simple, "Linefit"+Suffix,\
        inputResponse=Pulses, fitName="LineFit"+Suffix, If=If)
    tray.AddSegment(lilliput.segments.I3SinglePandelFitter, "SPEFitSingle"+Suffix,\
        fitname="SPEFitSingle"+Suffix, domllh="SPE1st", Pulses=Pulses, Seeds=["LineFit"+Suffix], If=If)
    tray.AddSegment(lilliput.segments.I3IterativePandelFitter, "SPEFit%i"%(NIterations)+Suffix,\
        fitname="SPEFit%i"%(NIterations)+Suffix, Pulses=Pulses, Seeds=["SPEFitSingle"+Suffix], n_iterations=NIterations, If=If)
    tray.AddSegment(DoBayesianFit, "bayes"+Suffix, Pulses=Pulses, Seed="SPEFit%i" % (NIterations)+Suffix,\
        NIterations=NIterations, noiserate=10, Suffix=str(NIterations)+Suffix, If=If)

@icetray.traysegment
def DoSplitFits(tray, Name, Pulses, NIterations=4, Suffix="", If=lambda frame: True):
    # split time
    tray.AddModule( "I3ResponseMapSplitter", "SplitTime"+Suffix,
        InputPulseMap=Pulses, # ! Use timewindow cleaned Pulses
        InputTrackName="",    # Default
        MinimumNch=2,         # Default
        DoTRes=False,         # Default
        MinTRes=0.0,          # Default
        MaxTRes=1000.0,       # Default
        DoBrightSt=False,     # Default
        MaxDBrightSt=150.0,   # Default
        If=If)

    # split geo
    tray.AddModule( "I3ResponseMapSplitter", "SplitGeo"+Suffix,
        InputPulseMap=Pulses,       # ! Use timewindow cleaned Pulses
        InputTrackName="BestTrack", # Default
        MinimumNch=2,               # Default
        DoTRes=False,               # Default
        MinTRes=0.0,                # Default
        MaxTRes=1000.0,             # Default
        DoBrightSt=False,           # Default
        MaxDBrightSt=150.0,         # Default
        If=If)

    # do time and geo split fits
    for num in [1,2]:
        for type in ["Time","Geo"]:
            tray.AddModule( "I3TimeWindowCleaning<I3RecoPulse>", "TimeWindow"+type+Suffix+str(num),
                InputResponse="Split"+type+Suffix+str(num),
                OutputResponse="TWCleanSplit"+type+Suffix+str(num),
                TimeWindow=6000 * I3Units.ns,
                If=If)

            tray.AddSegment(SplitRecos,"splits%s_nits%i_num%i" % (type,NIterations,num),
                Pulses="TWCleanSplit"+type+Suffix+str(num),
                NIterations=NIterations,
                Suffix=type+"Split"+Suffix+str(num),
                If=If)

@icetray.traysegment
def DoSplineReco(tray, Name, Pulses, Seed, LLH, Suffix, spline, If=lambda frame: True):
        tray.AddService("I3BasicSeedServiceFactory", "SplineSeed%s"%(LLH)+Suffix,
            FirstGuesses=[Seed])

        tray.AddService("I3SplineRecoLikelihoodFactory","LLHSpline%s"%(LLH)+Suffix,
            PhotonicsService=spline,
            Pulses=Pulses,
            Likelihood=LLH,
            NoiseRate=10*I3Units.hertz)

        tray.AddModule("I3SimpleFitter", "Spline%s"%(LLH)+Suffix,
            SeedService="SplineSeed%s"%(LLH)+Suffix,
            Parametrization="SimpleTrack",
            LogLikelihood="LLHSpline%s"%(LLH)+Suffix,
            Minimizer="Minuit",
            OutputName="Spline%s"%(LLH)+Suffix,
            If=If)

@icetray.traysegment
def CalculateShieldVars(tray, name):
    SplineMPEShieldEvents = lambda frame: frame.Has("SplineMPE") and frame["SplineMPE"].fit_status==dataclasses.I3Particle.FitStatus.OK and frame["SplineMPE"].dir.zenith < 85.*I3Units.degree
    BestTrackShieldEvents = lambda frame: frame.Has("BestTrack") and frame["BestTrack"].dir.zenith < 85.*I3Units.degree

    tray.AddModule("I3ShieldDataCollector", "SplineMPEShieldHLC",
        InputRecoPulses="OfflineIceTopHLCVEMPulses",
        InputTrack="SplineMPE",
        OutputName="SplineMPEShieldHLC",
        If=SplineMPEShieldEvents)

    tray.AddModule("I3ShieldDataCollector", "SplineMPEShieldSLC",
        InputRecoPulses="OfflineIceTopSLCVEMPulses",
        InputTrack="SplineMPE",
        OutputName="SplineMPEShieldSLC",
        If=SplineMPEShieldEvents)

    tray.AddModule("I3ShieldDataCollector", "ShieldHLC",
        InputRecoPulses="OfflineIceTopHLCVEMPulses",
        InputTrack="BestTrack",
        OutputName="BestTrackShieldHLC",
        If=BestTrackShieldEvents)

    tray.AddModule("I3ShieldDataCollector", "ShieldSLC",
        InputRecoPulses="OfflineIceTopSLCVEMPulses",
        InputTrack="BestTrack",
        OutputName="BestTrackShieldSLC",
        If=BestTrackShieldEvents)

    def CountVetoHits(frame, Shield_HLC, Shield_SLC, tmin, tmax, OutputName):
        VetoHitsHLC = 0
        VetoHitsSLC = 0
        if not frame.Has(Shield_HLC):
            frame.Put(OutputName, icetray.I3Int(-1))
        else:
            HLC = frame[Shield_HLC]
            SLC = frame[Shield_SLC]
            for item in SLC:
                if tmin < item.time_residual < tmax:
                    VetoHitsSLC+=1
            frame.Put(OutputName+"SLC", icetray.I3Int(VetoHitsSLC))
            for item in HLC:
                if tmin < item.time_residual < tmax:
                    VetoHitsHLC+=1
            frame.Put(OutputName+"HLC", icetray.I3Int(VetoHitsHLC))

    tray.AddModule(CountVetoHits, "count_em_on_splinempe",
            Shield_HLC='SplineMPEShieldHLC',
            Shield_SLC='SplineMPEShieldSLC',
            tmin=-50,
            tmax=500,
            OutputName='SplineMPEShieldNHitsOnTime',
            If=SplineMPEShieldEvents)

    tray.AddModule(CountVetoHits, "count_em_off_splinempe",
            Shield_HLC='SplineMPEShieldHLC',
            Shield_SLC='SplineMPEShieldSLC',
            tmin=-1550,
            tmax=-1000,
            OutputName='SplineMPEShieldNHitsOffTime',
            If=SplineMPEShieldEvents)

    tray.AddModule(CountVetoHits, "count_em_on_besttrack",
            Shield_HLC='BestTrackShieldHLC',
            Shield_SLC='BestTrackShieldSLC',
            tmin=-50,
            tmax=500,
            OutputName='BestTrackShieldNHitsOnTime',
            If=BestTrackShieldEvents)

    tray.AddModule(CountVetoHits, "count_em_off_besttrack",
            Shield_HLC='BestTrackShieldHLC',
            Shield_SLC='BestTrackShieldSLC',
            tmin=-1550,
            tmax=-1000,
            OutputName='BestTrackShieldNHitsOffTime',
            If=BestTrackShieldEvents)

@icetray.traysegment
def DoReconstructions(tray, Name, Pulses, Suffix,photonicsdir,photonicsdriverdir,photonicsdriverfile,infmuonampsplinepath,infmuonprobsplinepath,cascadeampsplinepath,cascadeprobsplinepath):

    # setup minuit, parametrization, and bayesian priors for general use
    tray.AddService("I3GulliverMinuitFactory", "Minuit",
        Algorithm="SIMPLEX",
        MaxIterations=1000,
        Tolerance=0.01)

    tray.AddService("I3SimpleParametrizationFactory", "SimpleTrack",
        StepX = 20*I3Units.m,
        StepY = 20*I3Units.m,
        StepZ = 20*I3Units.m,
        StepZenith = 0.1*I3Units.radian,
        StepAzimuth= 0.2*I3Units.radian,
        BoundsX = [-2000*I3Units.m, 2000*I3Units.m],
        BoundsY = [-2000*I3Units.m, 2000*I3Units.m],
        BoundsZ = [-2000*I3Units.m, 2000*I3Units.m])

    tray.AddService( "I3PowExpZenithWeightServiceFactory", "ZenithWeight",
        Amplitude=2.49655e-07,               # Default
        CosZenithRange=[ -1, 1 ],            # Default
        DefaultWeight=1.383896526736738e-87, # Default
        ExponentFactor=0.778393,             # Default
        FlipTrack=False,                     # Default
        PenaltySlope=-1000,                  # ! Add penalty for being in the wrong region
        PenaltyValue=-200,                   # Default
        Power=1.67721)                       # Default

    #####################
    # fits for all events
    #####################

    # muex - iterative angular
    tray.AddModule("muex", "muex_angular4",
        Pulses=Pulses,
        rectrk="",
        result="MuEXAngular4",
        lcspan=0,
        repeat=4,
        usempe=True,
        detail=False,
        energy=False,
        icedir=os.path.expandvars("$I3_BUILD/mue/resources/ice/mie"))

    # cramer-rao
    tray.AddModule("CramerRao",
        InputResponse = Pulses,
        InputTrack = "BestTrack",
        OutputResult = "BestTrackCramerRao",
        AllHits = True,                      # Default
        DoubleOutput = False,                # ! Default
        z_dependent_scatter = True)          # Default

    # high-noise MPE
    tray.AddSegment(HighNoiseMPE, "HighNoiseMPE",
        Pulses=Pulses,
        Seed="BestTrack",
        Suffix="",
        noiserate=100000)

    # paraboloid
    tray.AddSegment(Paraboloid, "paraboloid",
        Pulses=Pulses,
        Seed="BestTrack",
        Suffix="",
        noiserate=10)

    # spline MPE
    spline_mie=photonics_service.I3PhotoSplineService(infmuonampsplinepath,infmuonprobsplinepath, 4)
    llh="MPE"
    tray.AddSegment(DoSplineReco,"spline%s"%(llh),
        Pulses=Pulses,
        Seed="MuEXAngular4",
        LLH=llh,
        Suffix="",
        spline=spline_mie)

    # cramer-rao
    tray.AddModule("CramerRao",
        InputResponse = Pulses,
        InputTrack = "SplineMPE",
        OutputResult = "SplineMPECramerRao",
        AllHits = True,             # Default
        DoubleOutput = False,       # ! Default
        z_dependent_scatter = True) # Default

    # muex - differential energy
    tray.AddModule("muex", "muex_differential",
        Pulses = Pulses,
        rectrk = "SplineMPE",
        result = "SplineMPEMuEXDifferential",
        detail = True, # differential
        energy = True,
        lcspan = 0,
        icedir=os.path.expandvars("$I3_BUILD/mue/resources/ice/mie"))

    # truncated
    PhotonicsTabledirMu_SpiceMie = photonicsdir
    PhotonicsDriverdirMu_SpiceMie = photonicsdriverdir
    PhotonicsDriverfileMu_SpiceMie = photonicsdriverfile

    tray.AddService( "I3PhotonicsServiceFactory", "PhotonicsServiceMu_SpiceMie",
        PhotonicsTopLevelDirectory=PhotonicsTabledirMu_SpiceMie,
        DriverFileDirectory=PhotonicsDriverdirMu_SpiceMie,
        PhotonicsLevel2DriverFile=PhotonicsDriverfileMu_SpiceMie,
        PhotonicsTableSelection=2,
        ServiceName="PhotonicsServiceMu_SpiceMie")

    tray.AddSegment(Truncated,
        Pulses=Pulses,
        Seed="SplineMPE",
        Suffix="",
        PhotonicsService="PhotonicsServiceMu_SpiceMie",
        Model="_SPICEMie")

    # millipede - new year different pain
    cascade_service_mie = photonics_service.I3PhotoSplineService(cascadeampsplinepath,cascadeprobsplinepath,0)

    exclusionsHE=tray.AddSegment(HighEnergyExclusions, "excludes_high_energies",
        Pulses="Millipede"+Suffix+"SplitPulses",
        ExcludeDeepCore="DeepCoreDOMs",
        ExcludeSaturatedDOMs=False,
        ExcludeBrightDOMS="BrightDOMs",
        BrightDOMThreshold=10,
        SaturationWindows="SaturationWindows",
        BadDomsList="BadDomsList",
        CalibrationErrata="CalibrationErrata")
    exclusionsHE.append("Millipede"+Suffix+"SplitPulsesExcludedTimeRange")

    tray.AddModule("MuMillipede", "millipede_highenergy_mie",
        MuonPhotonicsService=None,
        CascadePhotonicsService=cascade_service_mie,
        PhotonsPerBin=15,
        MuonSpacing=0,
        ShowerSpacing=10,
        ShowerRegularization=1e-9,
        MuonRegularization=0,
        SeedTrack="SplineMPE",
        Output="SplineMPE_MillipedeHighEnergyMIE",
        ReadoutWindow="Millipede"+Suffix+"SplitPulsesReadoutWindow",
        ExcludedDOMs=exclusionsHE,
        DOMEfficiency=0.99,
        Pulses="Millipede"+Suffix+"SplitPulses")

    # fits for upgoing events
    UpgoingEvents = lambda frame: (frame.Has("BestTrack") and frame["BestTrack"].dir.zenith > 78.5*I3Units.degree) or (frame.Has("SplineMPE") and frame["SplineMPE"].dir.zenith > 78.5*I3Units.degree)

    NIts=2
    # bayesian fit
    tray.AddSegment(DoBayesianFit,"bayes%i" % (NIts),
        Pulses=Pulses,
        Seed="BestTrack",
        NIterations=NIts,
        noiserate=10,
        Suffix=str(NIts),
        If=UpgoingEvents)

    # time and geo split fits
    tray.AddSegment(DoSplitFits,"splits%i" % (NIts),
        Pulses=Pulses,
        NIterations=NIts,
        If=UpgoingEvents)

    # calculating icetop veto variables
    tray.AddSegment(CalculateShieldVars, "calculate_shield_variables")
