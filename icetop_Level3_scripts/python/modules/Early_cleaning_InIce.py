from icecube import phys_services,dataclasses, icetray
import math

class Early_cleaning_InIce(icetray.I3ConditionalModule): 
    """ 
    Test on double coinc data (select 1 day, first_time II - first_time IT < 4us) 
    RESULT: WORKS perfectly, but not that fast... !!                                                                                                                                                            defines function which selects hits in cylinder around the track and hits can't be earlier than track
    """
    def __init__(self, ctx):
          icetray.I3ConditionalModule.__init__(self, ctx)
          self.AddParameter("trackName","Name of the reco icetop track",None)
          self.AddParameter("inputpulseName", "Input inice pulses",None)
          self.AddParameter("outputpulseName","Output inice pulses",None)
          self.AddParameter("max_radius", "Maximum radius", None)
          self.AddParameter("min_time","Minimum time",None)
          self.AddOutBox("OutBox")

    def Configure(self):
          self.trackname=self.GetParameter("trackName")
          self.inputpulses=self.GetParameter("inputpulseName")
          self.outputpulses=self.GetParameter("outputpulseName")
          self.maxRadius=self.GetParameter("max_Radius")
          self.minTime=self.GetParameter("min_time")
          self.omgeo=None
          
    def Geometry(self,frame):
        self.omgeo=frame['I3Geometry'].omgeo
        self.PushFrame(frame,"OutBox")

    def Physics(self,frame):
        if (self.trackname in frame and self.inputpulses in frame): 
            track = frame[self.trackname]
            select = lambda omkey, index, pulse: phys_services.I3Calculator.time_residual(track, self.omgeo[omkey].position, pulse.time,dataclasses.I3Constants.n_ice_group,dataclasses.I3Constants.n_ice_phase) > self.minTime\
                and phys_services.I3Calculator.closest_approach_distance(track,self.omgeo[omkey].position) < self.maxRadius

            frame[self.outputpulses] = dataclasses.I3RecoPulseSeriesMapMask(frame, self.inputpulses, select)
        else: 
            frame[self.outputpulses]= dataclasses.I3RecoPulseSeriesMap()
            
        self.PushFrame(frame,"OutBox")
        
