#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright (c) 2019
# Jakob van Santen <jakob.van.santen@desy.de>
# and the IceCube Collaboration <http://www.icecube.wisc.edu>
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
# OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
# $Id$
#
# @file hobo-multisim.py
# @version $Revision$
# @date $Date$
# @author Jakob van Santen

from os.path import expandvars
import os
import itertools, tempfile, json

from icecube import icetray, dataclasses, dataio, clsim
from I3Tray import I3Tray
from icecube.clsim.traysegments.common import setupPropagators, setupDetector, configureOpenCLDevices
from icecube.clsim.traysegments.I3CLSimMakePhotons import I3CLSimMakePhotonsWithServer

from icecube.ice_models.icewave import PlusModePerturber

class FrameSequenceReader(icetray.I3Module):
    def __init__(self, ctx):
        super(FrameSequenceReader,self).__init__(ctx)
        self.AddParameter("Sequence", "Iterable of frames to emit", None)
    def Configure(self):
        self._frames = self.GetParameter("Sequence")
    def Process(self):
        frame = self._frames.next()
        if frame is None:
            self.RequestSuspension()
        else:
            self.PushFrame(frame)

class Bumper(icetray.I3Module):
    def __init__(self, ctx):
        super(Bumper,self).__init__(ctx)
        self.AddParameter("NumFrames", "", 100)
    def Configure(self):
        self._numframes = self.GetParameter("NumFrames")
        self._count = 0
    def DAQ(self, frame):
        self._count += 1
        if self._count >= self._numframes:
            self.RequestSuspension()
        else:
            self.PushFrame(frame)

class GatherStatistics(icetray.I3Module):
    """Mimick the summary stage of I3CLSimModule::Finish()"""
    def Finish(self):
        if not 'I3SummaryService' in self.context:
            return
        summary = self.context['I3SummaryService']
        server = self.context['CLSimServer']
        for k, v in server.GetStatistics().items():
            if k in summary and (k.startswith('Total') or k.startswith('Num')):
                summary[k] += v
            else:
                summary[k] = v

