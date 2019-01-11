from icecube import icetray, dataclasses, dataio
from icecube.icetray import traysegment, load, I3Units, module_altconfig

@icetray.traysegment
def preparePulses(tray, name, Simulation=False, Qified=True,InIceCscd = lambda frame: True):

	from icecube import WaveCalibrator, wavedeform, DomTools
	import copy 

	tray.AddModule('Delete', 'DeleteErrata', Keys=['CalibrationErrata'])                                                                   
	kwargs = dict(Launches='InIceRawData', Waveforms='CalibratedWaveforms', Errata='CalibrationErrata')


	"""	
	if not Simulation:
               	tray.AddModule('I3WaveCalibrator', name+'wavecal', If=lambda frame: frame.Has('InIceRawData'), WaveformRange="CalibratedWaveformRange_new", **kwargs)
        
        else:
                tray.AddSegment(WaveCalibrator.DOMSimulatorCalibrator, name+'wavecal', If=lambda frame: frame.Has('InIceRawData'), WaveformRange="CalibratedWaveformRange_new", **kwargs)
        """
	kwargs = [dict(), dict(UseDOMsimulatorTemplates=True)][Simulation]

##---for monopod and credo
	tray.AddModule('I3PMTSaturationFlagger', 'SaturationWindows', If = lambda frame: frame.Has('CalibratedWaveforms') and InIceCscd, Output="SaturationWindows_new")  # run on CalibratedWavefroms
#	tray.AddModule(wavedeform.AddMissingTimeWindow, 'pulserange', Pulses='SRTOfflinePulses')        
	
	def clean_saturated_doms(frame):
                import numpy

                pulsemap = dataclasses.I3RecoPulseSeriesMap.from_frame(frame, 'SRTOfflinePulses')
                charges = numpy.array([sum([p.charge for p in pulses]) for pulses in pulsemap.itervalues()])
                qmean = charges.mean()
		mask = dataclasses.I3RecoPulseSeriesMapMask(frame, 'SRTOfflinePulses')

		if frame.Has('SaturationWindows'):
			baddies = list(frame['SaturationWindows'].keys())
			
                # for flasher events, also remove the flashing DOM
			if 'flasher' in frame:
				for info in frame['flasher']:
					baddies.append(info.flashing_om)

			clean_and_mask_doms(frame, baddies, 'SRTOfflinePulses', 'CleanSRTOfflinePulses')
		else:
			for om, pulses in pulsemap:
				mask.set(om, True)
			frame['CleanSRTOfflinePulses'] = mask
			ns = 0.
			nd = 0.
			frame['HowManySaturDOMs'] = dataclasses.I3Double(nd)
			frame['HowManySaturStrings'] = dataclasses.I3Double(ns)
			BadDoms_copy = dataclasses.I3VectorOMKey()
			for om in frame['BadDomsListSLC']:
				BadDoms_copy.append(om)
			frame.Put('BadDomsListSLCSaturated',BadDoms_copy)
	tray.AddModule(clean_saturated_doms, 'clippy_must_die', Streams=[icetray.I3Frame.Physics])


def clean_and_mask_doms(frame, keys, PulseName, MaskName):
        """ 
        Mask out DOMs from a pulse series *and* add them to the bad DOM list
        for proper phit/pnohit accounting.
        """
        pulsemap = dataclasses.I3RecoPulseSeriesMap.from_frame(frame, PulseName)
        mask = dataclasses.I3RecoPulseSeriesMapMask(frame, PulseName)

	Strings = []
	for om in pulsemap.keys():
		if not om.string in Strings:
			Strings.append(om.string)
		
        for om in keys:
                if om in pulsemap:
                        mask.set(om, False)

        qtot_clipped = sum([sum([p.charge for p in pulses]) for om, pulses in pulsemap if om in keys])
        qtot = sum([sum([p.charge for p in pulses]) for om, pulses in pulsemap])
        nstring = len(set([om.string for om in keys]))

	frame['HowManySaturDOMs'] = dataclasses.I3Double(len(set(keys)))
	frame['HowManySaturStrings'] = dataclasses.I3Double(nstring)

        NewBadDoms = dataclasses.I3VectorOMKey()
        for om in frame['BadDomsListSLC']:
                NewBadDoms.append(om)

        for om in keys:
                if not om in NewBadDoms:
                        NewBadDoms.append(om)

        frame.Put('BadDomsListSLCSaturated',NewBadDoms)
                                                                                                                                                            
        frame[MaskName] = mask
                                   
