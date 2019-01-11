from icecube.icetray import traysegment
from .BadDomListModule import BadDomListModule 
from .IceTopBadDomListModule import IceTopBadDomListModule
from .IceTopBadTankListModule import IceTopBadTankListModule

@traysegment
def BadDomList(Tray, Name, Simulation = False, RunId = 0, AddIceTopBadDoms = True, **kwargs):
    """
    Adds the bad dom list to the DetectorStatus frame.

    It renames old bad dom lists (`BadDomsList`, `BadDomsListSLC`, `IceTopBadDOMs`, `IceTopBadTanks`)
    to `Old*`. After that it utilizes the `BadDomListModule` to generate the `BadDomsList` and the `BadDomsListSLC`.
    The difference if the `BadDomsList` and `BadDomsListSLC` is that the `BadDomsList` has doms that are only in the SoftLC mode
    too. Those doms are NOT in the `BadDomsListSLC`.
    It also adds the bad dom lists for IceTop (`IceTopBadDOMs`, `IceTopBadTanks`) if `AddIceTopBadDoms` is `True` (default).

    Args:
        Tray (icetray.I3Tray): The tray
        Name (str): The name
        Simulation (bool): Indicates if the the list is used for data or simulation. If it is set to `True`, only disabled OM keys
                         are written into the bad dom list. The `runId` parameter will be ignored.
        RunId (int): The run id for the GCD file. Is required if `simulation = False`.
        AddIceTopBadDoms (bool): If it is set to `True`, the bad dom lists for IceTop will be added. Default is `True`.
        **kwargs: Those additional arguments will be passed to the BadDomListModule as well.
    """

    # Rename old BadDomsList if already present
    Tray.AddModule('Rename', 'CleanBadOMList',
                   Keys = [ 'BadDomsList', 'OldBadDomsList',
                            'BadDomsListSLC', 'OldBadDomsListSLC',
                            'IceTopBadDOMs','OldIceTopBadDOMs',
                            'IceTopBadTanks','OldIceTopBadTanks',
                          ]
    )

    Tray.AddModule(BadDomListModule, 'BadDoms',
        RunId = RunId,
        ListName = "BadDomsList",
        Simulation = Simulation,
        DisabledKeysOnly = Simulation,
        AddGoodSlcOnlyKeys  = True,
        **kwargs
    )

    Tray.AddModule(BadDomListModule, 'BadDomsSLC',
        RunId = RunId,
        ListName = "BadDomsListSLC",
        Simulation = Simulation,
        DisabledKeysOnly = Simulation,
        AddGoodSlcOnlyKeys  = False,
        **kwargs
    )

    if AddIceTopBadDoms:
        Tray.AddModule(IceTopBadDomListModule, 'IceTopBadDoms',
            BadDomListName = "BadDomsList",
            ListName = "IceTopBadDOMs"
        )

        Tray.AddModule(IceTopBadTankListModule, 'IceTopBadTanks',
            IceTopBadDOMListName = "IceTopBadDOMs",
            ListName = "IceTopBadTanks"
        )
