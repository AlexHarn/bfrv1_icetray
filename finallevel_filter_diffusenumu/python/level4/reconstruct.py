from icecube import icetray, dataclasses, dataio
from icecube import linefit
from icecube.lilliput.segments import I3SinglePandelFitter, I3IterativePandelFitter
from I3Tray import *

load("libbayesian-priors")

@icetray.traysegment
def BasicRecos(tray, name, If=lambda f: True):
    tray.AddSegment(linefit.simple,
        inputResponse = "TWSRTHVInIcePulsesIC",
        fitName = "LineFitIC",
        If=If)

    tray.AddSegment(I3SinglePandelFitter, "SPEFitSingleIC",
        fitname="SPEFitSingleIC",
        domllh="SPE1st",
        pulses="TWSRTHVInIcePulsesIC",
        seeds=["LineFitIC"],
        If=If)

    tray.AddSegment(I3IterativePandelFitter, "SPEFit2IC",
        fitname="SPEFit2IC",
        pulses="TWSRTHVInIcePulsesIC",
        seeds=["SPEFitSingleIC"],
        n_iterations=2,
        If=If)

    tray.AddSegment(I3SinglePandelFitter, "MPEFitIC",
        fitname="MPEFitIC",
        domllh="MPE",
        pulses="TWSRTHVInIcePulsesIC",
        seeds=["SPEFit2IC"],
        If=If)

load("libtruncated_energy")
load("libmue")

@icetray.traysegment
def EnergyRecos(tray, name, PhotonicsTableDir, PhotonicsDriverDir, PhotonicsDriverFile, If=lambda f: True):
    Model="SPICEMie"
    TruncatedName="SplineMPEICTruncatedEnergy"+Model # ! base result name to put into frame
    MuEXName="SplineMPEICMuEXDifferential"

    tray.Add("I3PhotonicsServiceFactory",
        PhotonicsTopLevelDirectory=PhotonicsTableDir,
        DriverFileDirectory=PhotonicsDriverDir,
        PhotonicsLevel2DriverFile=PhotonicsDriverFile,
        PhotonicsTableSelection=2,
        ServiceName="PhotonicsService"+Model)

    tray.Add("I3TruncatedEnergy",
        RecoPulsesName="TWSRTHVInIcePulsesIC", # ! Name of pulses
        RecoParticleName="SplineMPEIC",
        ResultParticleName=TruncatedName, # ! Name of result Particle
        I3PhotonicsServiceName="PhotonicsService"+Model,  # ! Name of photonics service to use
        UseRDE=True, # ! Correct for HQE DOMs
        If=If)

    # muex - differential energy
    tray.Add("muex",
        pulses="TWSRTHVInIcePulsesIC",
        rectrk="SplineMPEIC",
        result=MuEXName,
        detail=True, # differential
        energy=True,
        lcspan=0,
        icedir=os.path.expandvars("$I3_BUILD/mue/resources/ice/mie"),
        If=If)

@icetray.traysegment
def BayesianRecos(tray, name, If=lambda frame: True):
    # setup minuit, parametrization, and bayesian priors for general use
    tray.Add("I3GulliverMinuitFactory", "BayesMinuit",
        Algorithm="SIMPLEX",
        MaxIterations=1000,
        Tolerance=0.01)

    tray.Add("I3SimpleParametrizationFactory", "BayesSimpleTrack",
            StepX = 20*icetray.I3Units.m,
            StepY = 20*icetray.I3Units.m,
            StepZ = 20*icetray.I3Units.m,
            StepZenith = 0.1*icetray.I3Units.radian,
            StepAzimuth= 0.2*icetray.I3Units.radian,
            BoundsX = [-2000*icetray.I3Units.m, 2000*icetray.I3Units.m],
            BoundsY = [-2000*icetray.I3Units.m, 2000*icetray.I3Units.m],
            BoundsZ = [-2000*icetray.I3Units.m, 2000*icetray.I3Units.m])

    tray.Add("I3PowExpZenithWeightServiceFactory", "BayesZenithWeight",
        Amplitude=2.49655e-07,               # Default
        CosZenithRange=[ -1, 1 ],            # Default
        DefaultWeight=1.383896526736738e-87, # Default
        ExponentFactor=0.778393,             # Default
        FlipTrack=False,                     # Default
        PenaltySlope=-1000,                  # ! Add penalty for being in the wrong region
        PenaltyValue=-200,                   # Default
        Power=1.67721)                       # Default

    tray.AddService("I3BasicSeedServiceFactory", "BayesSeed",
        InputReadout="TWSRTHVInIcePulsesIC",
        FirstGuesses=["SplineMPEIC"],
        TimeShiftType="TFirst")

    tray.AddService("I3GulliverIPDFPandelFactory", "BayesSPEPandel",
        InputReadout="TWSRTHVInIcePulsesIC",
        Likelihood="SPE1st",
        PEProb="GaussConvoluted",
        NoiseProbability=10*icetray.I3Units.hertz,
        JitterTime=15.0*icetray.I3Units.ns)

    tray.AddService("I3EventLogLikelihoodCombinerFactory", "BayesZenithWeightPandel",
        InputLogLikelihoods=["BayesSPEPandel", "BayesZenithWeight"],
        Multiplicity="Max",    # Default
        RelativeWeights=[])    # Default

    tray.Add("I3IterativeFitter","SPEFit2BayesianIC",
        OutputName="SPEFit2BayesianIC",
        RandomService="SOBOL",
        NIterations=2,
        SeedService="BayesSeed",
        Parametrization="BayesSimpleTrack",
        LogLikelihood="BayesZenithWeightPandel",
        CosZenithRange=[ 0, 1 ],                 # ! This is a downgoing hypothesis
        Minimizer="BayesMinuit",
        If=If)
