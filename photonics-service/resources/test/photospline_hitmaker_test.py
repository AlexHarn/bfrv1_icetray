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

GenerationRadius = 1000.
ZenithMin = 0*I3Units.degree
ZenithMax = 180*I3Units.degree
MuonEnergy = 100 * I3Units.TeV
mu_type =dataclasses.I3Particle.MuPlus

gcdfile = os.path.join(os.environ['I3_TESTDATA'],"GCD",
                       "GeoCalibDetectorStatus_2012.56063_V0.i3.gz")
TablePath = "/cvmfs/icecube.opensciencegrid.org/data/photon-tables/splines"
amplitudetable = TablePath+'/ems_mie_z20_a10.abs.fits'
timingtable = TablePath+'/ems_mie_z20_a10.prob.fits'

if not os.path.exists(TablePath):
	print("tables not found, skipping test")
	sys.exit(0)

# The results of this test depend strongly on the distance of closest approach.
# Use a pregenerated set of tracks that do not change when propagator
# implementations change the number of random values they consume between
# frames.
# x, y, z, zenith, azimuth
tracks = iter([
  [-692.0821595186383, -406.38534323145575, 596.5511187506651, 0.9315993856664951, 3.6725415257078335],
  [-232.53303197281232, -446.0681111472593, -864.2636341181129, 2.6144810183349056, 4.231850942423073],
  [-146.80878516207454, 453.99543509775407, 878.8261065248556, 0.4973999877061872, 1.8835538019500448],
  [-285.88021869068194, 672.1328025102099, -683.0153705087016, 2.322679394700654, 1.9729489407480325],
  [-96.5187098132897, -943.938551612804, -315.69629303350996, 1.891986711746489, 4.610492063971288],
  [-128.66803395211357, -305.83447712067215, 943.3503112011056, 0.3382093537156779, 4.314156463669455],
  [-721.7900165300813, 294.50335624860327, 626.3281449813774, 0.8939622454076169, 2.7541933303834822],
  [-315.4074993652831, 77.92586386574885, -945.7513780507621, 2.810695904987455, 2.8993791117758763],
  [494.10484355278584, -850.4594974912196, -180.49666674113183, 1.7522877150837515, 5.238710069572165],
  [715.9464958299037, 697.9385236933559, 17.390579450396633, 1.5534048706464918, 0.7726623339359069],
])

def muon_injector(frame):
    x,y,z,zenith,azimuth = next(tracks)

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
    assert (500 < l < 15000), "the length of 'residuals' is out of range: l == %i" % l
    m  = np.mean(residuals)
    assert (100 < m < 1000), "the mean of 'residuals' is out of range: m == %i" % m
    s = np.std(residuals)
    assert (100 < s < 1000), "the standard deviation of 'residuals' is out of range: s == %i" % s

randomService = phys_services.I3GSLRandomService(seed = 12345)

cascade_service = photonics_service.I3PhotoSplineService(
        amplitudetable=amplitudetable,
        timingtable=timingtable,
        timingSigma=0.)

tray = I3Tray()

tray.AddModule("I3InfiniteSource", "source",
               prefix = gcdfile)

tray.AddModule(muon_injector,
               Streams=[icetray.I3Frame.DAQ])

tray.AddSegment(PropagateMuons, 
                RandomService = randomService
                )

tray.AddModule("I3MCTreeHybridSimulationSplitter")

tray.AddModule("I3PhotonicsHitMaker", 
               CascadeService = cascade_service,
               TrackService = None, 
               Input = "I3MCTreeCascades",
               RandomService = randomService)

tray.AddModule(test,Streams=[icetray.I3Frame.DAQ])

tray.Execute(13)


