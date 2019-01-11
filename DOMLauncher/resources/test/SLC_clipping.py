#!/usr/bin/env python

from I3Tray import *
from icecube import icetray, dataclasses, dataio, phys_services, simclasses, DOMLauncher
import unittest

tray = I3Tray()
i3_testdata = os.path.expandvars("$I3_TESTDATA")
tray.AddModule("I3InfiniteSource","FrameMaker",Prefix = i3_testdata + '/sim/GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz')

nLaunches=0
nBig=0

class SLC_check(unittest.TestCase):

	def __init__(self,dummy):
		super(SLC_check,self).__init__(dummy)
	
	def testSLC(self):
		random_service = phys_services.I3GSLRandomService(1337)
		key = OMKey(40,40)
		cal = self.frame["I3Calibration"].dom_cal
		stat = self.frame["I3DetectorStatus"].dom_status
		
		for charge in range(1,160):
			charge=charge/2.
			for i in range(0,30):
				dom = DOMLauncher.I3InIceDOM(random_service, key)
				dom.configure(cal[key],stat[key])
				pulses = simclasses.I3MCPulseSeries()
				pulse = simclasses.I3MCPulse()
				pulse.charge = charge
				pulse.time = 0
				pulses.append(pulse)
				
				dcstream = DOMLauncher.DCStream()
				dom.discriminator(pulses,dcstream)
				dcstream = sorted(dcstream, key = lambda t: t.time)
				for trigger in dcstream:
					dom.add_trigger(trigger)
				dom.trigger_launch(True)
				launches = dom.get_domlaunches()
				
				for launch in launches:
					self.assert_(launch.lc_bit==False,"All launches should be SLC")
					global nLaunches
					nLaunches+=1
					stamp = launch.raw_charge_stamp
					any_big = False
					all_even = True
					for sample in stamp:
						if sample>=512:
							any_big = True
						if sample%2==1:
							all_even = False
					if(any_big):
						global nBig
						nBig+=1
					if any_big and not all_even:
						print("Bad SLC readout:",stamp)
					self.assert_(not any_big or (any_big and all_even),"All samples must be small, or all must be even")

tray.AddModule(icetray.I3TestModuleFactory(SLC_check),'testy',Streams = [icetray.I3Frame.DAQ])
tray.Execute(3 + 1)
print("nLaunches =",nLaunches)
print("nBig =",nBig)
