from I3Tray import *
from icecube import icetray, dataio, dataclasses
from icecube.icetray import traysegment

#####################################################
@traysegment
def multiCalculator(tray, name, pulses, InIceCscd = lambda frame: True):

    pulsesNoDC   = pulses+'_noDC' 
    pulsesDCOnly = pulses+'_DCOnly' 

    def IceCubeStripper(frame):
        if InIceCscd(frame) :
            try:
                mask = dataclasses.I3RecoPulseSeriesMapMask(frame, pulses)
            except:
                return
            rpsm = dataclasses.I3RecoPulseSeriesMap.from_frame(frame, pulses)
            for k in rpsm.iterkeys():
                if (k.string) < 79:
                    mask.set(k, False);
            frame[pulsesDCOnly] = mask

    def DeepCoreStripper(frame):
        if InIceCscd(frame) :
            try:
                mask = dataclasses.I3RecoPulseSeriesMapMask(frame, pulses)
            except:
                return
            rpsm = dataclasses.I3RecoPulseSeriesMap.from_frame(frame, pulses)
            for k in rpsm.iterkeys():
                if (k.string) >= 79:
                    mask.set(k, False);
            frame[pulsesNoDC] = mask

    def nchCalculation(frame, pulsename):
        if InIceCscd(frame) :
            if not pulsename in frame:
                return True
            nCh = 0
            pulsemap = dataclasses.I3RecoPulseSeriesMap.from_frame(frame, pulsename)
            nCh = len(pulsemap)
            frame['NCh_%s' % pulsename] = dataclasses.I3Double(nCh)

    def nstringCalculation(frame, pulsename):
        if InIceCscd(frame) :
            if not pulsename in frame:
                return True
            pulsemap = dataclasses.I3RecoPulseSeriesMap.from_frame(frame, pulsename)
            str=[entry.key().string for entry in pulsemap]
            ss=sorted(set(str))
            nString=len(ss)
            frame['NString_%s' % pulsename] = dataclasses.I3Double(nString)

    tray.AddModule(DeepCoreStripper,'DCStripper_%s' % pulses)
    tray.AddModule(IceCubeStripper,'ICStripper_%s'% pulses)

    tray.AddModule(nchCalculation,'NCh_%s' % pulses,pulsename=pulses)
    tray.AddModule(nchCalculation,'NCh_%s' % pulsesNoDC,pulsename=pulsesNoDC)
    tray.AddModule(nchCalculation,'NCh_%s' % pulsesDCOnly,pulsename=pulsesDCOnly)

    tray.AddModule(nstringCalculation,'Nstring_%s' % pulses,pulsename=pulses)
    tray.AddModule(nstringCalculation,'Nstring_%s' % pulsesNoDC,pulsename=pulsesNoDC)
    tray.AddModule(nstringCalculation,'Nstring_%s' % pulsesDCOnly,pulsename=pulsesDCOnly)

