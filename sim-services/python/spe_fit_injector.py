
import os
from icecube import icetray, dataclasses

def convert_omkey(key):
    try:
        string = int(key.split(',')[0])
        om = int(key.split(',')[1])
        omkey = icetray.OMKey(string, om)
        return omkey
    except ValueError:
        icetray.logging.log_warn("%s is not an OMKey" % str(key))

class SPEFitInjector:
    def __init__(self, filename):
        import json
        
        self.filename = filename
        if not os.path.exists(self.filename):
            icetray.logging.log_fatal("%s file does not exist." % self.filename)

        with open(self.filename) as f:
            self.fit_values = json.load(f)                

    def __repr__(self):
        return 'FitInjector(%s)' % self.filename
            
    def __call__(self, frame):

        cal = frame['I3Calibration']

        attributes = ['exp1_amp','exp1_width', 'exp2_amp', 'exp2_width',                      
                      'gaus_amp', 'gaus_mean', 'gaus_width', 'compensation_factor',
                      'slc_gaus_mean']
        
        for key, fits in self.fit_values.items():
            omkey = convert_omkey(key)
            if not omkey:
                continue

            spe_distribution = dataclasses.SPEChargeDistribution()
            for attr in attributes:
                if attr.startswith('slc_'):
                    setattr(spe_distribution, attr, fits['SLC_fit'][attr.replace('slc_','')])
                else:
                    setattr(spe_distribution, attr, fits['ATWD_fit'][attr])
            if omkey in cal.dom_cal:
                cal.dom_cal[omkey].combined_spe_charge_distribution = spe_distribution
            else:
                icetray.logging.log_warn("SPE Fit for %s has no calibration object." % str(omkey))
        del frame['I3Calibration']
        frame['I3Calibration'] = cal
        
class I3SPEFitInjector(icetray.I3Module):
    def __init__(self, context):
        icetray.I3Module.__init__(self, context)
        self.AddParameter("Filename", "Uncompressed JSON file with SPE fit data", "")
	    
    def Configure(self):
        self.spe_fit_injector = SPEFitInjector(self.GetParameter("Filename"))

    def Calibration(self, frame):
        self.spe_fit_injector(frame)
        self.PushFrame(frame)


