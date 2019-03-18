#!/usr/bin/env python

import os
import math
import numpy as np
from I3Tray import *
from icecube import icetray,dataclasses,phys_services,photonics_service
try:
    from icecube.simprod.segments.PropagateMuons import PropagateMuons
except ImportError:
    print("Disable the test if simprod isn't installed, e.g. in IceRec.")
    sys.exit(0)
        

CylinderRadius = 800.
ZenithMin = 90*I3Units.degree
ZenithMax = 92*I3Units.degree
HeightMin =  0*I3Units.m
HeightMax = 40*I3Units.m
MuonEnergy = 100 * I3Units.TeV
mu_type =dataclasses.I3Particle.MuPlus

gcdfile = os.path.join(os.environ['I3_TESTDATA'],"sim",
                       "GeoCalibDetectorStatus_2012.56063_V0.i3.gz")
tablesdir  = os.path.join(os.environ["I3_TESTDATA"],
                          "photonics-tables","SPICEMie_i3coords")
driversdir = os.path.join(tablesdir,"driverfiles")

def muon_injector(frame):
    zenith  = randomService.uniform(ZenithMin,ZenithMax)
    azimuth = randomService.uniform(0,360*I3Units.degree)
    x = CylinderRadius*math.cos(azimuth)
    y = CylinderRadius*math.sin(azimuth)
    z = CylinderRadius*math.cos(zenith) + (HeightMin+HeightMax)/2

    global mu_type
    if mu_type==dataclasses.I3Particle.MuMinus:
        mu_type=dataclasses.I3Particle.MuPlus
    else:
        mu_type = dataclasses.I3Particle.MuMinus
    
    mu        = dataclasses.I3Particle()
    mu.type   = mu_type
    mu.pos    = dataclasses.I3Position(x,y,z)
    mu.dir    = dataclasses.I3Direction(zenith,azimuth)
    mu.energy = MuonEnergy
    mu.time   = 0 * I3Units.ns
    mu.length = NaN
    mu.location_type = dataclasses.I3Particle.InIce

    mctree = dataclasses.I3MCTree()
    mctree.add_primary(mu)
    frame["I3MCTree_preMuonProp"]=mctree

def test(frame):
    residuals = []
    geo = frame['I3Geometry'].omgeo
    primary = dataclasses.I3MCTree.get_primaries(frame['I3MCTree_preMuonProp'])[0]
    for om,pes in frame['I3MCPESeriesMap']:
        pos = geo[om].position
        residuals += [phys_services.I3Calculator.time_residual(primary,pos,p.time) for p in pes]

    l = len(residuals)
    assert (1000 < l < 11000), "the length of 'residuals' is out of range: l == %i" % l
    m  = np.mean(residuals)
    assert (200 < m < 500), "the mean of 'residuals' is out of range: m == %i" % m
    s = np.std(residuals)
    assert (300 < s < 500), "the standard deviation of 'residuals' is out of range: s == %i" % s

randomService = phys_services.I3GSLRandomService(seed = 12345)

tray = I3Tray()

tray.context["I3RandomService"]=randomService

tray.AddModule("I3InfiniteSource", "source",
               prefix = gcdfile)

tray.AddModule(muon_injector,
               Streams=[icetray.I3Frame.DAQ])

tray.AddSegment(PropagateMuons,
                RandomService="I3RandomService")

tray.AddService(
    "I3PhotonicsServiceFactory","photonics",
    PhotonicsTopLevelDirectory = tablesdir,
    DriverFileDirectory = driversdir,
    PhotonicsLevel1DriverFile = "SPICEMie_i3coords_level1_shower.list",
    PhotonicsLevel2DriverFile = "SPICEMie_i3coords_level2_muon.list",
    PhotonicsInterpolationMode = 7,
    PhotonicsAngularSelectionLow = ZenithMin,
    PhotonicsAngularSelectionHigh = ZenithMax,
    PhotonicsZSelectionHigh = 40 * I3Units.m, 
    PhotonicsZSelectionLow  =  0 * I3Units.m, 
    )

tray.AddModule("I3PhotonicsHitMaker")

tray.AddModule(test,Streams=[icetray.I3Frame.DAQ])
    
tray.Execute(13)


