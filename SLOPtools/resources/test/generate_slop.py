#!/usr/bin/env python

# $Id: generate_slop.py 136765 2015-08-24 08:15:15Z jacobi $

from icecube import icetray, dataclasses

import numpy as np
#np.random.seed(42)

n_strings = 86
n_doms = 60


class sloppozela(icetray.I3Module):
  """
  Creates simplified noise hits for the purpose of testing the SLOPtools.
  By default each event is 1ms long and the number of hits per OM is drawn
  from a poisson distribution with lambda=0.5
  The times of the hit in an OM are drawn from the continuous uniform
  random distribution along the event length.
  """
  def __init__(self, context):
    icetray.I3Module.__init__(self,context)
    self.AddParameter("lambda_poisson", "lambda_poisson", 0.5)
    self.AddParameter("event_length", "event_length", 1e6*icetray.I3Units.nanosecond)
    self.AddOutBox('OutBox')
    
  def Configure(self):
    self.lambda_poisson=self.GetParameter("lambda_poisson")
    self.event_length=self.GetParameter("event_length")
    
  def Physics(self, frame):
    pulse_map = dataclasses.I3RecoPulseSeriesMap()
    for string in range(1,n_strings+1):
        for om in range(1,n_doms+1):
            om_key=icetray.OMKey(string,om)
            n_hits=np.random.poisson(lam=self.lambda_poisson)
            times=(np.random.random(n_hits)*self.event_length).round(1)
            pulse_series=dataclasses.I3RecoPulseSeries()
            for time in sorted(times):
                pulse=dataclasses.I3RecoPulse()
                pulse.time=time
                pulse.charge=1.0
                pulse_series.append(pulse)
            pulse_map[om_key]=pulse_series
            
    frame["SLOPPOZELA_Pulses"]=pulse_map
    self.PushFrame(frame)
    

class slopporator(icetray.I3Module):
  """
  Creates a simplified pseudo SLOP event straight along string 51 till string 59 
  for the purpose of testing the SLOPtools.
  The time interval between the hits on neighboured string are 1/9 of the event length.
  On each string DOM 40 till 44 is hit. The number of hits per OM is drawn from
  a poisson distribution with lambda=3. The hit times are drawn from a uniform distribution
  50 microseconds around the calculated hit time.
  For all five OMs on one string the calculated time is the same, assuming that the time for the
  direct propagating of the light (bright monopole) is much shorter than the time
  the monopole is in the vicinity of the OM. This generates the pattern that MPCleaner is able to find.
  All pulses get the charge 1.0 and the flag 7 (HLC), since TupleTagger only accounts HLCs (like the DAQ does).
  """
  def __init__(self, context):
    icetray.I3Module.__init__(self,context)
    self.AddParameter("event_length", "event_length", 1e6*icetray.I3Units.nanosecond)
    self.AddOutBox('OutBox')
    
  def Configure(self):
    self.event_length=self.GetParameter("event_length")
    
  def Physics(self, frame):
    pulse_map = dataclasses.I3RecoPulseSeriesMap()
    for string in range(51,60):
        track_time=self.event_length/9 * (string-50)
        for om in range(40,45):
            om_key=icetray.OMKey(string,om)
            n_hits=np.random.poisson(lam=3)
            times=(np.random.random(n_hits)*50000-25000+track_time).round(1)
            pulse_series=dataclasses.I3RecoPulseSeries()
            for time in sorted(times):
                pulse=dataclasses.I3RecoPulse()
                pulse.time=time
                pulse.charge=1.0
                pulse.flags=7
                pulse_series.append(pulse)
            pulse_map[om_key]=pulse_series
            
    frame["SLOPPORATOR_Pulses"]=pulse_map
    self.PushFrame(frame)
    


def slopunion(frame):
    slopunion=dataclasses.I3RecoPulseSeriesMapUnion(frame,["SLOPPOZELA_Pulses","SLOPPORATOR_Pulses"])
    frame["SLOP_Pulses"]=slopunion
    return True
    
    