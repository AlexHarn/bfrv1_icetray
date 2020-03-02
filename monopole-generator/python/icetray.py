import os
from icecube import icetray, dataclasses
from icecube.icetray import I3Units
from icecube.simprod.segments import DetectorSim


@icetray.traysegment
def DefaultMonopoleSimulationTray(tray, name,
                                  RunID,
                                  Nevents,
                                  GCDFile,
                                  Mass,
                                  betaRange,
                                  taudnde=None,
                                  Disk_dist=1000. * I3Units.m,
                                  Disk_rad=800. * I3Units.m,
                                  PowerLawIndex=float("NaN"),
                                  StepSize=float("NaN"),
                                  MinLength=1*I3Units.m,
                                  MaxLength=50*I3Units.m,
                                  MaxDistanceFromCenter=1300 * I3Units.m,
                                  ZenithRange=(0.0 * I3Units.degree, 180.0 * I3Units.degree),
                                  AzimuthRange=(0.0 * I3Units.degree, 360.0 * I3Units.degree),
                                  ShiftCenter=(0.0 * I3Units.meter, 0.0 * I3Units.meter, 0.0 * I3Units.meter),
                                  icemodel="$I3_BUILD/ppc/resources/ice/",
                                  If=lambda f: True
                                  ):

    icemodel = os.path.expandvars(icemodel)
    assert os.path.isdir(icemodel), "IceModel does not point to folder"
    os.putenv("PPCTABLESDIR", os.path.expandvars(icemodel))

    tray.AddModule("I3MonopoleGenerator", name + "_generator",
                   Nevents=Nevents,
                   Mass=Mass,
                   BetaRange=betaRange,
                   Disk_dist=Disk_dist,  # meters
                   Disk_rad=Disk_rad,  # meters
                   PowerLawIndex=PowerLawIndex,
                   ZenithRange=ZenithRange,
                   AzimuthRange=AzimuthRange,
                   ShiftCenter=ShiftCenter,
                   If=If
    )

    tray.AddModule("I3MonopolePropagator", name + "_propagator",
               MaxDistanceFromCenter=MaxDistanceFromCenter,
               StepSize=StepSize,
               MinLength=MinLength,
               MaxLength=MaxLength,
               If=If
    )

    tray.AddModule("i3ppc", name + "_ppc",
               gpu=0,
               tau_dnde_vec   = taudnde,
               infoName = "PPCInfoDict",
               If=If
    )

    #unify name for rest of icecube
    tray.Add("Rename", name + "_ppc_rename",
         Keys=["MCPESeriesMap", "I3MCPESeriesMap"]
    )

    tray.Add(DetectorSim, name + "_detectorsim",
         "DetectorTrigger",
         RandomService='I3RandomService',
         GCDFile=GCDFile,
         InputPESeriesMapName='I3MCPESeriesMap',
         SkipNoiseGenerator=False,
         RunId=RunID,
         KeepMCHits=True,#False
         KeepPropagatedMCTree=True,#False
         KeepMCPulses=True,#False
         LowMem=False,
         BeaconLaunches=True,
         TimeShiftSkipKeys=[],
         FilterTrigger=True,
         If=If
    )

