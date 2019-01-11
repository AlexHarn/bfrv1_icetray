#!/usr/bin/env python
from I3Tray import *
from icecube import icetray, dataio, dataclasses, DomTools, hdfwriter
import sys, numpy

load('VHESelfVeto')

pulses = 'OfflinePulses'

tray = I3Tray()
tray.AddModule('I3Reader', 'reader', FilenameList=sys.argv[2:])
tray.AddModule('HomogenizedQTot', 'qtot_total', Pulses=pulses)
tray.AddModule('I3LCPulseCleaning', 'cleaning', OutputHLC='HLCPulses',
    OutputSLC='', Input=pulses, If=lambda frame: 'HLCPulses' not in frame)
tray.AddModule(lambda fr: fr['QTot'].value > 1500, 'qtotcut')
tray.AddModule('VHESelfVeto', 'selfveto', TimeWindow=1500, VertexThreshold=50,
    VetoThreshold=3, Pulses='HLCPulses')
tray.AddModule('HomogenizedQTot', 'qtot_causal', Pulses=pulses,
    Output='CausalQTot', VertexTime='VHESelfVetoVertexTime')
tray.AddModule(lambda fr: fr['CausalQTot'].value > 1500, 'causalqtotcut')
vetopass = 0
vetopass = 0
vetofail = 0
vetoabsent = 0
def printstats(fr):
	global vetopass
	global vetofail
	global vetoabsent
	if not 'VHESelfVeto' in fr:
		vetoabsent += 1
	elif fr['VHESelfVeto'].value:
		vetofail += 1
	else:
		vetopass += 1
tray.AddModule(printstats, 'stats')
def qtot(fr):
	pulseseries = dataclasses.I3RecoPulseSeriesMap.from_frame(fr, pulses)
	charges = [p.charge for ps in pulseseries.values() for p in ps]
	print('Qtot: %f' % numpy.sum(charges))
tray.AddModule(qtot, 'qtot')
tray.AddSegment(hdfwriter.I3HDFWriter, 'hdf', Output=sys.argv[1]+'.h5', Keys=['I3EventHeader', 'VHESelfVeto', 'VHESelfVetoVertexTime', 'QTot', 'CausalQTot'], SubEventStreams=['in_ice'])
tray.AddModule(lambda fr: 'VHESelfVeto' in fr and fr['CausalQTot'].value > 6000 and not fr['VHESelfVeto'].value, 'i3filecut')
tray.AddModule('I3Writer', 'writer', Filename=sys.argv[1], DropOrphanStreams=[icetray.I3Frame.DAQ])


tray.Execute()


print('%d events total' % (vetoabsent + vetopass + vetofail))
print('%d events passed the veto' % vetopass)
print('%d events were vetoed' % vetofail)
print('%d events could not find a vertex' % vetoabsent)
