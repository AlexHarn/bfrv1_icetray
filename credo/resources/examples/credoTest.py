import os
import numpy as n
import pylab as p

from icecube import icetray, dataclasses, dataio, credo, gulliver, photonics_service
from I3Tray import *

#
#   Usage: python credoTest.py
#
#   Description:  This script takes a L3 NuGen file that has been run through
#   the cascade filters.  Picks a specific event and calculates the log 
#   likelihood of the event being a cascade
#
#


#   Load photonics
pservice = photonics_service.I3PhotonicsTableService(
    photonicsTopLevelDirectory  = 
        "/data/sim/sim-new/PhotonTablesProduction/AHA07v1ice/",
    driverFileDirectory         = 
        "/data/sim/sim-new/PhotonTablesProduction/AHA07v1ice/driverfiles/",
    level1DriverFile            = "",
    level2DriverFile            = "cscd_photorec.list",
    interpolMode                = 255,
    verbosity                   = 0,
    angularSelectLow            = 0. * I3Units.degree,
    angularSelectHigh           = 180. * I3Units.degree,
    zSelectLow                  = -2000 * I3Units.meter,
    zSelectHigh                 = +2000 * I3Units.meter,
    photonicsTableSelection     = 2,
    isPhotorecTable             = True )

#   Set up Credo LogLikelihood to be calculated using the above chosen 
#   photonics and OfflinePulses
logl = credo.I3PoissonGaussLogLikelihood( "credollh", pservice, 
    "OfflinePulses", 480.0 * I3Units.hertz, -1., 450., 1000., 
    False, False, False, False, 0.85 * 0.99 )


#   Input files
gcdfile = "/data/sim/IceCube/2011/filtered/level2/neutrino-generator/10601/01000-01999/GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz"
infile  = "/data/ana/Cscd/IC86-1/level3a/sim/10601/Level3a_Cscd_IC86.2011_nugen_nue.010601.00390X.i3.bz2"

gfile   = dataio.I3File( gcdfile )
pfile   = dataio.I3File( infile )

#   Geometry, DetectorStatus, and Calibration frames are needed by Credo so
#   grab them and put them in the frame.
geo     = None
stat    = None
calib   = None
frame   = None

while geo == None:
    frame   = gfile.pop_frame( )
    geo     = frame.Get( "I3Geometry" )

while calib == None:
    frame   = gfile.pop_frame( )
    calib   = frame.Get( "I3Calibration" )

while stat == None:
    frame   = gfile.pop_frame( )
    stat    = frame.Get( "I3DetectorStatus" )

#   Create a fake particle with event characteristics copied from the 
#   selected event.
trueparticle = dataclasses.I3Particle( )

trueparticle.type   = dataclasses.I3Particle.NuEBar
trueparticle.pos    = dataclasses.I3Position( -73.9886, -223.496, 39.3778 )
trueparticle.dir    = dataclasses.I3Direction( 26.5364 * I3Units.degree, 
                                               336.068 * I3Units.degree)
trueparticle.energy = 567.568 * I3Units.GeV
trueparticle.time   = 9580.33 * I3Units.ns
trueparticle.shape  = dataclasses.I3Particle.ParticleShape.Cascade 
trueparticle.location_type = dataclasses.I3Particle.InIce

#   Create a hypothetical particle
hypo = gulliver.I3EventHypothesis( trueparticle )

#   This is the actual selected event (6th physics frame)
for i in range( 6 ):
    frame = pfile.pop_physics( )

#   Put various needed GCD frames in this physics frame.
frame.Put( "I3Geometry", geo )
frame.Put( "I3Calibration", calib )
frame.Put( "I3DetectorStatus", stat )

#   Make everything in the frame necessary for the reconstruction.
logl.SetEvent( frame )

#   Create arrays to scan for the location of the cascade
xs = n.linspace(-500,500,100)
ys = n.linspace(-500,500,200)

result = n.zeros((len(ys), len(xs)), dtype=float)

centerx = 200.
centery = 0.
centerz = 0.

#   Get the log likelihood for various x and y positions
for i_x in range(len(xs)):
    print(i_x)
    for i_y in range(len(ys)):
        hypo.particle.pos = dataclasses.I3Position( centerx+xs[i_x], centery+ys[i_y], centerz )
        result[i_y, i_x] = logl.GetLogLikelihood(hypo)
        print "xpos: %6.2f ypos: %6.2f: logl: %6.2f" % (xs[i_x], ys[i_y], result[i_y, i_x])

#   Divide loglikelihood by the multiplicity
result /= logl.GetMultiplicity()

#   Plot
p.contourf(xs, ys, n.log10(-result), 100)
cb = p.colorbar()
cb.set_label("log(+logl/multi)")
p.xlabel("x")
p.ylabel("y")

p.show()

print "llh: ", logl.GetLogLikelihood(hypo)
print "multiplicity: ", logl.GetMultiplicity() 
print "chi2 proxy for llh: ", logl.GetLogLikelihood(hypo)/logl.GetMultiplicity() * (-2)
