#!/usr/bin/env python
import os
from glob import glob
from I3Tray import I3Tray
from icecube import icetray, dataclasses, dataio, trigger_splitter
from icecube.icetray import I3PacketModule
 
tray = I3Tray()

I3_TESTDATA = os.path.expandvars("$I3_TESTDATA")
if not I3_TESTDATA.endswith("/") :
    I3_TESTDATA += "/"

gcd_file = I3_TESTDATA + "sim/GeoCalibDetectorStatus_2013.56429_V1.i3.gz"
input_file = I3_TESTDATA + "sim/Level2_IC86.2011_corsika.010281.001664.00.i3.bz2"

tray.AddModule("I3Reader", Filenamelist = [gcd_file, input_file] )

# I drop the null split p-frames that L2a comes with...
# not necessary but don't wanna check for stream name in sanity check module
tray.AddModule(lambda frame: 0, Streams=[icetray.I3Frame.Physics])

# Rename the trigger Hieararchy names in the q-frame so it has a different name from the p-frame
keys=['I3TriggerHierarchy','QTriggerHierarchy']
tray.AddModule("Rename", Keys = keys )

# here I'm splitting using SMT8 and ClusterTrigger as triggers of interest
# the default puts a new trigger hieararchy named "I3TriggerHierarchy" in P-frames, 
# Q-frame trig hierarchy will remain named "QTriggerHierarchy"
tray.AddModule("I3TriggerSplitter",
    TrigHierName = 'QTriggerHierarchy',
    InputResponses = ['OfflinePulses'],
    OutputResponses = ['NaokosTrigSplitPulses'],
    TriggerConfigIDs = [1006,1007]
)


class GetStartTime(I3PacketModule):
    def __init__(self, context):
        I3PacketModule.__init__(self, context, icetray.I3Frame.DAQ)
        self.AddOutBox("OutBox")
    def Configure(self):
        pass
    def FramePacket(self, frames):
        if frames[0].Has('I3EventHeader'):
            globStartTime = frames[0]['I3EventHeader'].start_time
        for fr in frames:
            fr['globalStartTime']=globStartTime
            self.PushFrame(fr)
tray.AddModule(GetStartTime, "writeStartTime")

# a quick python module to check if the trigger splitter ran correctly
def SanityCheck(frame):
    if frame.Has('NaokosTrigSplitPulses'):
        stime = frame['I3EventHeader'].start_time - frame['globalStartTime']
        etime = frame['I3EventHeader'].end_time   - frame['globalStartTime']
        times=[]
        for keys, values in frame['NaokosTrigSplitPulses'].apply(frame):
            for ent in values:
                times.append(ent.time)
        times.sort() #time-order pulses
        if sum([(x<stime-200.0 or x>etime) for x in times]) > 0:
            #give it some wiggle room since we don't know the launch-pulse time difference
            print('SPLITTING GONE WRONG!!!: ' +
                  repr(sum([(x<stime or x>etime) for x in times])) +
                  ' pulses outside range ('+repr(len(times))+' pulses total)')
            print(frame)

tray.AddModule(SanityCheck, Streams=[icetray.I3Frame.Physics])

tray.AddModule("I3Writer",
               Filename = "TestOutput.i3",
               Streams = [icetray.I3Frame.Geometry,
                          icetray.I3Frame.Calibration,
                          icetray.I3Frame.DetectorStatus,
                          icetray.I3Frame.DAQ,
                          icetray.I3Frame.Physics])

tray.Execute(20)

