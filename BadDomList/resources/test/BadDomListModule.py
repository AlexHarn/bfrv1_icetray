#!/usr/bin/env python
import os
from os.path import expandvars
import tempfile
import shutil

from I3Tray import I3Tray
from icecube import dataio, icetray
from icecube.BadDomList.BadDomListModule import BadDomListModule
from icecube.BadDomList.IceTopBadDomListModule import IceTopBadDomListModule
from icecube.BadDomList.IceTopBadTankListModule import IceTopBadTankListModule

orig_dir = os.getcwd()
tmp_dir = tempfile.mkdtemp(dir=orig_dir)
os.chdir(tmp_dir)

try:
    # Configs for input/output parameters
    runId = 127797
    gcd = expandvars('$I3_TESTDATA/GCD/Level2_IC86.2015_data_Run00127797_21_242_GCD.i3.gz')
    outFile = os.path.join('./', os.path.basename(gcd))

    with open('./empty.127797.json', 'w') as f:
        pass
        
    with open('./trash.127797.json', 'w') as f:
        f.write('{"abc": 42}')

    i3live_responses = [
        {
            'I3LiveUrlRunDoms': expandvars('$I3_TESTDATA/i3live.run_doms.%s.json'), # Needs `%s` as placeholder for the run id, see BadDomListModule
            'I3LiveUrlSnapshotExport': expandvars('$I3_TESTDATA/i3live.snapshot-export.127797.json')
        },
        {
            'I3LiveUrlRunDoms': os.path.join(tmp_dir, 'trash.%s.json'), # Needs `%s` as placeholder for the run id, see BadDomListModule
            'I3LiveUrlSnapshotExport': os.path.join(tmp_dir, 'trash.127797.json')
        },
        {
            'I3LiveUrlRunDoms': os.path.join(tmp_dir, 'empty.%s.json'), # Needs `%s` as placeholder for the run id, see BadDomListModule
            'I3LiveUrlSnapshotExport': os.path.join(tmp_dir, 'empty.127797.json')
        }
    ]

    for i3live_response in i3live_responses:
        print(i3live_response)

        try:
            if not os.path.isfile(i3live_response['I3LiveUrlRunDoms'] % '127797') or not os.path.isfile(i3live_response['I3LiveUrlSnapshotExport']):
                print('i3live response json files are not available for this test:')
                print(' * %s' % i3live_response['I3LiveUrlRunDoms'] % '127797')
                print(' * %s' % i3live_response['I3LiveUrlSnapshotExport'])
                print('Stop test.')
                exit()
        
            i3live_response = {k: 'file:' + v for k, v in i3live_response.items()}
        
            # The tray...
            tray = I3Tray()
            tray.AddModule('I3Reader', 'reader', 
                           FilenameList = [gcd]
                          )
        
            # Rename old BadDomsList if already present
            tray.AddModule('Rename', 'CleanBadOMList',
                           Keys = [ 'BadDomsList', 'OldBadDomsList',
                                    'BadDomsListSLC', 'OldBadDomsListSLC',
                                    'IceTopBadDOMs','OldIceTopBadDOMs',
                                    'IceTopBadStations','OldIceTopBadStations',
                                    'IceTopBadTanks','OldIceTopBadTanks',
                                  ]
                          )
        
            # Add a new BadDomsList
            tray.AddModule(BadDomListModule, 'BadDoms',
                           RunId = runId,
                           ListName = "BadDomsList",
                           AddGoodSlcOnlyKeys  = True,
                           **i3live_response
                          )
        
            # Add the SLC bad dom list. It contains the same
            # bad doms except for the SLC only OM keys.
            tray.AddModule(BadDomListModule, 'BadDomsSLC',
                           RunId = runId,
                           ListName = "BadDomsListSLC",
                           AddGoodSlcOnlyKeys  = False,
                           **i3live_response
                          )
        
            # Add the bad dom lists for IceTop
            tray.AddModule(IceTopBadDomListModule, 'IceTopBadDoms',
                           BadDomListName = "BadDomsList",
                           ListName = "IceTopBadDOMs"
            )
        
            tray.AddModule(IceTopBadTankListModule, 'IceTopBadTanks',
                           IceTopBadDOMListName = "IceTopBadDOMs",
                           ListName = "IceTopBadTanks"
            )
        
            # Write the updated GCD
            tray.AddModule('I3Writer', 'writer',
                           Filename = outFile,
                           Streams = [icetray.I3Frame.Geometry,
                                      icetray.I3Frame.Calibration,
                                      icetray.I3Frame.DetectorStatus
                                     ]
                          )
        
            tray.Execute()
        except KeyError:
            pass
        except ValueError:
            pass
finally:
    os.chdir(orig_dir)
    shutil.rmtree(tmp_dir)
