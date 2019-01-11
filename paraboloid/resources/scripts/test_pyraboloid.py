#!/usr/bin/env python

"""
Test script for pyraboloid: define a trivial likelihood function and check
that pyraboloid reconstructs the angular uncertainty correctly.

The fake likelihood function has its maximum fixed at zenith=90deg and azimuth=0,
with a standard deviation of about 1 degree in both directions.
The "reconstruction" lands 0.25 degree away from that (in azimuth).
This test script currently only checks that pyraboloid reconstructs the correlation matrix correctly.

TODO (required for release):
- also check the offset, err1 and err2, rotation angle, etc.
- make this run over arbitrarily many random test cases, random directions
  (let the test generator store the correct values in the frame, the checker module can then use those)
- throw an error in case the computed correlation matrix differs too much from the correct answer

TODO (optional):
- add higher order terms to fake_logL
- store output in HDF5 for performance plots
- aim for pathological cases (vertical direction, very small standard deviation, b=0, etc)
"""

from numpy import sin, log, pi
from icecube.paraboloid.pyraboloid import pyraboloid
from icecube.dataclasses import I3Particle,I3Direction,I3Position
from icecube.gulliver import I3EventLogLikelihood
from icecube.lilliput.segments import add_minuit_simplex_minimizer_service
from icecube.lilliput.scipymin import SciPyMinimizer
from icecube.icetray import logging
from I3Tray import *

# global names
label="angerr"
para_label=label+"FitParams"
test_particle_name="guineapig"
seed_service_name="pyraboloidseed"

# python function, going to be used as an I3ConditionalModule
def add_test_particle(frame):
    p            = I3Particle()
    p.dir        = I3Direction(90*I3Units.degree,0.25*I3Units.degree)  # off by 0.25 degree
    p.pos        = I3Position(9.*I3Units.m,11.*I3Units.m,13.*I3Units.m) # irrelevant for this test, but should be non-NAN
    p.time       = 10.*I3Units.microsecond # irrelevant for this test, but should be non-NAN
    p.fit_status = I3Particle.OK           # irrelevant for this test, but should be OK
    frame.Put(test_particle_name,p)

# Likelihood function based on a dumb 2D gauss (Kent or Von Mises distribution would be better)
# this is going to be used as a gulliver likelihood service
class fake_logL(I3EventLogLikelihood):
    def __init__(self,name):
        super(fake_logL, self).__init__()
        self.name = name
        # righthanded local cartesian frame similar to the one in pyraboloid:
        # dir0 is the reconstructed dir (here true dir)
        # perp1 is dir0 rotated by 90 in zenith
        # perp2 is dir0.cross(p1)
        self.truedir  = I3Direction(90.*I3Units.degree,0.) # along the x-axis
        self.perp1dir = I3Direction(180.*I3Units.degree,0.)
        self.perp2dir = self.truedir.cross(self.perp1dir)
        # self.lognorm = -log(2*pi*self.sigma**2) # todo: normalization constant?
        self.a = 1.25*I3Units.degree**-2
        self.b = 0.5*I3Units.degree**-2
        self.c = 0.75*I3Units.degree**-2
    def SetGeometry(self,geo):
        pass
    def SetEvent(self,frame):
        pass
    def GetName(self):
        return self.name
    def GetMultiplicity(self):
        return 42
    def GetLogLikelihood(self,hypothesis):
        hpos = hypothesis.particle.pos
        p1 = hypothesis.particle.dir * self.perp1dir
        p2 = hypothesis.particle.dir * self.perp2dir
        # note: in a real likelihood function the position and direction are very intertwined.
        f = hpos*hpos + 0.5 * (self.a * p1**2 + 2*self.b * p1 * p2 + self.c * p2**2)
        return -f

def check_pyraboloid(frame):
    global para_label
    if not para_label in frame:
        logging.log_fatal("pyraboloid result (%s) not found in frame" % para_label)
    fitparams = frame[para_label]
    logging.log_notice("PBF status=%d" % fitparams.pbfStatus )
    logging.log_notice("Curv11=%f Curv12=%f Curv22=%f" % (fitparams.pbfCurv11,fitparams.pbfCurv12,fitparams.pbfCurv22) )
    logging.log_notice("compare: a=%f b=%f c=%f" % ( 1.25*I3Units.degree**-2, 0.5*I3Units.degree**-2, 0.75*I3Units.degree**-2) )

#################################################
# done with definitions, now build & setup a tray
#################################################

# instantiate a tray
tray=I3Tray()

# the bottomless source generates infinitely many empty events ("frames")
tray.Add("BottomlessSource")

# we add our test thingy
tray.Add(add_test_particle)

# now we add the gulliver services that pyraboloid will use
# first: a seed service, to let it import the "reconstructed" track/particle
tray.Add("I3BasicSeedServiceFactory",seed_service_name,FirstGuesses=[test_particle_name])

# this makes a gulliver-wrapper object for the scipy implementation of simplex.
scipy_simplex = SciPyMinimizer(name="scipy_simplex", method='Nelder-Mead', tolerance=0.001, max_iterations=1000)

# Pyraboloid stores two objects in the frame
# an I3Particle (using label as key)
# an I3ParaboloidFitParams (using label+"FitParams" as key)
# gulliver services can be python objects or objects created by a C++ factory
tray.AddModule(pyraboloid,label,
        LikelihoodService = fake_logL("fake"), # get service as a python object, created on the spot
        SeedService       = seed_service_name, # get service object from C++ service factory
        MinimizerService  = scipy_simplex,     # get service as a python object, created earlier
        NRings            = 2,                 # paraboloid grid: two rings around input direction
        NPointsPerRing    = 6,                 # paraboloid grid: six grid points per ring
        Radius            = 1*I3Units.degree)  # angular distance between rings (or: radius of inner ring)

# add the testing module
tray.Add(check_pyraboloid)

#################################################
# Now run it! Currently only one useful test case...
#################################################
tray.Execute(1)
