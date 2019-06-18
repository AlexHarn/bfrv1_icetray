#!/usr/bin/env python
import os
from os.path import expandvars
import tempfile
import shutil

from I3Tray import I3Tray
from icecube import dataio, icetray
from icecube.BadDomList.BadDomListTraySegment import BadDomList

orig_dir = os.getcwd()
tmp_dir = tempfile.mkdtemp(dir=orig_dir)
os.chdir(tmp_dir)

try:

    # Configs for input/output parameters
    runId = 127797
    gcd = expandvars('$I3_TESTDATA/GCD/Level2_IC86.2015_data_Run00127797_21_242_GCD.i3.gz')
    outFile = os.path.join('./', "BDLTEST_%s_" + os.path.basename(gcd))

    i3live_response = {
        'I3LiveUrlRunDoms': expandvars('$I3_TESTDATA/i3live.run_doms.%s.json'), # Needs `%s` as placeholder for the run id, see BadDomListModule
        'I3LiveUrlSnapshotExport': expandvars('$I3_TESTDATA/i3live.snapshot-export.127797.json')
    }

    if not os.path.isfile(i3live_response['I3LiveUrlRunDoms'] % '127797') or not os.path.isfile(i3live_response['I3LiveUrlSnapshotExport']):
        print('i3live response json files are not available for this test:')
        print(' * %s' % i3live_response['I3LiveUrlRunDoms'] % '127797')
        print(' * %s' % i3live_response['I3LiveUrlSnapshotExport'])
        print('Stop test.')
        exit()

    i3live_response = {k: 'file:' + v for k, v in i3live_response.items()}

    # The tray...
    # ---------------------- EXPERIMENTAL DATA -----------------------------------------
    # Test for experimental data (parameter simulation = False)
    tray = I3Tray()
    tray.AddModule('I3Reader', 'reader', 
                   FilenameList = [gcd]
                  )

    # Add the bad dom list tray segment
    # Since it is a GCD file for experimental data, it has
    # a run number. Let the `Simulation` flag be `False` (default)
    # and pass teh run number. Now it gets the information
    # form I3Live.
    tray.AddSegment(BadDomList, 'BadDomList',
                    RunId = runId,
                    **i3live_response
    )

    # Write the updated GCD
    tray.AddModule('I3Writer', 'writer',
                   Filename = outFile % 'EXP',
                   Streams = [icetray.I3Frame.Geometry,
                              icetray.I3Frame.Calibration,
                              icetray.I3Frame.DetectorStatus
                             ]
                  )

    tray.Execute()
    del tray

    # ---------------------- SIMULATED DATA --------------------------------------------
    # Test for simulated data (parameter simulation = False)
    # It doesn't matter that the input GCD file is for experimental data
    tray = I3Tray()
    tray.AddModule('I3Reader', 'reader', 
                   FilenameList = [gcd]
    )

    # Add the bad dom list tray segment
    # If the GCD file doesn't have a run number, e.g. for simulated data,
    # set the `Simulation` flag to `True`. This is important since the source
    # of data is different.
    tray.AddSegment(BadDomList, 'BadDomList',
                    Simulation = True
    )

    # Write the updated GCD
    tray.AddModule('I3Writer', 'writer',
                   Filename = outFile % 'SIM',
                   Streams = [icetray.I3Frame.Geometry,
                              icetray.I3Frame.Calibration,
                              icetray.I3Frame.DetectorStatus
                             ]
                  )

    tray.Execute()

finally:
    os.chdir(orig_dir)
    shutil.rmtree(tmp_dir)
