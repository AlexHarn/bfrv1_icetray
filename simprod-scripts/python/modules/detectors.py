import os
import sys
import getopt
import logging
from os.path import expandvars

from I3Tray import *
from icecube import icetray, dataclasses, simclasses, dataio
from icecube.icetray import I3Frame
from .. import ipmodule
from .. import segments
from ..util import ReadI3Summary, WriteI3Summary

logger = logging.getLogger('detector')


class ICBase(ipmodule.ParsingModule):
    """
    Base class for detectors IC86, IC79, IC59.

    Common parts are factored out to reduce code redundancy.
    Overwrite addparameter_X and segment_X in derived classes as needed
    (X may be input or output, and must be main).
    """

    def __init__(self, detector_label):
        ipmodule.ParsingModule.__init__(self)
        self.detector_label = detector_label

        self.addparameter_input()
        self.addparameter_output()
        self.addparameter_main()

    def Execute(self, stats):
        'Runs the tray, but the important stuff happens in segment_X(...)'
        if not ipmodule.ParsingModule.Execute(self,stats):
            return 0

        # Load libraries
        from icecube import clsim
        from icecube import phys_services
        from icecube import sim_services
        from icecube import vuvuzela
        from icecube import DOMLauncher
        from icecube import trigger_sim

        # Instantiate a tray
        tray = I3Tray()

        self.segment_input(tray, stats)
        self.segment_main(tray, stats)
        self.segment_output(tray, stats)

        # Execute the Tray
        tray.Execute()

        for k in tray.Usage():
            stats[str(k.key())+":usr"] = k.data().usertime
            stats[str(k.key())+":sys"] = k.data().systime
            stats[str(k.key())+":ncall"] = k.data().systime

        # Free memory
        del tray
        return 0

    def addparameter_input(self):
        self.AddParameter('gcdfile', 'GeoCalibDetStatus filename', '')
        self.AddParameter('inputfile', 'Input filename', '')
        self.AddParameter('seed', 'RNG Seed', 123)
        self.AddParameter('procnum', 'RNG stream number', 0)
        self.AddParameter('nproc', 'RNG number of streams', 1)
        self.AddParameter('MCPESeriesName', 'Name of MCPESeriesMap in frame', 'I3MCPESeriesMap')
        self.AddParameter('HistogramFilename', 'Histogram filename.', None)
        self.AddParameter('EnableHistogram', 'Write a SanityChecker histogram file.', False)

    def segment_input(self, tray, stats):
        rngstate = "rng.state"
        if not os.path.exists(rngstate):
            rngstate = ''
            self.logger.warning("no RNG state found. Using seed instead.")

        tray.AddService("I3SPRNGRandomServiceFactory","random",
            Seed=self.seed, StreamNum=self.procnum, NStreams=self.nproc,
            inStateFile=rngstate, outStateFile="rng.state",
        )

        # Configure IceTray modules
        tray.AddModule("I3Reader","reader", FilenameList=[self.gcdfile, self.inputfile])

        if self.mcpeseriesname != 'I3MCPESeriesMap':
            def mover(fr):
                if 'I3MCPESeriesMap' in fr:
                    del fr['I3MCPESeriesMap']
                fr['I3MCPESeriesMap'] = fr[self.mcpeseriesname]
            tray.Add(mover, "move_MCPESeries",Streams=[icetray.I3Frame.DAQ])

    def addparameter_main(self):
        # not having parameters is okay
        pass

    def segment_main(self, tray, stats):
        # not having a main segment is not okay! :)
        raise NotImplementedError

    def addparameter_output(self):
        self.AddParameter('outputfile', 'Output filename', 'output.i3.gz')
        self.AddParameter('SkipKeys', 'Skip keys for the writer', [])

    def segment_output(self, tray, stats):

        if self.enablehistogram and self.histogramfilename:         
            from icecube.production_histograms import ProductionHistogramModule
            from icecube.production_histograms.histogram_modules.simulation.pmt_response import PMTResponseModule
            from icecube.production_histograms.histogram_modules.simulation.dom_mainboard_response import InIceResponseModule
            from icecube.production_histograms.histogram_modules.simulation.trigger import TriggerModule
            from icecube.production_histograms.histograms.simulation.noise_occupancy import NoiseOccupancy

            tray.AddModule(ProductionHistogramModule, 
                           Histograms = [PMTResponseModule,
                                         InIceResponseModule,
                                         TriggerModule,
                                         NoiseOccupancy],
                           OutputFilename = self.histogramfilename)

        tray.AddModule("I3Writer","writer",
            Filename=self.outputfile,
            SkipKeys=self.skipkeys,
            Streams=[icetray.I3Frame.TrayInfo,
                     icetray.I3Frame.DAQ,
                     icetray.I3Frame.Stream('S'),
                     icetray.I3Frame.Stream('M')])


