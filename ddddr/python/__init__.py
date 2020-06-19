from icecube.load_pybindings import load_pybindings
from icecube.icetray import traysegment
import icecube.dataclasses # be nice and pull in our dependencies

load_pybindings(__name__,__path__)

try:
    import icecube.tableio
    from . import converters
except ImportError:
    pass

@traysegment
def TrueAndRecoMuonEnergy(tray, name, 
                          BinWidth=50, I3MCTree='I3MCTree',
                          MMCTrackList='MMCTrackList', InputPulses='TWOfflinePulsesHLC',
                          MaxImpact=100, Prefix='I3MuonEnergy',
                          SaveDomResults=False):
    """This segment uses the I3TrueMuonEnergy module and the I3MuonEnergy module to compare
       the true and the reconstructed energy loss along the true Monte Carlo track."""
    
    tray.AddModule('I3TrueMuonEnergy', name+'_true',
                   BinWidth = BinWidth,
                   I3MCTree   = I3MCTree,
                   MMCTrackList      = MMCTrackList,
                   Prefix  = Prefix+'True',
                   SaveEnergyLosses = SaveDomResults,
                  )

    tray.AddModule('I3MuonEnergy', name+'me_reco',
                   BinWidth           = BinWidth,
                   InputPulses        = InputPulses,
                   MaxImpact          = MaxImpact,
                   UseMonteCarloTrack = True,
                   I3MCTree           = I3MCTree,
                   MMCTrackList       = MMCTrackList,
                   Prefix             = Prefix,
                   SaveDomResults     = SaveDomResults,
                  )

del load_pybindings
del icecube
#import sys
#from icecube import icetray
#
##try:
##    icetray.load('libddddr') #, False)
##except RuntimeError, err:
##    sys.stderr.write("ERROR: Could not load libddddr (%s)." % err)
##
#del icetray
#del sys
