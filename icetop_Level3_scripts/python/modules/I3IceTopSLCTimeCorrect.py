from icecube import icetray, dataclasses
import os, copy
import numpy as np

def correct_slc_time(mean_slc_charge,median_time_diff,slc_time,slc_charge):
    if slc_charge<=0:
        #do nothing
        #log_debug('SLC Charge less than zero')
        return slc_time

    xx = np.log10(slc_charge)
    if np.isnan(xx):
        #do nothing
        #log_warn('SLC Charge is nan')
        return slc_time

    yy = np.absolute(xx - mean_slc_charge)
    select = yy==np.amin(yy)
    correction = median_time_diff[select]

    correction = np.float(correction)
    corrected_slc_time=slc_time + correction

    return corrected_slc_time


class I3IceTopSLCTimeCorrect(icetray.I3ConditionalModule):
    """
    I3Module to apply a correction to SLC charges.
    """
    def __init__(self, ctx):
        icetray.I3ConditionalModule.__init__(self, ctx)
        self.AddParameter("SLCPulses", "ID of input charge calibrated SLC pulse list (this gets replaced by default)",
                          "IceTopSLCTankPulses")
        self.AddParameter("SLCPulsesOut", "ID of output calibrated SLC pulse list. If it is not specified, it is set equal to SLCPulses. If an object with this name is present, it will be replaced.", "")
        self.AddParameter('SLCTimeCorrectionPickle','Pickle that contains Time Correction Parametrization',
                          os.path.expandvars('${I3_BUILD}/icetop_Level3_scripts/resources/data/SLCTimeCorrection.pickle'))
        self.AddOutBox("OutBox")

    def Configure(self):
        import pickle # make python3 happy
        self.slc_name = self.GetParameter("SLCPulses")
        self.slc_name_out = self.GetParameter("SLCPulsesOut")
        self.slc_time_corr = self.GetParameter('SLCTimeCorrectionPickle')
        if self.slc_name_out == "":
            self.slc_name_out = self.slc_name

        f=open(self.slc_time_corr,'rb')
        self.mean_slc_charge = np.array( pickle.load(f,encoding='latin1') )
        self.median_time_diff = np.array( pickle.load(f,encoding='latin1') )
        # Not used
        # variance_time = np.array( pickle.load(f) )
        f.close()
        
    def DAQ(self, frame):
        if self.slc_name in frame:
            pulses =copy.deepcopy(dataclasses.I3RecoPulseSeriesMap.from_frame(frame, self.slc_name))

            for om in pulses.keys():
                for i in range(len(pulses[om])):
                    orig_time = pulses[om][i].time
                    orig_charge = pulses[om][i].charge
                    pulses[om][i].time = correct_slc_time(self.mean_slc_charge, self.median_time_diff, orig_time, orig_charge)

            if self.slc_name_out in frame:
                frame.Delete(self.slc_name_out)

            frame.Put(self.slc_name_out, pulses)

        self.PushFrame(frame)
        
