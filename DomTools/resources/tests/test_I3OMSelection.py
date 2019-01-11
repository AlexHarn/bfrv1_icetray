#!/usr/bin/env python

from __future__ import print_function

import unittest, random

import I3Tray
from icecube import icetray
from icecube import dataclasses as dc

OMITTEDKEYS=[icetray.OMKey(12,1,0),icetray.OMKey(13,2,0)]
OUTMAPNAME='SelectedTestPulseSeriesMap'

class Create_PulsesMapAndGeometry(icetray.I3Module):
    def __init__(self, context):
        super(Create_PulsesMapAndGeometry, self).__init__(context)
        self.AddOutBox("OutBox")
        self.strings=[it+10 for it in range(5)]
        self.doms=[it+1 for it in range(3)]
        self.geometry = False
    def Configure(self):
        pass
    def Process(self):
        if not self.geometry:
            self.geometry = True
            geometry = dc.I3Geometry()
            for string in self.strings:
                for dom in self.doms:
                    omkey= icetray.OMKey(string,dom)
                    geometry.omgeo[omkey] = dc.I3OMGeo()
                    x=random.uniform(-500,500)
                    y=random.uniform(-500,500)
                    z=random.uniform(-300,300)
                    geometry.omgeo[omkey].position = dc.I3Position(x,y,z)

            frame = icetray.I3Frame(icetray.I3Frame.Geometry);
            frame.Put('I3Geometry',geometry)
            self.PushFrame(frame)

        pulsesmap= dc.I3RecoPulseSeriesMap()
        for string in self.strings:
            for dom in self.doms:
                omkey= icetray.OMKey(string,dom)
                pulse= dc.I3RecoPulse()
                pulse.charge= random.uniform(0.3,6.)#pulses are not used in the algorithm of this module, 
                                                    #just put a single pulse with any value of the charge
                pulsesmap[omkey]= dc.I3RecoPulseSeries([pulse])
    
        frame = icetray.I3Frame(icetray.I3Frame.Physics);
        frame.Put("TestPulseSeriesMap",pulsesmap)
        self.PushFrame(frame)


class I3OMSelectionTest(unittest.TestCase):
    def test_I3OMSelectionModule(self):
        self.assertTrue(self.frame.Has(OUTMAPNAME),
                        'I do not have %s '%OUTMAPNAME)

        cleanmap= self.frame[OUTMAPNAME]
        for omkey in OMITTEDKEYS:
            self.assertTrue(not cleanmap.has_key(omkey),
                            'The OMKey {0} should not be in {1}'.format(omkey,OUTMAPNAME))

        self.assertTrue(self.frame.Has('BadOMSelection'),
                        'I do not have BadOMSelection')

        self.assertEquals(len(self.frame['BadOMSelection']),len(OMITTEDKEYS),
                       'length of BadOMSelection is {0}, but should be {1}'.format(len(self.frame['BadOMSelection']),len(OMITTEDKEYS)))
                                                                      

tray = I3Tray.I3Tray()
icetray.set_log_level(icetray.I3LogLevel.LOG_TRACE)        

tray.Add(Create_PulsesMapAndGeometry,"Create_LaunchesSeriesMap")

from icecube import DomTools
tray.AddModule('I3OMSelection<I3RecoPulseSeries>','omselection',
               InputResponse="TestPulseSeriesMap",
               OutputResponse=OUTMAPNAME,
               OmittedKeys=OMITTEDKEYS,
               OutputOMSelection='BadOMSelection')#this is the default name to put into the frame, 
                                                  #all the bad OMKey's are stored in a I3Vector<OMKey>
              
tray.AddModule(icetray.I3TestModuleFactory(I3OMSelectionTest),'testmodule')

tray.Execute(10)


