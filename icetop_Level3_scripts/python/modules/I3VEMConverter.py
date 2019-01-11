from icecube import dataclasses
from icecube.icetray import I3ConditionalModule, I3Frame, I3Units
from icecube.dataclasses import I3WaveformSeriesMap


class I3VEMConverter(I3ConditionalModule):
    """I3Module to calibrate waveforms. This module replicates the
    calibration applied in tpx/I3TopHLCPulseExtractor, which does not
    add the VEM-calibrated waveform to the frame.
    """
    def __init__(self, ctx):
        I3ConditionalModule.__init__(self, ctx)
        self.AddParameter('Waveforms', 'Waveforms before VEM calibration', 'IceTopCalibratedWaveforms')
        self.AddParameter('VEMWaveforms', 'VEM-calibrated waveforms', 'IceTopVEMCalibratedWaveforms')
        self.AddOutBox("OutBox")

    def Configure(self):
        self.vem_waveforms_ = self.GetParameter('VEMWaveforms')
        self.pe_waveforms_ = self.GetParameter('Waveforms')

    def DAQ(self, frame):
        import numpy
        import copy
        if not 'I3Calibration' in frame:
            raise Exception("I3Calibration is not in the frame. I can't calibrate! Did you pass a GCD file?")
        if not 'I3DetectorStatus' in frame:
            raise Exception("I3DetectorStatus is not in the frame. I can't calibrate! Did you pass a GCD file?")

        status = frame['I3DetectorStatus']
        calibration = frame['I3Calibration']

        if not self.pe_waveforms_ in frame:
            self.PushFrame(frame)
            return

        waveforms = frame[self.pe_waveforms_]
        vem_waveforms = copy.deepcopy(waveforms)

        for omKey in waveforms.keys():
            if omKey in calibration.vem_cal and omKey in calibration.dom_cal and omKey in status.dom_status.keys():
                vemcal = calibration.vem_cal[omKey]
                domcal = calibration.dom_cal[omKey]
                dom_status = status.dom_status[omKey]
                pe_per_vem = vemcal.pe_per_vem/vemcal.corr_factor
                spe_mean = dataclasses.spe_mean(dom_status, domcal)
                scale = 1./spe_mean/domcal.front_end_impedance/pe_per_vem
            else:
                log_error('OM %s present in waveform keys but not in I3Calibration and I3DetectorStatus! Charge will be NAN'%omKey)
                scale = numpy.nan
            for launch in range(len(waveforms[omKey])):
                w_in = waveforms[omKey][launch].waveform
                w_out = vem_waveforms[omKey][launch].waveform
                for i in range(len(w_in)):
                    w_out[i] = w_in[i]*waveforms[omKey][launch].bin_width*scale

        frame.Put(self.vem_waveforms_, vem_waveforms)
        self.PushFrame(frame)
