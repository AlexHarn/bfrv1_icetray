'''
This small piece of code is here used after seededRT cleaning, since right now SeededRT cannot create masks yet. 
(This feature will probably be added to the code later)
It needs the original pulse(Mask) as input, together with the cleaned pulses and the name of the mask.
'''

def makePulseMask(frame,PrimaryPulsesName,SecondaryPulsesName,SecondaryMaskName):
    from icecube import dataclasses, icetray
    if PrimaryPulsesName in frame and SecondaryPulsesName in frame:
        if PrimaryPulsesName.__class__==dataclasses.I3RecoPulseSeriesMapMask:
            secondary_mask = dataclasses.I3RecoPulseSeriesMapMask(frame, frame[PrimaryPulsesName].source)
        else:
            secondary_mask=dataclasses.I3RecoPulseSeriesMapMask(frame,PrimaryPulsesName)
        secondary_mask.set_none()
        secondaryPulses=frame[SecondaryPulsesName]
        for key in secondaryPulses.keys():
            for p in secondaryPulses[key]:
                secondary_mask.set(key, p, True)
        frame.Delete(SecondaryPulsesName)
        frame[SecondaryMaskName]=secondary_mask