def HoboMultiSim(
    GCDFile,
    InputFiles,
    OutputFile,
    RandomService,
    SummaryFile=None,
    NumEventsPerModel=100,
    UseCPUs=False,
    UseGPUs=True,
    UseOnlyDeviceNumber=None,
    MCTreeName="I3MCTree",
    OutputMCTreeName=None,
    FlasherInfoVectName=None,
    FlasherPulseSeriesName=None,
    MMCTrackListName="MMCTrackList",
    PhotonSeriesName="PhotonSeriesMap",
    MCPESeriesName="MCPESeriesMap",
    IceModelLocation=expandvars("$I3_BUILD/ice-models/resources/models/spice_lea"),
    DisableTilt=False,
    UnWeightedPhotons=False,
    UnWeightedPhotonsScalingFactor=None,
    UseGeant4=False,
    ParticleHistory=False,
    CrossoverEnergyEM=None,
    CrossoverEnergyHadron=None,
    UseCascadeExtension=True,
    StopDetectedPhotons=True,
    PhotonHistoryEntries=0,
    DoNotParallelize=False,
    DOMOversizeFactor=5.,
    UnshadowedFraction=0.9,
    HoleIceParameterization=expandvars("$I3_BUILD/ice-models/resources/models/angsens/as.h2-50cm"),
    WavelengthAcceptance=None,
    DOMRadius=0.16510*icetray.I3Units.m, # 13" diameter
    OverrideApproximateNumberOfWorkItems=None,
    IgnoreSubdetectors=['IceTop'],
    ExtraArgumentsToI3CLSimClientModule=dict(),
    ):
    """
    Mimick the original Snowstorm (aka MultiSim) by running a series of short
    trays, each with a different ice model. This works by front-loading as much
    of the expensive initialization (reading the GCD file, setting up
    PROPOSAL/Geant4, etc) as possible, so that the only the propagation kernel
    needs to be recompiled for every tray.
    """
    if not isinstance(ExtraArgumentsToI3CLSimClientModule, dict):
        ExtraArgumentsToI3CLSimClientModule = dict()

    clsimParams = setupDetector(
        GCDFile=GCDFile,
        SimulateFlashers=bool(FlasherInfoVectName or FlasherPulseSeriesName),
        IceModelLocation=IceModelLocation,
        DisableTilt=DisableTilt,
        UnWeightedPhotons=UnWeightedPhotons,
        UnWeightedPhotonsScalingFactor=UnWeightedPhotonsScalingFactor,
        MMCTrackListName=MMCTrackListName,
        UseGeant4=UseGeant4,
        CrossoverEnergyEM=CrossoverEnergyEM,
        CrossoverEnergyHadron=CrossoverEnergyHadron,
        UseCascadeExtension=UseCascadeExtension,
        StopDetectedPhotons=StopDetectedPhotons,
        DOMOversizeFactor=DOMOversizeFactor,
        UnshadowedFraction=UnshadowedFraction,
        HoleIceParameterization=HoleIceParameterization,
        WavelengthAcceptance=WavelengthAcceptance,
        DOMRadius=DOMRadius,
        IgnoreSubdetectors=IgnoreSubdetectors,
    )

    openCLDevices = configureOpenCLDevices(
        UseGPUs=UseGPUs,
        UseCPUs=UseCPUs,
        OverrideApproximateNumberOfWorkItems=OverrideApproximateNumberOfWorkItems,
        DoNotParallelize=DoNotParallelize,
        UseOnlyDeviceNumber=UseOnlyDeviceNumber
    )

    gcdFrames = list(dataio.I3File(GCDFile))
    inputStream = dataio.I3FrameSequence(InputFiles)
    perturber = PlusModePerturber(clsimParams['MediumProperties'])
    summary = dataclasses.I3MapStringDouble()
    intermediateOutputFiles = []

    while inputStream.more():
        tray = I3Tray()
        tray.context['I3RandomService'] = RandomService
        tray.context['I3SummaryService'] = summary
        
        # TODO: populate M frame
        model = icetray.I3Frame('M')
        clsimParams['MediumProperties'] = perturber.perturb(RandomService)

        # TODO: populate S frame

        tray.Add(FrameSequenceReader, Sequence=itertools.chain(gcdFrames, [model], inputStream))
        tray.Add(Bumper, NumFrames=NumEventsPerModel)

        address = 'ipc://'+tempfile.mktemp(prefix='clsim-server-')
        converters = setupPropagators(RandomService, clsimParams,
            UseGPUs=UseGPUs,
            UseCPUs=UseCPUs,
            OverrideApproximateNumberOfWorkItems=OverrideApproximateNumberOfWorkItems,
            DoNotParallelize=DoNotParallelize,
            UseOnlyDeviceNumber=UseOnlyDeviceNumber
        )
        server = clsim.I3CLSimServer(address, clsim.I3CLSimStepToPhotonConverterSeries(converters))
        # stash server instance in the context to keep it alive
        tray.context['CLSimServer'] = server

        module_config = \
            tray.Add(I3CLSimMakePhotonsWithServer,
                ServerAddress=address,
                DetectorSettings=clsimParams,
                MCTreeName=MCTreeName,
                OutputMCTreeName=OutputMCTreeName,
                FlasherInfoVectName=FlasherInfoVectName,
                FlasherPulseSeriesName=FlasherPulseSeriesName,
                MMCTrackListName=MMCTrackListName,
                PhotonSeriesName=PhotonSeriesName,
                MCPESeriesName=MCPESeriesName,
                RandomService=RandomService,
                ParticleHistory=ParticleHistory,
                ExtraArgumentsToI3CLSimClientModule=ExtraArgumentsToI3CLSimClientModule
            )
        # recycle StepGenerator to prevent repeated, expensive initialization
        ExtraArgumentsToI3CLSimClientModule['StepGenerator'] = module_config['StepGenerator']

        intermediateOutputFiles.append(tempfile.mktemp(suffix=OutputFile,dir=os.path.dirname(OutputFile)))
        tray.Add("I3Writer", Streams=map(icetray.I3Frame.Stream, 'SMQP'),
            filename=intermediateOutputFiles[-1])
        tray.Add(GatherStatistics)
        tray.Execute()

    # Concatenate intermediate files
    tray = I3Tray()
    tray.Add("I3Reader", FilenameList=intermediateOutputFiles)
    tray.Add("I3Writer", Filename=OutputFile, Streams=map(icetray.I3Frame.Stream, 'SMQP'))
    tray.Execute()
    for fname in intermediateOutputFiles:
        os.unlink(fname)

    # Recalculate averages
    summary['DeviceUtilization'] = summary['TotalDeviceTime']/summary['TotalHostTime']
    summary['AverageDeviceTimePerPhoton'] = summary['TotalDeviceTime']/summary['TotalNumPhotonsGenerated']
    summary['AverageHostTimePerPhoton'] = summary['TotalHostTime']/summary['TotalNumPhotonsGenerated']
    if SummaryFile:
        with open(SummaryFile, 'w') as f:
            json.dump(dict(summary), f)

if __name__ == "__main__":
    from argparse import ArgumentParser
    parser = ArgumentParser()
    parser.add_argument("inputfiles", nargs="+")
    parser.add_argument("outputfile")
    parser.add_argument("--summaryfile")
    parser.add_argument("--gcdfile")
    parser.add_argument("--seed", type=int)
    parser.add_argument("--events-per-model", type=int, default=100)
    parser.add_argument("--cpu", action="store_true", default=False)
    args = parser.parse_args()

    from icecube import phys_services
    HoboMultiSim(args.gcdfile, args.inputfiles, args.outputfile,
        phys_services.I3GSLRandomService(args.seed),
        NumEventsPerModel=args.events_per_model,
        SummaryFile=args.summaryfile,
        UseCPUs=args.cpu, UseGPUs=not args.cpu)
