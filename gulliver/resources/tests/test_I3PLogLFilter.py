#!/usr/bin/env python

import unittest
import numpy as np

from I3Tray import *
import icecube
from icecube import icetray, dataio, dataclasses, gulliver, phys_services
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

    self.pulseSeries = "ThePulseSeries"
    self.AddParameter("PulseSeries", "Name of the pulse series", self.pulseSeries)

    self.nPulses = 1
    self.AddParameter("NPulses", "The cut value", self.nPulses)

    self.nChannels = 0
    self.AddParameter("NChannels", "The cut value", self.nChannels)
    
    self.cutValueName = "TheCutValue"
    self.AddParameter("CutValueName", "Name of the cut value", self.cutValueName)

 
  def Configure(self):
    self.particleName = self.GetParameter("ParticleName")
    self.status = self.GetParameter("Status")
    self.llhPars = self.GetParameter("Pars")
    self.val = self.GetParameter("Val")
    self.pulseSeries = self.GetParameter("PulseSeries")
    self.nPulses = self.GetParameter("NPulses")
    self.nChannels = self.GetParameter("NChannels")
    self.cutValueName = self.GetParameter("CutValueName")

  def Physics(self, frame):
    particle = dataclasses.I3Particle()
    if self.status == "OK":
      particle.fit_status = dataclasses.I3Particle.FitStatus.OK
    else:
      particle.fit_status = dataclasses.I3Particle.FitStatus.FailedToConverge
    frame[self.particleName] = particle

    llh = gulliver.I3LogLikelihoodFitParams()
    llh.logl = self.val
    frame[self.llhPars] = llh

    pulseSeries = dataclasses.I3RecoPulseSeriesMap()
    for i in range(self.nPulses):
      key = icetray.OMKey(i,0,0)
      temp = dataclasses.I3RecoPulseSeries()
      pulseSeries[key] = temp
    frame[self.pulseSeries] = pulseSeries

    cutValues = phys_services.I3CutValues()
    cutValues.nchan = self.nChannels
    frame[self.cutValueName] = cutValues


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
    fromFrame = frame["I3PLogLFilter"]
    self.assertTrue(fromFrame == self.expectedStatus)
    self.PushFrame(frame)

#Wrapper for the tray execution
def RunTest(name="ParticleName", 
            llh="ParticleNameFitParams", 
            status="OK", 
            val=0, 
            expectPass=False, 
            maxlog=2,
            pulseSeries="ThePulseSeries",
            cutValueName="TheCutValue",
            NPulses=1,
            NChannels=1):

    tray = I3Tray()

    #Make a dummy PFrame
    tray.AddModule("BottomlessSource", "BS", Stream=icetray.I3Frame.Physics)
    tray.AddModule(AddInfoToTray, "AddInfoToTray",
                   ParticleName=name,
                   Pars=llh,
                   Status=status,
                   val=val,
                   PulseSeries=pulseSeries,
                   NPulses=NPulses,
                   NChannels=NChannels,
                   CutValueName=cutValueName
                   )
    tray.Add('I3IcePickModule<I3PLogLFilter>', "Filter",
                    ParticleKey = "ParticleName",
                    MaxPLogL=maxlog,
                    RecoPulseSeries="ThePulseSeries",
                    CutValuesKey="TheCutValue"
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

  #Cut event: missing the pulse series and cut Values
  def test_missing_pulse_series_and_cutValues(self):
    RunTest(pulseSeries="BAD", cutValueName="BAD2")

  #Test when only one of the two exists
  def test_only_one_exists(self):
    RunTest(pulseSeries="BAD", expectPass=True, NPulses=10, val=0, maxlog=1)
    RunTest(pulseSeries="BAD", expectPass=True, NPulses=0, val=0, maxlog=1)
    RunTest(pulseSeries="BAD", expectPass=False, NChannels=0, val=0, maxlog=1) #div by zero
    RunTest(pulseSeries="BAD", expectPass=True, NChannels=10, val=0, maxlog=1)

    RunTest(cutValueName="BAD", expectPass=False, NPulses=0, val=0, maxlog=1) #div by zero
    RunTest(cutValueName="BAD", expectPass=True, NPulses=10, val=0, maxlog=1)
    RunTest(cutValueName="BAD", expectPass=True, NChannels=0, val=0, maxlog=1)
    RunTest(cutValueName="BAD", expectPass=True, NChannels=10, val=0, maxlog=1)

  def test_llh_values(self):
    RunTest(cutValueName="BAD", expectPass=False, NPulses=3, val=2, maxlog=0.5)
    RunTest(cutValueName="BAD", expectPass=False, NPulses=4, val=2, maxlog=0.5)
    RunTest(cutValueName="BAD", expectPass=True, NPulses=8, val=2, maxlog=0.5)
    RunTest(cutValueName="BAD", expectPass=False, NPulses=8, val=3, maxlog=0.5)
    RunTest(cutValueName="BAD", expectPass=True, NPulses=8, val=2.9, maxlog=0.5)
    RunTest(cutValueName="BAD", expectPass=True, NPulses=8, val=3, maxlog=0.6)

    RunTest(pulseSeries="BAD", expectPass=False, NChannels=3, val=2, maxlog=0.5)
    RunTest(pulseSeries="BAD", expectPass=False, NChannels=4, val=2, maxlog=0.5)
    RunTest(pulseSeries="BAD", expectPass=True, NChannels=8, val=2, maxlog=0.5)
    RunTest(pulseSeries="BAD", expectPass=False, NChannels=8, val=3, maxlog=0.5)
    RunTest(pulseSeries="BAD", expectPass=True, NChannels=8, val=2.9, maxlog=0.5)
    RunTest(pulseSeries="BAD", expectPass=True, NChannels=8, val=3, maxlog=0.6)


unittest.main()