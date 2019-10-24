#!/usr/bin/env python

from I3Tray import *
import sys, os, glob
from icecube import icetray, dataio, dataclasses, hdfwriter, phys_services, lilliput, gulliver, gulliver_modules, linefit, rootwriter
from icecube.icetray import I3Frame, I3PacketModule


from icecube.level3_filter_muon.level3_Functions import CleanInputStreams
from icecube.level3_filter_muon.level3_Cuts import DoPrecuts,DoLevel3Cuts
from icecube.level3_filter_muon.level3_SplitHiveSplitter import SplitAndRecoHiveSplitter
from icecube.level3_filter_muon.level3_CalculateCutValues import CalculateCutValues
from icecube.level3_filter_muon.level3_Reconstruct import DoReconstructions
from icecube.level3_filter_muon.level3_WriteOutput import WriteOutput

@icetray.traysegment
def RestoreTimewindow(tray, name) :
    # Restore SplitInIcePulsesTimeRange if it doesn't exist.

    icetray.load("trigger-splitter", False)

    # redo TriggerSplitter and write TimeRanges
    # see filterscripts/offlineL2/Rehydration.py.
    # we copy exactly same code from for Data.
    tray.AddModule('I3TriggerSplitter',name+'_InIceSplit',
        SubEventStreamName = 'InIceSplit_dummy',
        TrigHierName = 'DSTTriggers',
        InputResponses = ['InIceDSTPulses','InIcePulses'],
        OutputResponses = ['SplitInIceDSTPulses', 'SplitInIcePulses'],
        WriteTimeWindow = True,
    )

    class TimeWindowCopier(I3PacketModule):
        def __init__(self, ctx):
            I3PacketModule.__init__(self, ctx, I3Frame.DAQ)
            self.AddOutBox("OutBox")
        def Configure(self):
            pass
        def FramePacket(self, frames):
            targetframe=None
            dummyframe=None
            for frame in frames:
                stream=frame["I3EventHeader"].sub_event_stream
                if (stream=="InIceSplit"):
                    targetframe=frame
                if(stream=="InIceSplit_dummy"):
                    dummyframe=frame

            #just copy missing info from dummy frame
            if (dummyframe) and (targetframe) :
                if not targetframe.Has('SplitInIcePulsesTimeRange') :
                    targetframe['SplitInIcePulsesTimeRange'] = dummyframe['SplitInIcePulsesTimeRange']

            for frame in frames:
                stream=frame["I3EventHeader"].sub_event_stream
                # omit dummy frame here 
                if (stream!="InIceSplit_dummy") :
                    self.PushFrame(frame)

    # copy TimeRange
    tray.AddModule(TimeWindowCopier,"twc")


@icetray.traysegment
def MuonL3(tray, name, gcdfile, infiles, output_i3, output_hd5, output_root, photonicsdir, photonicsdriverdir, photonicsdriverfile, infmuonampsplinepath, infmuonprobsplinepath, cascadeampsplinepath, cascadeprobsplinepath, restore_timewindow_forMC = False):
    
    # Add I3Reader to the tray only if infiles are given
    if infiles is not None:
        # check for GCD file
        if gcdfile is None:
            raise RuntimeError("No GCD file specified!")
            
        files = [gcdfile]

        if isinstance(infiles, str):
            files.append(infiles)
        else:
            for f in infiles:files.append(f)

        tray.Add(dataio.I3Reader, FilenameList=files)
    
    #Select only events passing muon filter and that are inice
    tray.AddSegment(CleanInputStreams)

    # Some simulation don't have SplitInIcePulsesTimeRange...
    if restore_timewindow_forMC :
        tray.AddSegment(RestoreTimewindow)

    # Apply Precuts
    tray.AddSegment(DoPrecuts,
        Pulses="SRTInIcePulses",
        Suffix="",
        If=lambda frame: True)

    # Everything in the stream should now be our good events
    # So now, split, re-reconstruct and recombine
    Suffix="HV"
    tray.AddSegment(SplitAndRecoHiveSplitter,
        Suffix=Suffix)

    # Recalculate cut values for new pulses and fits from ttrigger
    final_stream = lambda frame: frame["I3EventHeader"].sub_event_stream=="Final"
    tray.AddSegment(CalculateCutValues,
        Pulses="SRT"+Suffix+"InIcePulses",
        Suffix=Suffix,
        If=final_stream)

    # Re-apply precuts
    tray.AddSegment(DoPrecuts,
        Pulses="SRT"+Suffix+"InIcePulses",
        Suffix=Suffix,
        If=final_stream)

    # Apply level3 cuts
    tray.AddModule(DoLevel3Cuts,
        Track="BestTrack",
        Pulses="SRT"+Suffix+"InIcePulses",
        Suffix=Suffix,
        If=final_stream)

    # Do level3 reconstructions
    tray.AddSegment(DoReconstructions,
        Pulses="TWSRT"+Suffix+"InIcePulses",
        Suffix=Suffix,
        photonicsdir=photonicsdir,
        photonicsdriverdir=photonicsdriverdir,
        photonicsdriverfile=photonicsdriverfile,
        infmuonampsplinepath=infmuonampsplinepath,
        infmuonprobsplinepath=infmuonprobsplinepath,
        cascadeampsplinepath=cascadeampsplinepath,
        cascadeprobsplinepath=cascadeprobsplinepath)


    # common variables, keep keys, write output
    tray.AddSegment(WriteOutput,
        Suffix=Suffix,
        output_i3=output_i3,
        output_hd5=output_hd5,
        output_root=output_root)
    