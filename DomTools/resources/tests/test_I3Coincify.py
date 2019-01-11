#!/usr/bin/env python

import unittest
import I3Tray
from icecube import icetray
from icecube import dataclasses as dc

def Create_PulseSeriesMap(frame):
    pulsesmap= dc.I3RecoPulseSeriesMap()

    p1=dc.I3RecoPulse()
    p1.charge=2.3
    p1.time=1.0*icetray.I3Units.ns
    p1.width=.0*icetray.I3Units.ns
    p2=dc.I3RecoPulse()
    p2.charge=1.5
    p2.time=2.*icetray.I3Units.ns
    p2.width=.0*icetray.I3Units.ns
    pulsesmap[icetray.OMKey(10,4)]= dc.I3RecoPulseSeries([p1,p2])

    p1=dc.I3RecoPulse()
    p1.charge=4.5
    p1.time=3.0*icetray.I3Units.ns
    p1.width=.0*icetray.I3Units.ns
    p2=dc.I3RecoPulse()
    p2.charge=0.9
    p2.time=802.*icetray.I3Units.ns #will find two pulses (in OMKey(10,4)) that are not in the window of 800 ns
    p2.width=.0*icetray.I3Units.ns
    pulsesmap[icetray.OMKey(10,5)]= dc.I3RecoPulseSeries([p1,p2])
    
    frame.Put("TestRecoPulseSeriesMap",pulsesmap)


class I3ConcifyTest(unittest.TestCase):
    def test_I3ConcifyModule(self):
        pulsesmap1= self.frame['TestConcifyRecoPulseSeriesMap']
        for pulses in pulsesmap1.values():
            for p in pulses:
                self.failIfEqual(p.time,802.,
                                 "There is a pulse with time = %d that exceeds the window of 800ns respect to its neighbors" 
                                 %p.time) 
                    

tray = I3Tray.I3Tray()
icetray.set_log_level(icetray.I3LogLevel.LOG_DEBUG)

tray.AddModule("BottomlessSource",'BottomlessSource')

tray.Add(Create_PulseSeriesMap,"Create_PulseSeriesMap")

from icecube import DomTools
tray.AddModule('I3Coincify<I3RecoPulse>','I3Coincify',
               InputName="TestRecoPulseSeriesMap",
               OutputName="TestConcifyRecoPulseSeriesMap",
               IsolatedResponseName="IsolatedResponse",
               CoincidenceWindow=800.0*icetray.I3Units.ns)#default

tray.AddModule(icetray.I3TestModuleFactory(I3ConcifyTest),'testmodule')

tray.Execute(10)


