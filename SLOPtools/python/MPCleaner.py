# MPCleaner
# Masks DOMs which have multiple pulses in one event
#
# (c) 2012 The IceCube Collaboration
# written by: Emanuel Jacobi
#
# $Author: jacobi $
# $Revision: 136760 $
# $Date: 2015-08-23 05:46:55 -0400 (Sun, 23 Aug 2015) $
#

from icecube import icetray, dataclasses

class MPCleaner(icetray.I3ConditionalModule):

  def __init__(self, context):
    icetray.I3ConditionalModule.__init__(self,context)
    self.AddParameter("PulseMapName", "Name of the input pulse map", "SLOPPulseMask")
    self.AddParameter("MinTimeBetweenPulses", "Minimum time between two pulses in one DOM",  10000)
    self.AddParameter("MaxTimeBetweenPulses", "Maximum time between two pulses in one DOM", 300000)
    self.AddParameter("MinNumberOfPulses", "Minimum number of pulses in one DOM", 3)
    self.AddOutBox('OutBox')

  def Configure(self):
    self.pulseMapName = self.GetParameter("PulseMapName")
    self.minTimeBetweenPulses = self.GetParameter("MinTimeBetweenPulses")
    self.maxTimeBetweenPulses = self.GetParameter("MaxTimeBetweenPulses")
    self.minNumberOfPulses = self.GetParameter("MinNumberOfPulses")
    self.pulseMapNameMPClean = self.pulseMapName + 'MPClean'
    self.pulseMapNameSuperClean =  self.pulseMapName + 'SuperClean'
    self.pulseMapNameHyperClean =  self.pulseMapName + 'HyperClean'
    self.pulseMapNameUltraClean =  self.pulseMapName + 'UltraClean'

  def Physics(self, frame):
    if frame.Has(self.pulseMapName):
        RecoPulses = dataclasses.I3RecoPulseSeriesMap.from_frame(frame,self.pulseMapName)
    else:
        print("Error, could not find %s" % self.pulseMapName)
        return False

    # make a new pulse mask. upon initialization they are copies of the SLOPPulses        
    SLOPPulseMaskMPClean=dataclasses.I3RecoPulseSeriesMapMask(frame, self.pulseMapName)
    SLOPPulseMaskSuperClean=dataclasses.I3RecoPulseSeriesMapMask(frame, self.pulseMapName)
    SLOPPulseMaskHyperClean=dataclasses.I3RecoPulseSeriesMapMask(frame, self.pulseMapName)
    SLOPPulseMaskUltraClean=dataclasses.I3RecoPulseSeriesMapMask(frame, self.pulseMapName)
    

    # loop over OMs, and set pulses to false if they should not belong to this mask.
    for om_key in RecoPulses.keys():
        neighborDOMs = [ icetray.OMKey(om_key.string,om_key.om+j) for j in [-2,-1,1,2] if om_key.om+j>0 and om_key.om+j<=60 ]

        def check_len(om):
            try:
                if len(RecoPulses[om]) < self.minNumberOfPulses:
                    return False
                else:
                    return True
            except KeyError:
                return False
                
        def check_time(om):
            try:
                pulse_vector = list(RecoPulses[om])
            except KeyError:
                return False
            # reco pulses should be time sorted already, but better safe than sorry
            pulse_vector.sort(key = lambda k: k.time) 
            mask=list( [True] * len(pulse_vector) )
            for i in range(len(pulse_vector)-1):
                tdiff_fwd=pulse_vector[i+1].time - pulse_vector[i].time
                tdiff_pre=pulse_vector[i].time - pulse_vector[i-1].time
                if tdiff_fwd > self.minTimeBetweenPulses and tdiff_fwd < self.maxTimeBetweenPulses:
                    pass
                if tdiff_pre > self.minTimeBetweenPulses and tdiff_pre < self.maxTimeBetweenPulses:
                    pass
                else:
                    mask[i] = False
                #last iteration    
                if i == len(pulse_vector)-2:
                    if tdiff_fwd > self.minTimeBetweenPulses and tdiff_fwd < self.maxTimeBetweenPulses:
                        pass
                    else:
                        mask[i+1] = False
            if sum(mask)>=3:
                return True
            else:
                return False
                
        def check_len_n_time(om):
            return check_len(om) & check_time(om)
                
        if not check_len(om_key):
            SLOPPulseMaskMPClean.set(om_key, False)
            SLOPPulseMaskSuperClean.set(om_key, False)
            SLOPPulseMaskHyperClean.set(om_key, False)
            SLOPPulseMaskUltraClean.set(om_key, False)
        else:
            # Super Clean
            pulse_vector = list(RecoPulses[om_key])
            # reco pulses should be time sorted already, but better safe than sorry
            pulse_vector.sort(key = lambda k: k.time)
            for i in range(len(pulse_vector)-1):
                tdiff_fwd  = pulse_vector[i+1].time - pulse_vector[i].time
                tdiff_pre =  pulse_vector[i].time - pulse_vector[i-1].time
                if tdiff_fwd > self.minTimeBetweenPulses and tdiff_fwd < self.maxTimeBetweenPulses:
                    pass
                elif tdiff_pre > self.minTimeBetweenPulses and tdiff_pre < self.maxTimeBetweenPulses:
                    pass
                else:
                    SLOPPulseMaskSuperClean.set(om_key, i, False)
                    SLOPPulseMaskUltraClean.set(om_key, i, False)
                # last iteration
                if i == len(pulse_vector)-2:
                    if tdiff_fwd > self.minTimeBetweenPulses and tdiff_fwd < self.maxTimeBetweenPulses:
                        pass
                    else:
                        SLOPPulseMaskSuperClean.set(om_key, i+1, False)
                        SLOPPulseMaskUltraClean.set(om_key, i+1, False)
                        
            # Hyper Clean
            if not any(map(check_len,neighborDOMs)):
                SLOPPulseMaskHyperClean.set(om_key, False)
                SLOPPulseMaskUltraClean.set(om_key, False)
                
            # Ultra Clean
            if not any(map(check_len_n_time,neighborDOMs)):
                SLOPPulseMaskUltraClean.set(om_key, False)
              

    # write the new pulse mask to the frame
    frame[self.pulseMapNameMPClean] = SLOPPulseMaskMPClean
    frame[self.pulseMapNameSuperClean] = SLOPPulseMaskSuperClean
    frame[self.pulseMapNameHyperClean] = SLOPPulseMaskHyperClean
    frame[self.pulseMapNameUltraClean] = SLOPPulseMaskUltraClean
        
    # push the P-frame
    self.PushFrame(frame)
            

  def DAQ(self, frame):
    self.PushFrame(frame)
  
  def Finish(self):
    return True

