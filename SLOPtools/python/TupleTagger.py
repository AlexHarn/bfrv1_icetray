# TupleTagger
# Tag SLOP tuples from pulses
#
# (c) 2012 The IceCube Collaboration
# written by: Emanuel Jacobi
#
# $Author: jacobi $
# $Revision: $
# $Date: 2015-08-23 05:46:55 -0400 (Sun, 23 Aug 2015) $
#

import math
from icecube import icetray, dataclasses

class SlowMPHit():
  def __init__(self, OMKey, Time, Index):
    self.OMKey = OMKey
    self.Time = Time
    self.Index = Index

class TupleTagger(icetray.I3ConditionalModule):

  def __init__(self, context):
    icetray.I3ConditionalModule.__init__(self,context)
    self.AddParameter("PulseMapName", "Name of the pulse map or mask", "SLOPPulseMask")
    self.AddParameter("LaunchMapName", "Name of the DOMLaunch map", "InIceRawData")
    self.AddParameter("RunOnPulses", "Tag tuples from pulses", True)
    self.AddParameter("RunOnLaunches", "Tag tuples from launches", False)
    self.AddParameter("alpha_min", "Minimum opening angle for tuples", 140)
    self.AddParameter("rel_v", "Relative velocity inside a tuple", 0.5)
    self.AddParameter("min_tuples", "Minimum number of tuples", 5)
    self.AddParameter("t_proximity", "Muon cleaning time", 2500)
    self.AddParameter("t_min", "Minimum time difference inside a tuple", 0)
    self.AddParameter("t_max", "Maximum time difference inside a tuple", 500000)
    self.AddParameter("max_event_length", "Maximum event length", 5000000)
    self.AddParameter("ConstrainToTriggerTime", "Only evaluate hits from within the trigger window", False)
    self.AddOutBox('OutBox')

  def Configure(self):
    self.PulseMapName = self.GetParameter("PulseMapName")
    self.LaunchMapName = self.GetParameter("LaunchMapName")
    self.RunOnPulses = self.GetParameter("RunOnPulses")
    self.RunOnLaunches = self.GetParameter("RunOnLaunches")
    self.alpha_min = self.GetParameter("alpha_min")
    self.rel_v =  self.GetParameter("rel_v")
    self.min_tuples = self.GetParameter("min_tuples")
    self.t_proximity = self.GetParameter("t_proximity")
    self.t_min = self.GetParameter("t_min")
    self.t_max = self.GetParameter("t_max")
    self.max_event_length = self.GetParameter("max_event_length")
    self.constrainToTriggerTime = self.GetParameter("ConstrainToTriggerTime")
    self.OneLaunchTime = 6400 * icetray.I3Units.ns           # max time of pulses from one DOMLaunch.
    self.PulseTimeOffset = 135 * icetray.I3Units.ns          # default average offset of pulse time and launch time (used if deltaT cannot be calculated from GCD)
    self.cos_alpha_min = math.cos( (math.pi/180) * self.alpha_min)
    self.TriggerList = list()

  def Geometry(self, frame):
    self.geo=frame['I3Geometry']
    self.PushFrame(frame)

  def Calibration(self, frame):
    self.cal=frame['I3Calibration']
    self.PushFrame(frame)

  def DetectorStatus(self, frame):
    self.det=frame['I3DetectorStatus']
    if self.RunOnPulses:
        self.GetDeltaT()
    self.PushFrame(frame)

  def Physics(self, frame):
    if self.RunOnPulses:
        if frame.Has(self.PulseMapName):
            self.TuplePulseMask = dataclasses.I3RecoPulseSeriesMapMask(frame, self.PulseMapName)
            self.TuplePulseMask.set_none()
        else:
            print(("Error, cannot find pulse mask %s") % self.PulseMapName)
            return False
        self.TupleCosAlphaVector_Pulses = dataclasses.I3VectorDouble()
        self.TupleRelvVector_Pulses = dataclasses.I3VectorDouble()
        self.RunTaggerOnPulses(frame)
        frame['SLOPPulseMaskTuples'] = self.TuplePulseMask
        frame['SLOPTuples_CosAlpha_Pulses'] = self.TupleCosAlphaVector_Pulses
        frame['SLOPTuples_RelV_Pulses'] = self.TupleRelvVector_Pulses
    if self.RunOnLaunches:
        self.TupleLaunchMap = dataclasses.I3DOMLaunchSeriesMap()
        self.TupleCosAlphaVector_Launches = dataclasses.I3VectorDouble()
        self.TupleRelvVector_Launches = dataclasses.I3VectorDouble()
        self.RunTaggerOnLaunches(frame)
        frame['SLOPLaunchMapTuples'] = self.TupleLaunchMap
        frame['SLOPTuples_CosAlpha_Launches'] = self.TupleCosAlphaVector_Launches
        frame['SLOPTuples_RelV_Launches'] = self.TupleRelvVector_Launches
    self.PushFrame(frame)

  def DAQ(self, frame):
    if self.constrainToTriggerTime:
        assert(len(self.TriggerList)==0), "Assertion: len(self.TriggerList) is %d and should be 0" %len(self.TriggerList) # At the beginning of a Q Frame the trigger list should always be empty.
        triggerHierarchy=frame["DSTTriggers"].unpack(self.det)
        for trigger in triggerHierarchy:
            if trigger.key.type == dataclasses.I3Trigger.SLOW_PARTICLE:
                self.TriggerList.append(trigger)
        self.TriggerList.sort(key = lambda n: n.time)
    self.PushFrame(frame)

  def Finish(self):
    return True



  def RunTaggerOnPulses(self, frame):
    self.WorkingOnPulses = True
    self.TuplesFound = 0
    self.OneHitList = []
    self.TwoHitList = []
    self.muon_time_window = -1
    PulseMap = dataclasses.I3RecoPulseSeriesMap.from_frame(frame,self.PulseMapName)
    HLCPulseList = self.SelectPulses(PulseMap)
    for newHit in HLCPulseList:
        self.CheckOneHitList(newHit)
    self.CheckTriggerStatus()                                   # final check after looping through all hits
    return True



  def RunTaggerOnLaunches(self, frame):
    self.WorkingOnPulses = False
    self.TuplesFound = 0
    self.OneHitList = []
    self.TwoHitList = []
    self.muon_time_window = -1
    if frame.Has(self.LaunchMapName):
        InIceRawData = frame[self.LaunchMapName]
        HLCLaunchList = self.SelectLaunches(InIceRawData)
    else:
        print(("Error, cannot find DOMLaunches: %s") % self.LaunchMapName)
        return False
    for newHit in HLCLaunchList:
        self.CheckOneHitList(newHit)
    self.CheckTriggerStatus()                                   # final check after looping through all hits
    return True



  def SelectPulses(self, PulseMap):
    if self.constrainToTriggerTime:
        assert(len(self.TriggerList)>0),"len(self.TriggerList) is %d and should be >0"%len(self.TriggerList) # The P frames are created by trigger splitter so there should never be a P frame if the TriggerList is empty
        trigger=self.TriggerList.pop(0)
        #print("[SelectPulses]: Found SLOP trigger number %d (time: %f, duration: %f)" %(len(self.TriggerList),trigger.time, trigger.length) )

    HLCPulseList = []
    for OMKey, PulseVector in PulseMap.items():
        i=0                                                      # index of pulse in pulse vector (needed to set bits in the PulseMapMask)
        j=0                                                      # counter for HLC pulses in pulse vector (needed to determine if subsequent pulses belong to one launch)
        for Pulse in PulseVector:
            if Pulse.flags % 2 == 1:                             # we want only HLCs
                if self.constrainToTriggerTime and ( Pulse.time <  trigger.time or Pulse.time > trigger.time+trigger.length ):
                    continue
                else:
                    if j == 0:                                       # always consider the first pulse of a given OM
                        myPulse = SlowMPHit(OMKey, Pulse.time + self.OffsetsDT[OMKey], i)
                        HLCPulseList.append(myPulse)
                        j+=1
                    elif Pulse.time + self.OffsetsDT[OMKey] - HLCPulseList[-1].Time > self.OneLaunchTime:   # treat pulses close in time as one launch
                        myPulse = SlowMPHit(OMKey, Pulse.time + self.OffsetsDT[OMKey], i)
                        HLCPulseList.append(myPulse)
                        j+=1
            i+=1

    HLCPulseList.sort(key = lambda n: n.Time)     # sort all hits timewise
    return HLCPulseList



  def SelectLaunches(self, InIceRawData):
    if self.constrainToTriggerTime:
        assert(len(self.TriggerList)>0),"len(self.TriggerList) is %d and should be >0"%len(self.TriggerList) # The P frames are created by trigger splitter so there should never be a P frame if the TriggerList is empty
        trigger=self.TriggerList.pop(0)
        #print("[SelectLaunches]: Found SLOP trigger number %d (time: %f, duration: %f)" %(len(self.TriggerList),trigger.time, trigger.length) )

    HLCLaunchList = []
    for OMKey, LaunchVector in InIceRawData.items():
        i=0
        for Launch in LaunchVector:
            if Launch.lc_bit == True:                             # we want only HLCs
                if self.constrainToTriggerTime and ( Launch.time < trigger.time or Launch.time > trigger.time+trigger.length ):
                    continue
                else:
                    myLaunch = SlowMPHit(OMKey, Launch.time, i)
                    HLCLaunchList.append(myLaunch)
            i+=1
    HLCLaunchList.sort(key = lambda n: n.Time)     # sort all hits timewise
    return HLCLaunchList



  def CheckOneHitList(self, newHit):
    if len(self.OneHitList) == 0:                        # one hit list is empty. just add current hit to list
        self.OneHitList.append(newHit)
    else:                                                # one hit list contains stuff. compare current hit to the one in the list
        while math.fabs(newHit.Time - self.OneHitList[0].Time) > 1000.:
            self.OneHitList.pop(0)
            if len(self.OneHitList) == 0: break
        i=0
        while i < len(self.OneHitList):                  # iterate over one hit list to form HLCs
            if self.HLCPairCheck(self.OneHitList[i], newHit):
                payload=self.OneHitList[i]
                self.CheckTwoHitList(payload)
                self.OneHitList.pop(i)                   # delete the used hit from OneHitList after checking TwoHitList
            else:
                i+=1
        self.OneHitList.append(newHit)                   # at the end add the current hitPayload for further comparisons
        if len(self.TwoHitList) > 0:                     # definitly cannot produce a trigger...
            if self.OneHitList[0].Time - self.TwoHitList[-1].Time > self.t_max:
                self.CheckTriggerStatus()



  def CheckTwoHitList(self, payload):
    if len(self.TwoHitList) == 0:                        # hit list is empty
        if self.muon_time_window == -1:
            self.TwoHitList.append(payload)                      # add to list (no time_window set)
        else:
            if payload.Time - self.muon_time_window <= self.t_proximity:       
                self.muon_time_window = payload.Time                 # not adding hit, due to t_proximity, set time window to new hit
            else:
                self.TwoHitList.append(payload)                  # add to list (time window set)
                self.muon_time_window = -1                         # reset time window
    else:                                                   # hit list is not empty
        if self.muon_time_window == -1:
            if payload.Time - self.TwoHitList[-1].Time <= self.t_proximity:
                self.muon_time_window = payload.Time                 # t_proximity test failed, discard current hit, set time window, remove last hit
                self.TwoHitList.pop()
            else:                                         # t_proximity test passed
                if payload.Time - self.TwoHitList[-1].Time < self.t_max\
                and payload.Time - self.TwoHitList[0].Time < self.max_event_length:
                    self.TwoHitList.append(payload)              # add to list
                else:
                    self.CheckTriggerStatus()     # this hit is too late to make a tuple, but check previous hits in list
                    self.TwoHitList.append(payload)              # now add current hit to list
        else:                                            # time window is set
                 if payload.Time - self.muon_time_window <= self.t_proximity:
                     self.muon_time_window = payload.Time        # t_proximity test failed, disregard current pulse, set time window
                 else:                                      # t_proximity test passed
                     self.muon_time_window = -1                    # this is not a muon, set window back to -1
                     if payload.Time - self.TwoHitList[-1].Time < self.t_max\
                     and payload.Time - self.TwoHitList[0].Time < self.max_event_length:
                         self.TwoHitList.append(payload)          # add to list
                     else:
                         self.CheckTriggerStatus() # this hit is too late to make a tuple, but check previous hits in list
                         self.TwoHitList.append(payload)          # now add current hit to list


  def HLCPairCheck(self, hit1, hit2):
    if hit1.OMKey.string == hit2.OMKey.string:
        if abs(hit1.OMKey.om - hit2.OMKey.om) <= 2:
            return True
    return False



  def CheckTriggerStatus(self):
    if len(self.TwoHitList) >=3:
        for hit1 in self.TwoHitList[:-2]:
            for hit2 in self.TwoHitList[self.TwoHitList.index(hit1)+1:-1]:
                for hit3 in self.TwoHitList[self.TwoHitList.index(hit2)+1:]:
                    self.CheckTriple(hit1, hit2, hit3)
    # the c++ version loops here through the trigger_container_vector (which holds the tuples)
    # and issues the trigger if n>=min_tuples. after that the container is cleared
    self.TwoHitList = []
 


  def CheckTriple(self, hit1, hit2, hit3):
    t_diff1 = hit2.Time - hit1.Time
    t_diff2 = hit3.Time - hit2.Time
    t_diff3 = hit3.Time - hit1.Time
    if t_diff1 > self.t_min and t_diff2 > self.t_min and t_diff1 < self.t_max and t_diff2 < self.t_max:
        p_diff1 = self.GetDistance(hit1, hit2)
        p_diff2 = self.GetDistance(hit2, hit3)
        p_diff3 = self.GetDistance(hit1, hit3)
        if not ( p_diff1 > 0 and p_diff2 > 0 and p_diff3 > 0 ):
            return False
        cos_alpha = ( p_diff1**2 + p_diff2**2 - p_diff3**2 ) / ( 2 * p_diff1 * p_diff2 )
        if cos_alpha <= self.cos_alpha_min:
            inv_v1 = t_diff1/p_diff1
            inv_v2 = t_diff2/p_diff2
            inv_v3 = t_diff3/p_diff3
            inv_v_mean = ( inv_v1 + inv_v2 + inv_v3 ) / 3.0
            if math.fabs(inv_v2 - inv_v1)/inv_v_mean <= self.rel_v:
                if self.WorkingOnPulses:
                    self.TuplePulseMask.set(hit1.OMKey, hit1.Index, True)
                    self.TuplePulseMask.set(hit2.OMKey, hit2.Index, True)
                    self.TuplePulseMask.set(hit3.OMKey, hit3.Index, True)
                    self.TupleCosAlphaVector_Pulses.append(cos_alpha)
                    self.TupleRelvVector_Pulses.append(math.fabs(inv_v2 - inv_v1)/inv_v_mean)
                else:
                    self.AppendLaunches(self.TupleLaunchMap, hit1)
                    self.AppendLaunches(self.TupleLaunchMap, hit2)
                    self.AppendLaunches(self.TupleLaunchMap, hit3)
                    self.TupleCosAlphaVector_Launches.append(cos_alpha)
                    self.TupleRelvVector_Launches.append(math.fabs(inv_v2 - inv_v1)/inv_v_mean)
                self.TuplesFound+=1
                return True



  def GetDistance(self, hit_a, hit_b):
    x1 = self.geo.omgeo[hit_a.OMKey].position.x
    y1 = self.geo.omgeo[hit_a.OMKey].position.y
    z1 = self.geo.omgeo[hit_a.OMKey].position.z
    x2 = self.geo.omgeo[hit_b.OMKey].position.x
    y2 = self.geo.omgeo[hit_b.OMKey].position.y
    z2 = self.geo.omgeo[hit_b.OMKey].position.z
    diff = math.sqrt( (x2-x1)**2 + (y2-y1)**2 + (z2-z1)**2 )
    return diff


  def AppendLaunches(self, launchmap, hit):
    newLaunch = dataclasses.I3DOMLaunch()
    newLaunch.time = hit.Time
    newLaunch.lc_bit = True
    if hit.OMKey not in launchmap.keys():
        newLaunchSeries = dataclasses.I3DOMLaunchSeries()
        newLaunchSeries.append(newLaunch)
        launchmap[hit.OMKey] = newLaunchSeries
    else:
        for launch in  launchmap[hit.OMKey]:
            if launch.time == hit.Time: return
        launchmap[hit.OMKey].append(newLaunch)


  def GetDeltaT(self):     # calculate the delay between launch and pulse from the calibration 
    self.OffsetsDT = {}    # see https://wiki.icecube.wisc.edu/index.php/Transit_time
    for string in range(1,87):
        for om in range(1,61):
            omkey = icetray.OMKey(string, om, 0)
            self.OffsetsDT[omkey] = self.PulseTimeOffset                 # initialize with default value

    for omkey in self.OffsetsDT.keys():
        try:
            fit   = self.cal.dom_cal[omkey].transit_time
            pmtHV = self.det.dom_status[omkey].pmt_hv/icetray.I3Units.V
        except:
            continue
        if pmtHV > 0:
            transit_time = fit.slope/math.sqrt(pmtHV) + fit.intercept
            self.OffsetsDT[omkey] = transit_time

