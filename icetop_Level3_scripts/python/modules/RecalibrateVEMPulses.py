from icecube import dataclasses, icetray

class RecalibrateVEMPulses(icetray.I3ConditionalModule):
    
    def __init__(self, ctx):
        super(RecalibrateVEMPulses, self).__init__(ctx)
        self.AddParameter('InputPulsesQ', 'L2 calibrated pulses in DAQ',[])
        self.AddParameter('InputPulsesP', 'L2 calibrated pulses in Physics',[])
        self.AddOutBox('OutBox')
    
    def Configure(self):
        self.inputQ  = self.GetParameter('InputPulsesQ')
        self.inputP  = self.GetParameter('InputPulsesP')
        self.pe_per_vem_l2=None
        self.corr_factor_l2=None
        self.vemcal_l3=None
    
    def Calibration(self,frame):
        if ('IceTopLevel2PEperVEM' in frame and 'IceTopLevel2CorrFactor' in frame):
            self.pe_per_vem_l2 = frame['IceTopLevel2PEperVEM']
            self.corr_factor_l2 = frame['IceTopLevel2CorrFactor']
        else:
            icetray.logging.log_fatal('No L2 VEMCal constants found.')
        if 'I3Calibration' in frame:
            self.vemcal_l3 = frame['I3Calibration'].vem_cal
        else: 
            icetray.logging.log_fatal('No I3Calibration found.')
        self.PushFrame(frame)

    def DAQ(self, frame):
        for pulses_name in self.inputQ:
            if not frame.Has(pulses_name):
                icetray.logging.log_fatal('{0} not in frame'.format(pulses_name))  #This should not happen.                 

            pulses_l2 = dataclasses.I3RecoPulseSeriesMap.from_frame(frame, pulses_name)
            pulses_l3 = dataclasses.I3RecoPulseSeriesMap()
        
            for key, ps in pulses_l2:
                for p in ps:
                    pulse = dataclasses.I3RecoPulse(p)
                    pulse.charge = pulse.charge * (self.pe_per_vem_l2[key] / self.corr_factor_l2[key]) / (self.vemcal_l3[key].pe_per_vem / self.vemcal_l3[key].corr_factor)
                    # Do we need to check for NaNs here?
                    if not key in pulses_l3.keys():
                        pulses_l3[key] = dataclasses.I3RecoPulseSeries()
                    pulses_l3[key].append(pulse)

            frame.Delete(pulses_name)
            frame.Put(pulses_name, pulses_l3)
        self.PushFrame(frame)


    def Physics(self, frame):
        for pulses_name in self.inputP:
            if not frame.Has(pulses_name):
                icetray.logging.log_warn('{0} not in frame'.format(pulses_name))
                
            pulses_l2 = dataclasses.I3RecoPulseSeriesMap.from_frame(frame, pulses_name)
            pulses_l3 = dataclasses.I3RecoPulseSeriesMap()
        
            for key, ps in pulses_l2:
                for p in ps:
                    pulse = dataclasses.I3RecoPulse(p)
                    pulse.charge = pulse.charge * (self.pe_per_vem_l2[key] / self.corr_factor_l2[key]) / (self.vemcal_l3[key].pe_per_vem / self.vemcal_l3[key].corr_factor)
                    if not key in pulses_l3.keys():
                        pulses_l3[key] = dataclasses.I3RecoPulseSeries()
                    pulses_l3[key].append(pulse)

            frame.Delete(pulses_name)
            frame.Put(pulses_name, pulses_l3)
        self.PushFrame(frame)
