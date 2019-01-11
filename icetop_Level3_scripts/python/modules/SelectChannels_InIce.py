from icecube import icetray, dataclasses
#############
# nch cut
############
def SelectChannels_InIce(frame, InputIIpulses, NChThreshold, outputBoolName, RemoveEvents = True):
    nch = 0
    if InputIIpulses in frame:
        inIcePulses = frame[InputIIpulses]
        if inIcePulses.__class__ == dataclasses.I3RecoPulseSeriesMapMask or inIcePulses.__class__ == dataclasses.I3RecoPulseSeriesMapUnion:
            inIcePulses = inIcePulses.apply(frame)
        
        # only count HLC !!
        for om,pulses in inIcePulses:
            for pulse in pulses:
                #print pulse.flags, " LC? ", pulse.flags & dataclasses.I3RecoPulse.PulseFlags.LC
                ## use bit-wise operation in python to check if the last bit is 1 (LC)
                if (pulse.flags & dataclasses.I3RecoPulse.PulseFlags.LC) : 
                    nch += 1
                    break                    # only count one pulse per DOM if it isn't an SLC pulse (tested!)
    toCut = (nch > NChThreshold) 
    if not RemoveEvents :
        frame[outputBoolName] = icetray.I3Bool(toCut)
        return True
    else:
        return toCut
