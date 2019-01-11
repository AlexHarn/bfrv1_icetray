from I3Tray import *
from icecube import icetray, dataio, dataclasses
import sys, numpy

load('VHESelfVeto')

pulses = 'OfflinePulses'

tray = I3Tray()
tray.AddModule('I3Reader', 'reader', FilenameList=sys.argv[2:])
def qtotcut(fr):
	pulseseries = dataclasses.I3RecoPulseSeriesMap.from_frame(fr, pulses)
	charges = [p.charge for ps in pulseseries.values() for p in ps]
	return numpy.sum(charges) > 6000
tray.AddModule(qtotcut, 'qtotcut')
tray.AddModule('VHESelfVeto', 'selfveto', Pulses=pulses)
vetopass = 0
vetofail = 0
vetoabsent = 0
def printstats(fr):
	global vetopass
	global vetofail
	global vetoabsent
	if not 'VHESelfVeto' in fr:
		vetoabsent += 1
		return False
	if fr['VHESelfVeto'].value:
		vetofail += 1
		return False
	else:
		vetopass += 1
		return True
tray.AddModule(printstats, 'stats')
def qtot(fr):
	pulseseries = dataclasses.I3RecoPulseSeriesMap.from_frame(fr, pulses)
	charges = [p.charge for ps in pulseseries.values() for p in ps]
	print('Qtot: %f' % numpy.sum(charges))
tray.AddModule(qtot, 'qtot')
	
tray.AddModule('I3Writer', 'writer', Filename=sys.argv[1], DropOrphanStreams=[icetray.I3Frame.DAQ])


tray.Execute()


print('%d events total' % (vetoabsent + vetopass + vetofail))
print('%d events passed the veto' % vetopass)
print('%d events were vetoed' % vetofail)
print('%d events could not find a vertex' % vetoabsent)
