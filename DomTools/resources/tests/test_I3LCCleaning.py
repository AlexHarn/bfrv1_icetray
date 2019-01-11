#!/usr/bin/env python

import unittest
import I3Tray
from icecube import icetray
from icecube import dataclasses as dc

def Create_LaunchesSeriesMap(frame):
    launchesmap= dc.I3DOMLaunchSeriesMap()

    l1=dc.I3DOMLaunch()
    l1.time=5.1
    l1.lc_bit=True
    l2=dc.I3DOMLaunch()
    l2.time=34.7
    l2.lc_bit=False
    launchesmap[icetray.OMKey(10,4)]= dc.I3DOMLaunchSeries([l1,l2])

    l1=dc.I3DOMLaunch()
    l1.time=6.0
    l1.lc_bit=False
    l2=dc.I3DOMLaunch()
    l2.time=11.3
    l2.lc_bit=False
    launchesmap[icetray.OMKey(25,13)]= dc.I3DOMLaunchSeries([l1,l2])
    
    frame.Put("TestLauncheSeriesMap",launchesmap)


class I3LCCleaningTest(unittest.TestCase):
    def test_I3LCCleaningModule(self):
        
        self.assertTrue(self.frame.Has('TestLauncheSeriesMapCleaning'),
                        'I do not have TestLauncheSeriesMapCleaning')
        self.assertTrue(self.frame.Has('TestLauncheSeriesMapCleaningSLC'),
                        'I do not have TestLauncheSeriesMapCleaningSLC')
        
        cleanmap= self.frame['TestLauncheSeriesMapCleaning']
        slcmap= self.frame['TestLauncheSeriesMapCleaningSLC']

        n_hlclaunches=0
        for launches in cleanmap.values(): 
            n_hlclaunches+=len([launche for launche in launches if launche.lc_bit])

        self.assertEqual(n_hlclaunches,1,
                         "I get wrong number of HLC = %d, expected is 1" % n_hlclaunches)

        n_slclaunches=0
        for launches in slcmap.values(): 
            n_slclaunches+=len(launches)

        self.assertEqual(n_slclaunches,3,
                         "I get wrong number of SLC = %d, expected is 3" % n_slclaunches)
        
tray = I3Tray.I3Tray()
tray.AddModule("BottomlessSource",'BottomlessSource',
               stream = icetray.I3Frame.DAQ)

tray.Add(Create_LaunchesSeriesMap,"Create_LaunchesSeriesMap",
         Streams=[icetray.I3Frame.DAQ])

from icecube import DomTools
tray.AddModule('I3LCCleaning','I3LCCleaning',
               InIceInput="TestLauncheSeriesMap",
               InIceOutput="TestLauncheSeriesMapCleaning",
               InIceOutputSLC="TestLauncheSeriesMapCleaningSLC")

tray.AddModule(icetray.I3TestModuleFactory(I3LCCleaningTest),'testmodule',
               Streams=[icetray.I3Frame.DAQ])

tray.Execute(100)


