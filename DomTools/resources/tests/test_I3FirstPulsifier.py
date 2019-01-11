#!/usr/bin/env python

import unittest
import I3Tray
from icecube import icetray
from icecube import dataclasses as dc

def Create_PulseSeriesMap(frame):
    pulsesmap= dc.I3RecoPulseSeriesMap()

    p1=dc.I3RecoPulse()
    p1.charge=2.3
    p1.time=5.1
    p2=dc.I3RecoPulse()
    p2.charge=1.5
    p2.time=9.1
    pulsesmap[icetray.OMKey(10,4)]= dc.I3RecoPulseSeries([p1,p2])

    p1=dc.I3RecoPulse()
    p1.charge=4.5
    p1.time=43.1
    p2=dc.I3RecoPulse()
    p2.charge=0.9
    p2.time=1.2
    pulsesmap[icetray.OMKey(25,13)]= dc.I3RecoPulseSeries([p1,p2])
    
    frame.Put("TestRecoPulseSeriesMap",pulsesmap)


class I3FirstPulsifierTest(unittest.TestCase):
    def test_I3FirstPulsifierModule(self):
        pulsesmap= self.frame['TestFirstRecoPulseSeriesMap']
        firstpulse_inFirstKey= pulsesmap[icetray.OMKey(10,4)][0]#get an I3RecoPulseSeries with one element, the first one in time
        self.assertEqual(firstpulse_inFirstKey.time,5.1,
                         "I get wrong value for pulses = %d, expected is 5.1" % firstpulse_inFirstKey.time)
       
        firstpulse_inSecondKey= pulsesmap[icetray.OMKey(25,13)][0]#get an I3RecoPulseSeries with one element, the first one in time
        self.assertEqual(firstpulse_inSecondKey.time,1.2,
                         "I get wrong value for pulses = %d, expected is 5.1" % firstpulse_inSecondKey.time)

tray = I3Tray.I3Tray()
tray.Add("BottomlessSource")

tray.Add(Create_PulseSeriesMap,"Create_PulseSeriesMap")

from icecube import DomTools
tray.AddModule('I3FirstPulsifier','I3FirstPulsifier',
               InputPulseSeriesMapName="TestRecoPulseSeriesMap",
               OutputPulseSeriesMapName="TestFirstRecoPulseSeriesMap")

tray.AddModule(icetray.I3TestModuleFactory(I3FirstPulsifierTest),'testmodule')

tray.Execute(100)


