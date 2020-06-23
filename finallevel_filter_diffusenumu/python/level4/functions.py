from icecube import icetray, dataclasses, dataio

def removeBugEvents(frame):
    ''' At least in dataset 9095 their are events with a muon energy
        smaller then the neutrino rest mass. This somehow leads due to
        a bug to high-energy tracks from muons which should not create
        a visible track at all. This functions removes these events.
        http://lists.icecube.wisc.edu/pipermail/ice3sim/2013-July/007595.html '''

    # cut bug events
    tree=frame["I3MCTree"]

    # find the neutrino
    for primary in tree.primaries:
        if primary.type==primary.NuMu or primary.type==primary.NuMuBar:
            numu=primary
            break

    # find the stupid muon 
    daughters=tree.get_daughters(numu)
    for daughter in daughters:
        if daughter.type==daughter.MuPlus or daughter.type==daughter.MuMinus:
            muon=daughter
            break
        elif daughter.type==numu.type: # this is true for neutral currents
            return True

    # all muons with energies smaller than the muon restmass are affected
    if muon.energy <= 0.105658389*icetray.I3Units.GeV: # muon mass taken from ppc
        return False
    else:
        return True

from copy import copy
def selectIceCubeOnly(frame, InputPulses, OutputPulses):
    ''' Create a pulsemap where all DC strings are removed '''
    mask=copy(dataclasses.I3RecoPulseSeriesMapMask(frame, InputPulses))
    pulsemap=frame[InputPulses].apply(frame)
    for omkey in pulsemap.keys():
        if omkey.string in [79,80,81,82,83,84,85,86]:
            mask.set(omkey, False)
    frame[OutputPulses]=mask
