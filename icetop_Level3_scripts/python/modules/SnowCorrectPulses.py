from icecube import icetray, toprec
from icecube.dataclasses import I3RecoPulseSeriesMap, I3RecoPulseSeries, I3RecoPulse, I3Particle

# Snow correct the pulses.
# Put an empty RecoPulseSeries in the frame when the reconstruction failed.

class SnowCorrectPulses(icetray.I3ConditionalModule):
     def __init__(self, ctx):
         icetray.I3ConditionalModule.__init__(self,ctx)
         self.AddParameter('Pulses','Array of pulses to snowCorrect',[])
         self.AddParameter('Track', 'Reconstructed track to use for the zenith angle','Laputop')
         self.AddParameter('SnowService','Which Snow service to use for the correction',None)
         self.AddOutBox("OutBox")
         
     def Configure(self):
         self.pulseSeries=self.GetParameter('Pulses')
         self.reco=self.GetParameter('Track')
         self.snowservice=self.GetParameter('SnowService')
         self.staGeo=None
          
     def Geometry(self,frame):
         self.staGeo = frame['I3Geometry'].stationgeo
         self.PushFrame(frame)

     def Physics(self,frame):
          useReco=False
          if (self.reco in frame and frame[self.reco].fit_status_string=="OK"):
               reco=frame[self.reco]
               recoParam=frame[self.reco+"Params"]
               useReco=True
          for pulses in self.pulseSeries:
               psm_nosnow = I3RecoPulseSeriesMap.from_frame(frame, pulses)
               psm = I3RecoPulseSeriesMap()
               for omkey, series_nosnow in psm_nosnow.items():
                    for tank in self.staGeo[omkey.string]:
                         if (omkey in tank.omkey_list and useReco):
                              pos = tank.position
                              snow_depth = tank.snowheight
                              att = self.snowservice.attenuation_factor(pos, snow_depth, reco, recoParam)
                              break
                    series = I3RecoPulseSeries()
                    if useReco:
                         for pulse_nosnow in series_nosnow:
                              pulse = I3RecoPulse(pulse_nosnow)
                              pulse.charge = pulse_nosnow.charge / att
                              series.append(pulse)
                    psm[omkey] = series
               frame[pulses + "_SnowCorrected"] = psm
          self.PushFrame(frame)
