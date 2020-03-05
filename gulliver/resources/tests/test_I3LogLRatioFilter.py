#!/usr/bin/env python

import unittest
import numpy as np

from I3Tray import *
import icecube
from icecube import icetray, dataio, dataclasses, gulliver
from icecube.icetray.i3logging import *

# icetray.i3logging.set_level(icetray.i3logging.I3LogLevel.LOG_DEBUG)
 

#Class to put the two particles and I3LogLikelihoodFitParams to the gramd
class AddInfoToTray(icetray.I3Module):
  def __init__(self,ctx):
    icetray.I3Module.__init__(self,ctx)
    self.name1 = "Particle1"
    self.AddParameter("Particle1", "Name of the first particle", self.name1)
    self.status1 = "OK"
    self.AddParameter("Status1", "Fit status of the first particle", self.status1)
    self.name2 = "Particle2"
    self.AddParameter("Particle2", "Name of the second particle", self.name2)
    self.status2 = "OK"
    self.AddParameter("Status2", "Fit status of the second particle", self.status2)

    self.llhPars1 = "Pars1"
    self.AddParameter("Pars1", "Name of the first llh fit pars", self.llhPars1)
    self.llhPars2 = "Pars2"
    self.AddParameter("Pars2", "Name of the first llh fit pars", self.llhPars2)

    self.val1 = 0
    self.AddParameter("Val1", "Name of the first llh fit pars", self.val1)
    self.val2 = 0
    self.AddParameter("Val2", "Name of the first llh fit pars", self.val2)
 
  def Configure(self):
    self.name1 = self.GetParameter("Particle1")
    self.name2 = self.GetParameter("Particle2")
    self.status1 = self.GetParameter("Status1")
    self.status2 = self.GetParameter("Status2")
    self.llhPars1 = self.GetParameter("Pars1")
    self.llhPars2 = self.GetParameter("Pars2")
    self.val1 = self.GetParameter("Val1")
    self.val2 = self.GetParameter("Val2")

  def Physics(self, frame):
    particle1 = dataclasses.I3Particle()
    if self.status1 == "OK":
      particle1.fit_status = dataclasses.I3Particle.FitStatus.OK
    else:
      particle1.fit_status = dataclasses.I3Particle.FitStatus.FailedToConverge
    frame[self.name1] = particle1
    particle2 = dataclasses.I3Particle()
    if self.status2 == "OK":
      particle2.fit_status = dataclasses.I3Particle.FitStatus.OK
    else:
      particle2.fit_status = dataclasses.I3Particle.FitStatus.FailedToConverge
    frame[self.name2] = particle2

    llh1 = gulliver.I3LogLikelihoodFitParams()
    llh1.logl = self.val1
    frame[self.llhPars1] = llh1
    llh2 = gulliver.I3LogLikelihoodFitParams()
    llh2.logl = self.val2
    frame[self.llhPars2] = llh2

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
    fromFrame = frame["I3LogLRatioFilter"]
    self.assertTrue(fromFrame == self.expectedStatus)
    self.PushFrame(frame)

#Wrapper for the tray execution
def RunTest(name1="ParticleName1", 
            name2="ParticleName2", 
            llh1="ParticleName1FitParams", 
            llh2="ParticleName2FitParams", 
            status1="OK", 
            status2="OK",
            val1=0, 
            val2=0, 
            expectPass=False, 
            maxlog=2):

    tray = I3Tray()

    #Make a dummy PFrame
    tray.AddModule("BottomlessSource", "BS", Stream=icetray.I3Frame.Physics)
    tray.AddModule(AddInfoToTray, "AddInfoToTray",
                   Particle1=name1,
                   Particle2=name2,
                   Pars1=llh1,
                   Pars2=llh2,
                   Status1=status1,
                   Status2=status2,
                   val1=val1,
                   val2=val2
                   )
    tray.Add('I3IcePickModule<I3LogLRatioFilter>', "Filter",
                    ParticleKey1 = "ParticleName1",
                    ParticleKey2 = "ParticleName2",
                    MaxLogLRatio=maxlog
                    )
    tray.AddModule(FilterCheck, "FilterCheck",
                   ExpectedStatus = expectPass
                  )
    tray.Execute(1)


class Test_LogL_Ratio_Filter(unittest.TestCase):

  #Cut event: does not find particle1 in frame
  def test_missing_particle1(self):
    RunTest(name1="BAD")

  #Cut event: does not find llh1 in frame
  def test_missing_llh1(self):
    RunTest(llh1="BAD")

  #Cut event: bad recon status1
  def test_bad_reco_status1(self):
    RunTest(status1="BAD")

  #Cut event: does not find particle3 in frame
  def test_missing_particle2(self):
    RunTest(name2="BAD")

  #Cut event: does not find llh2 in frame
  def test_missing_llh2(self):
    RunTest(llh2="BAD")

  #Cut event: bad recon status2
  def test_bad_reco_status2(self):
    RunTest(status2="BAD")

  #Cut event: bad recon status
  def test_bad_ratio(self):
    RunTest(val1=0, val2=1, maxlog=0)
    RunTest(val1=1, val2=2, maxlog=0)
    RunTest(val1=1, val2=2, maxlog=-1) #edge case
    RunTest(val1=-1, val2=0, maxlog=-1)
    RunTest(val1=-1, val2=0, maxlog=-1)

  #Pass event: bad recon status
  def test_bad_ratio(self):
    RunTest(val1=2, val2=4, maxlog=1, expectPass=True)
    RunTest(val1=2, val2=4, maxlog=-1, expectPass=True)
    RunTest(val1=2, val2=-1, maxlog=3.01, expectPass=True)
    RunTest(val1=-1.5, val2=2.1, maxlog=-3.5, expectPass=True)
    RunTest(val1=-3.1, val2=-2.1, maxlog=-.5, expectPass=True)
    RunTest(val1=-3.1, val2=-2.1, maxlog=.5, expectPass=True)

  #Just to make sure that this test itself is actually doing something
  @unittest.expectedFailure
  def test_the_test_works(self):
    RunTest(val1=2, val2=0, maxlog=1, expectPass=True)


unittest.main()