class IC86(ICBase):

    def __init__(self):
        ICBase.__init__(self, "IC86:2012")

    def addparameter_main(self):
        self.AddParameter('SummaryFile','Summary filename', None)
        self.AddParameter('MCType','Generator particle type','corsika_weighted')
        self.AddParameter('UseLinearTree','Use I3LinearizedMCTree for serialization', False)
        self.AddParameter('MCPrescale','Prescale for keeping additional Monte Carlo info in the frame', 100)
        self.AddParameter('IceTop','Do IceTop Simulation?', False)
        self.AddParameter('Genie','Assume separate Genie MCPEs and BG MCPEs', False)
        self.AddParameter('FilterTrigger','filter untriggered events', True)
        self.AddParameter('Trigger','Run trigger simulation', True)
        self.AddParameter('LowMem','Low Memory mode', False)
        self.AddParameter('BeaconLaunches','Simulate beacon launches', True)
        self.AddParameter('TimeShiftSkipKeys','Skip keys in the triggersim TimeShifter', [])
        self.AddParameter('SampleEfficiency','Resample I3MCPESeriesMap for different efficiency', 0.0)
        self.AddParameter('GeneratedEfficiency','Generated efficiency for resampling', 0.0)
        self.AddParameter('RunID','Run ID', 0, explicit_type='int')

    def segment_main(self, tray, stats):

        # Instantiate a SummaryService if required
        summary = dataclasses.I3MapStringDouble()
        if self.summaryfile and os.path.exists(self.summaryfile):
              summary = ReadI3Summary(self.summaryfile)
        tray.context['I3SummaryService'] = summary

        tray.AddSegment(segments.DetectorSegment,"detector",
            gcdfile=self.gcdfile,
            mctype=self.mctype,
            uselineartree=self.uselineartree,
            detector_label=self.detector_label,
            runtrigger=self.trigger,
            filtertrigger=self.filtertrigger,
            stats=stats,
            icetop=self.icetop,
            genie=self.genie,
            prescale=self.mcprescale,
            lowmem=self.lowmem,
            BeaconLaunches=self.beaconlaunches,
            TimeShiftSkipKeys=self.timeshiftskipkeys,
            SampleEfficiency=self.sampleefficiency,
            GeneratedEfficiency=self.generatedefficiency,
            RunID=self.runid,
            KeepMCHits = not self.procnum % self.mcprescale,
            KeepPropagatedMCTree = not self.procnum % self.mcprescale,
            KeepMCPulses = not self.procnum % self.mcprescale
        )


class IC79(ICBase):

    def __init__(self):
        ICBase.__init__(self, "IC79")

    def addparameter_output(self):
        # overwriting default
        self.AddParameter('outputfile', 'Output filename', 'output.i3.gz')

    def segment_output(self, tray, stats):
        # custom output
        # Make P-frames for backwards compatibility
        tray.AddModule(PConverter, "fullevent")
        tray.AddModule("I3EventCounter", "counter")(
            ("physicscountername", "%s Triggered Events" % self.detector_label),
        )

        skipkeys = [ "I3Triggers",
                     "EnhancementFactor",
                     "MCPMTResponseMap",
                     "MCTimeIncEventID" ]

        if self.icetop:
             skipkeys += [ "IceTopRawData_unused",
                           "MCPMTResponseMap",
                           "MCTopHitSeriesMap" ]

        tray.AddModule("I3Writer","writer",
            filename=self.outputfile,
            Streams=[icetray.I3Frame.TrayInfo,
                     icetray.I3Frame.DAQ,
                     icetray.I3Frame.Stream('S'),
                     icetray.I3Frame.Stream('M')],
            SkipKeys=skipkeys)

    def addparameter_main(self):
        self.AddParameter('Summaryfile','Summary filename',None)
        self.AddParameter('IceTop','Enables IceTop Simulation', False)
        self.AddParameter('MCType','Generator particle type','CORSIKA')
        self.AddParameter('BeaconLaunches','Simulate beacon launches', True)

    def segment_main(self, tray, stats):
        # Instantiate a SummaryService if required
        summary = dataclasses.I3MapStringDouble()
        if self.summaryfile and os.path.exists(self.summaryfile):
           summary = ReadI3Summary(self.summaryfile)
        tray.context['I3SummaryService'] = summary


        tray.AddSegment(segments.DetectorSegment,"detector",
            detector_label=self.detector_label,
            mctype=self.mctype,
            runtrigger=True,
            filtertrigger=True,
            icetop=self.icetop,
            gcdfile=self.gcdfile,
            BeaconLaunches=self.beaconlaunches,
            lowmem=False,
        )

