#!/usr/bin/env python

import unittest
import numpy as np

from I3Tray import *
import icecube
from icecube import icetray, dataio, dataclasses, gulliver
from icecube.icetray.i3logging import *

# icetray.i3logging.set_level(icetray.i3logging.I3LogLevel.LOG_DEBUG)
 

#Class to put a particle and loglikelihoodfit pars into the frame
class AddInfoToTray(icetray.I3Module):
  def __init__(self,ctx):
    icetray.I3Module.__init__(self,ctx)
    self.particleName = "TheParticle"
    self.AddParameter("ParticleName", "Name of the particle", self.particleName)

    self.status = "OK"
    self.AddParameter("Status", "Fit status of the particle", self.status)

    self.llhPars = "Pars"
    self.AddParameter("Pars", "Name of the llh fit pars", self.llhPars)

    self.val = 0
    self.AddParameter("Val", "Value of the llh", self.val)
 
  def Configure(self):
    self.particleName = self.GetParameter("ParticleName")
    self.status = self.GetParameter("Status")
    self.llhPars = self.GetParameter("Pars")
    self.val = self.GetParameter("Val")

  def Physics(self, frame):
    particle = dataclasses.I3Particle()
    if self.status == "OK":
      particle.fit_status = dataclasses.I3Particle.FitStatus.OK
    else:
      particle.fit_status = dataclasses.I3Particle.FitStatus.FailedToConverge
    frame[self.particleName] = particle

    llh = gulliver.I3LogLikelihoodFitParams()
    llh.rlogl = self.val
    frame[self.llhPars] = llh

    self.PushFrame(frame)

#Checks to ensure that the event was correctly passed/cut
class FilterCheck(icetray.I3Module, unittest.TestCase):
  
  def __init__(self,ctx):
    icetray.I3Module.__init__(self,ctx)
    self.expectedStatus = True
    self.AddParameter("ExpectedStatus", "If true will fail in finish", self.expectedStatus)

  def Configure(self):
    self.expectedStatus = self.GetParameter("ExpectedStatus")

  def Physics(self, frame):
    fromFrame = frame["I3RLogLFilter"]
    self.assertTrue(fromFrame == self.expectedStatus)
    self.PushFrame(frame)

#Wrapper for the tray execution
def RunTest(name="ParticleName", 
            llh="ParticleNameFitParams", 
            status="OK", 
            val=0, 
            expectPass=False, 
            maxlog=2):

    tray = I3Tray()

    #Make a dummy PFrame
    tray.AddModule("BottomlessSource", "BS", Stream=icetray.I3Frame.Physics)
    tray.AddModule(AddInfoToTray, "AddInfoToTray",
                   ParticleName=name,
                   Pars=llh,
                   Status=status,
                   val=val,
                   )
    tray.Add('I3IcePickModule<I3RLogLFilter>', "Filter",
                    ParticleKey = "ParticleName",
                    MaxRLogL=maxlog
                    )
    tray.AddModule(FilterCheck, "FilterCheck",
                   ExpectedStatus = expectPass
                  )
    tray.Execute(1)


class Test_LogL_Ratio_Filter(unittest.TestCase):

  #Cut event: does not find particle in frame
  def test_missing_particle(self):
    RunTest(name="BAD")

  #Cut event: does not find llh in frame
  def test_missing_llh(self):
    RunTest(llh="BAD")

  #Cut event: bad recon status
  def test_bad_reco_status(self):
    RunTest(status="BAD")

  #Cut event: llh does not pass criteria
  def test_bad_llh_value(self):
    #Easy cases
    RunTest(val=100, maxlog=1)
    RunTest(val=100, maxlog=0)
    RunTest(val=100, maxlog=-1)
    RunTest(val=0, maxlog=-1)
    RunTest(val=-0.5, maxlog=-1)

    #Edge cases
    RunTest(val=0, maxlog=0)
    RunTest(val=33, maxlog=33)
    RunTest(val=-1, maxlog=-1)

  #Passed event
  def test_good_llh_value(self):
    RunTest(val=100, maxlog=101, expectPass=True)
    RunTest(val=0, maxlog=0.5, expectPass=True)
    RunTest(val=-3.3, maxlog=0.1, expectPass=True)
    RunTest(val=-2.2, maxlog=0, expectPass=True)
    RunTest(val=-4.1, maxlog=-1.6, expectPass=True)

  #Just to make sure that this test itself is actually doing something
  @unittest.expectedFailure 
  def test_the_test(self):
    RunTest(val=2, maxlog=1, expectPass=True)


unittest.main()