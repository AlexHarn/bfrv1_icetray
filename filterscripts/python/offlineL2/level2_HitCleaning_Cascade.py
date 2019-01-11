from icecube import icetray, dataclasses
from icecube.icetray import I3Units

# load needed libs, the "False" suppresses any "Loading..." messages
icetray.load('double-muon', False) #! This has the pulse map splitter module

@icetray.traysegment
def CascadeHitCleaning(tray, name, Pulses = 'SplitInIcePulses', 
                       TWOfflinePulsesHLC = 'TWOfflinePulsesHLC',      
                       If = lambda f: True,
                       ):
    
    tray.AddModule( 'I3LCPulseCleaning', name + '_LCCleaning',
                    Input = 'SplitInIcePulses',
                    OutputHLC = 'OfflinePulsesHLC', # ! Name of HLC-only DOMLaunches
                    OutputSLC = 'OfflinePulsesSLC', # ! Name of the SLC-only DOMLaunches
                    If = If,
                    )

    tray.AddModule( 'I3TimeWindowCleaning<I3RecoPulse>', name + 'TWC_HLC',
                    InputResponse = 'OfflinePulsesHLC', # ! Use pulse series
                    OutputResponse = 'TWOfflinePulsesHLC', # ! Name of cleaned pulse series
                    TimeWindow = 6000 * I3Units.ns, # ! 6 usec time window
                    If = If,
                    )

