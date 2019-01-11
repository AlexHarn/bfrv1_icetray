#!/usr/bin/env python
from icecube import icetray, fill_ratio

def ensure(condition):
    if not condition:
        print("Test FAILED")
        assert()

class FRTestModule(icetray.I3Module):
    def __init__(self, context):
        icetray.I3Module.__init__(self, context)
        self.AddOutBox("OutBox")
    
    def Configure(self):
        pass
    
    def Physics(self, frame):
        if not 'FillRatioTest' in frame:
            print("FillRatioTest not in frame")
            assert()
        
        frInfo = frame['FillRatioTest']
        
        fillRatioFromRMS = frInfo.fill_ratio_from_rms
        fillRatioFromMean = frInfo.fill_ratio_from_mean
        fillRatioFromMeanRMS = frInfo.fill_ratio_from_mean_plus_rms
        fillRatioFromNCh = frInfo.fillratio_from_nch
        fillRatioFromEnergy = frInfo.fill_ratio_from_energy
        
        #The fill ratio must, by definition be between zero and and one, or
        #have a value of "-1", which is an error signal for when a vertex is 
        #not found.  
        
        ensure((fillRatioFromRMS <= 1 and fillRatioFromRMS >= 0) or (fillRatioFromRMS == -1) )
        ensure((fillRatioFromMean <= 1 and fillRatioFromMean >= 0) or (fillRatioFromMean == -1) )
        ensure((fillRatioFromMeanRMS <= 1 and fillRatioFromMeanRMS >= 0) or (fillRatioFromMeanRMS == -1) )
        ensure((fillRatioFromNCh <= 1 and fillRatioFromNCh >= 0) or (fillRatioFromNCh == -1) )
        ensure((fillRatioFromEnergy <= 1 and fillRatioFromEnergy >= 0) or (fillRatioFromEnergy == -1) )

        self.PushFrame(frame)

class FRLiteTestModule(icetray.I3Module):
    def __init__(self, context):
        icetray.I3Module.__init__(self, context)
        self.AddOutBox("OutBox")
    
    def Configure(self):
        pass
    
    def Physics(self, frame):
        if not "clast" in frame.keys(): self.PushFrame(frame)
        if not frame["clast"].fit_status == 0: self.PushFrame(frame)
        if not "MaskedOfflinePulses" in frame.keys(): self.PushFrame(frame)

        if not 'FillRatioLiteTest' in frame:
            print("FillRatioLiteTest not in frame")
            assert()
        
        fillRatio = frame['FillRatioLiteTest'].value
        
        #The fill ratio must, by definition be between zero and and one, or
        #have a value of "-1", which is an error signal for when a vertex is 
        #not found.  
        
        ensure((fillRatio <= 1 and fillRatio >= 0) or (fillRatio == -1) )
        self.PushFrame(frame)


class FRMapTestModule(icetray.I3Module):
    def __init__(self, context):
        icetray.I3Module.__init__(self, context)
        self.AddOutBox("OutBox")
    
    def Configure(self):
        pass
    
    def Physics(self, frame):
        if not 'FillRatioInfo' in frame:
            print("FillRatioInfo not in frame")
            assert()
        if not 'FillRatioMap' in frame:
            print("FillRatioMap not in frame")
            assert()
        
        frInfo = frame['FillRatioInfo']
        frMap = frame['FillRatioMap']
        
        fillRatioFromRMS = frInfo.fill_ratio_from_rms
        fillRatioFromMean = frInfo.fill_ratio_from_mean
        fillRatioFromMeanRMS = frInfo.fill_ratio_from_mean_plus_rms
        fillRatioFromNCh = frInfo.fillratio_from_nch
        fillRatioFromEnergy = frInfo.fill_ratio_from_energy

        # The results from the map and I3FillRatioInfo MUST match.
        
        ensure(fillRatioFromMean == frMap["FillRatioFromMean"])
        ensure(fillRatioFromRMS == frMap["FillRatio"])
        ensure(fillRatioFromMeanRMS == frMap["FillRatioFromMeanPlusRMS"])
        ensure(fillRatioFromNCh == frMap["FillRatioFromNCh"])
        ensure(fillRatioFromEnergy == frMap["FillRatioFromEnergy"])
        
        self.PushFrame(frame)

