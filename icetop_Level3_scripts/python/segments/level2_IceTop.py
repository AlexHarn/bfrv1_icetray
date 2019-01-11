from icecube import icetray, dataclasses
from icecube import toprec
from icecube.icetop_Level3_scripts.icetop_globals import icetop_bad_tanks as BadTanks
    
#
# Module configurations. Could be moved to toprec if required
#

IceTopPlaneDefault = icetray.module_altconfig('I3TopRecoPlane',
    EventHeaderName = 'I3EventHeader', # Default
    Trigger = 3,            # Default
    Verbose = False,        # Default
    )

IceTopCOGDefault = icetray.module_altconfig('I3TopRecoCore',
    Weighting_Power = 0.5,  # Default
    Verbose = False,        # Default
    NTanks = 7,             # ! Use 7 largest pulses only
    )

IceTopLateralFitDefault = icetray.module_altconfig('I3TopLateralFit',
    Trigger = 5,            # ! At least 5 stations when combined fit
    ResidualCut = 3000.0,   # Default
    AsciiFile = '',         # Default
    CoreCut = 11.0,         # Default
    Ldf = 'dlp',            # Default
    Curvature = 'gausspar', # ! Fit a curved showerfront
    CutBad = 0,             # Default
    CutBadThreshold = 3.0,  # Default
    Verbose = False         # Default
    )

IceTopLateralFitSmall = icetray.module_altconfig('I3TopLateralFit',
    Trigger = 3,            # ! At least 3 stations
    ResidualCut = 3000.0,   # Default
    AsciiFile = '',         # Default
    Corecut = 11.0,         # Default
    Ldf = 'dlp',            # Default
    Curvature = '',         # ! No Curvature for events with less than 5 stations
    Cutbad = 0,             # Default
    Cutbadthreshold = 3.0,  # Default
    Verbose = False         # Default
    )

@icetray.traysegment
def OfflineIceTopReco(tray, name, If = lambda f: True,
                      Pulses = '',
                      Excluded = BadTanks,
                      SnowFactor = 2.1,
                      Tag = '',
                      Detector='IC86.2013'
    ):
    """
    Adds the following IceTop reconstructions to the frame:
    - Center-of-gravity calculation, output defined by parameter ShowerCOG;
    - Plane fit, output defined by ShowerPlane;
    - Lateral fit in two configurations, one with combined direction / lateral function fit, one without.
     Output defined by ShowerCombined and SmallShowerCombined. Uses bad station list defined in BadStations.
     This should be the output ExcludedStations list of I3TopHLCCluster cleaning, or whatever is used
     for splitting IceTop events.
    """

    if Tag !='':
        Tag="_"+Tag
    ShowerCOG = 'ShowerCOG'+Tag
    ShowerPlane = 'ShowerPlane'+Tag
    Laputop = 'Laputop'+Tag
    LaputopSmall = 'LaputopSmall'+Tag

    tray.AddSegment(IceTopCOGDefault, name + '_COG',
                    DataReadout = Pulses,
                    ShowerCore = ShowerCOG,
                    If = If)

    tray.AddSegment(IceTopPlaneDefault, name + '_Plane',
                    DataReadout = Pulses,
                    ShowerPlane = ShowerPlane,
                    If = If)
    
    tray.AddSegment(toprec.LaputopStandard, Laputop,
                    pulses=Pulses,
                    excluded=Excluded,
                    snowfactor=SnowFactor,
                    ShowerCOGSeed=ShowerCOG,
                    ShowerPlaneSeed=ShowerPlane,
                    If=If)
    
    tray.AddSegment(toprec.LaputopSmallShower, LaputopSmall,
                    pulses=Pulses,
                    excluded=Excluded,
                    snowfactor=SnowFactor,
                    ShowerCOGSeed=ShowerCOG,
                    ShowerPlaneSeed=ShowerPlane,
                    If = If
                    )

@icetray.traysegment
def RemoveOldLevel2(tray, name, Tag = ''):

    ShowerCOG = Tag + 'ShowerCOG'
    ShowerPlane = Tag + 'ShowerPlane'
    ShowerCombined = Tag + 'ShowerCombined'
    SmallShowerCombined = Tag + 'SmallShowerCombined'
    SmallShowerDecision = Tag + 'IsSmallShower'
    SmallShowerNStations = Tag + 'SmallShowerNStations'
    Laputop = Tag + 'LaputopStandard'
    LaputopSmall = Tag + 'LaputopSmallShower'
    Laputop2 = Tag + 'Laputop'
    LaputopSmall2 = Tag + 'LaputopSmall'

    tray.AddModule("Delete", name+"prune_IceTop",
                   keys = [ShowerCOG,
                           ShowerCombined, ShowerCombined + 'Params',
                           SmallShowerCombined, SmallShowerCombined + 'Params',
                           ShowerPlane, ShowerPlane + 'Params',
                           Laputop, Laputop + 'Params',Laputop2, Laputop2+'Params',
                           LaputopSmall, LaputopSmall + 'Params', LaputopSmall2, LaputopSmall2+'Params',
			   Laputop2, Laputop2 + 'Params',
			   LaputopSmall2, LaputopSmall2 + 'Params']
                   )
