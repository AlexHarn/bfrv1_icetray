from icecube.icetray import I3Module, I3ConditionalModule, I3Frame
from icecube import dataclasses
from icecube.dataclasses import I3RecoPulseSeriesMapMask, I3EventHeader
from icecube.icetray.i3logging import log_info, log_warn, log_debug, log_trace, log_fatal


class I3IceTopSLCCalibrator(I3ConditionalModule):
    """
    I3Module to apply a correction to SLC charges.
    """
    def __init__(self, ctx):
        I3ConditionalModule.__init__(self, ctx)
        self.AddParameter("SLCPulses", "ID of input calibrated SLC pulse list (this gets replaced by default)",
                          "IceTopSLCVEMPulses")
        self.AddParameter("SLCPulsesOut", "ID of output calibrated SLC pulse list. If it is not specified, it is set equal to SLCPulses. If an object with this name is present, it will be replaced.", "")
        self.AddParameter("InputWaveforms", "Name of input I3WaveformSeriesMap (required only to know the ATWD channel)",
                          'CalibratedIceTopATWD_SLC');
        self.AddParameter("Config", "Configuration file with the parameters for each OM/chip/ATWD")
        self.AddOutBox("OutBox")

        self.geometry = None
        self.warned = []

    def Configure(self):
        import pickle # make python3 happy
        self.slc_name = self.GetParameter("SLCPulses")
        self.slc_name_out = self.GetParameter("SLCPulsesOut")
        if self.slc_name_out == "":
            self.slc_name_out = self.slc_name
        self.waveforms_name = self.GetParameter("InputWaveforms")
        f = open(self.GetParameter("Config"))
        self.parameters = pickle.load(f)
        f.close()

    def DAQ(self, frame):
        import math
        import scipy

        if not self.slc_name in frame:
            self.PushFrame(frame)
            return

        import copy
        pulses = copy.deepcopy(dataclasses.I3RecoPulseSeriesMap.from_frame(frame, self.slc_name))

        header = frame['I3EventHeader']

        # First check whether SLC parameters are defined.                                                                                                                                                   
        # We'll use OM 2,61 for this (1 was non-existing in IC79)                                                                                                                                           
        p=self.parameters[(2,61,0,0)]
        if (header.start_time.mod_julian_day_double<p[0][0] or header.start_time.mod_julian_day_double>p[0][-1]):
            log_fatal('Time of this event outside range containing SLC calibration constants.')

        if self.waveforms_name in frame:
            waveforms = frame[self.waveforms_name]
            for om in pulses.keys():
                if len(pulses[om]) != len(waveforms[om]):
                    raise Exception("Pulse series and waveform series have different sizes!")
                for i in range(len(pulses[om])):
                    if len(waveforms[om][i].waveform_information)>0:
                        atwd = waveforms[om][i].waveform_information[0].channel
                    else:
                        atwd = 0
                    chip = waveforms[om][i].source_index
                    k = (om.string, om.om, chip, atwd)
                    if not k in self.parameters.keys():
                        if  not (k, 'calib') in self.warned:
                            log_warn('Skipping %s chip %s channel %s! (missing SLC calibration info)'%(om, chip, atwd))
                            self.warned.append((k, 'calib'))
                        continue
                    p = self.parameters[k]
                    p1 = scipy.interp(header.start_time.mod_julian_day_double, p[0], p[1])
                    p2 = scipy.interp(header.start_time.mod_julian_day_double, p[0], p[2])
                    pulses[om][i].charge = p1 + p2*pulses[om][i].charge

        else:
            for om in pulses.keys():
                # Get the average p1 and p2 for all 3 channels and the two chips
                p1_avg=0
                p2_avg=0
                nChannelsChips=0
                for chip in range(0,2): 
                    for atwd in range(0,3):
                        k=(om.string, om.om, chip, atwd)
                        if not k in self.parameters.keys():
                            if  not (k, 'calib') in self.warned:
                                log_warn('Skipping %s chip %s channel %s! (missing SLC calibration info)'%(om, chip, atwd))
                                self.warned.append((k, 'calib'))
                        else:
                            nChannelsChips=nChannelsChips+1
                            p = self.parameters[k]
                            p1_avg = p1_avg+scipy.interp(header.start_time.mod_julian_day_double, p[0], p[1])
                            p2_avg = p2_avg+scipy.interp(header.start_time.mod_julian_day_double, p[0], p[2])
                p1=p1_avg/float(nChannelsChips)
                p2=p2_avg/float(nChannelsChips)
                for i in range(len(pulses[om])):
                    pulses[om][i].charge = p1 + p2*pulses[om][i].charge

        if self.slc_name_out in frame:
            del frame[self.slc_name_out]
        frame.Put(self.slc_name_out, pulses)
        self.PushFrame(frame)