class IC59(IC79):
     def __init__(self):
        IC79.__init__(self)
        self.detector_label = "IC59"


class IceTop(ipmodule.ParsingModule):
  """
  Iceprod module to wrap iceprod.segments.IceTopSim. This is a convenience
  module that makes a simple simulation of the detector for IceTop only.
  """
  def __init__(self):
    ipmodule.ParsingModule.__init__(self)

    self.AddParameter('gcdfile','GeoCalibDetStatus filename','')
    self.AddParameter('inputfile','Input filename','')
    self.AddParameter('outputfile','Output filename','')
    self.AddParameter('SummaryFile','Summary filename', None)
    self.AddParameter('Seed','RNG Seed', 123)
    self.AddParameter('procnum','RNG stream number', 0)
    self.AddParameter('nproc','RNG number of streams', 1)
    self.AddParameter('DOMLauncher', 'Simulate with DOMLauncher', True)
    self.AddParameter('sim_trigger', 'Simulate trigger', False)
    self.AddParameter('calibrate', 'Calibrate and extract pulses (requires tpx module, which is in IceRec usually)', False)

  def Execute(self,stats):
    if not ipmodule.ParsingModule.Execute(self,stats):
      return 0

    from icecube import phys_services
    from icecube import sim_services
    from .. import segments

    tray = I3Tray()

    summary = dataclasses.I3MapStringDouble()
    if self.summaryfile and os.path.exists(self.summaryfile):
       summary = ReadI3Summary(self.summaryfile)
    tray.context['I3SummaryService'] = summary


    # set the random number generator
    rngstate    = "rng.state"
    if not os.path.exists(rngstate):
      rngstate = ''
      self.logger.warning("no RNG state found. Using seed instead.")

    tray.AddService("I3SPRNGRandomServiceFactory","random",
                    Seed = self.seed,
                    StreamNum = self.procnum,
                    NStreams = self.nproc,
                    instatefile = rngstate,
                    outstatefile = 'rng.state',
                    )

    tray.AddModule("I3Reader","reader",
                   filenamelist = [self.gcdfile, self.inputfile],
                   )

    # The main segment
    tray.AddSegment(segments.IceTopSim, 'IceTopSim',
                    sim_trigger = self.sim_trigger,
                    sim_new = self.domlauncher,
                    calibrate = self.calibrate,
                    gcd = self.gcdfile,
                    )

    tray.AddModule("I3Writer","writer",
                   filename = self.outputfile,
                   Streams=[icetray.I3Frame.TrayInfo,
                            icetray.I3Frame.DAQ,
                            icetray.I3Frame.Stream('S'),
                            icetray.I3Frame.Stream('M')],
                   )

    tray.Execute()

    summary = tray.context['I3SummaryService']
    for k in tray.Usage():
            stats[str(k.key())+":usr"] = k.data().usertime
            summary[str(k.key())+":usr"] = k.data().usertime

            stats[str(k.key())+":sys"] = k.data().systime
            summary[str(k.key())+":sys"] = k.data().systime

            stats[str(k.key())+":ncall"] = k.data().ncall
            summary[str(k.key())+":ncall"] = k.data().ncall

    if self.summaryfile:
       WriteI3Summary(summary, self.summaryfile)

    # Free memory
    del tray
    return 0

