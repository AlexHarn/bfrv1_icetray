from icecube.icetray import I3Units, load, traysegment, module_altconfig
from icecube.icetray import I3ConditionalModule, I3Frame
from icecube.phys_services import I3Splitter
from icecube.dataclasses import I3RecoPulse, I3RecoPulseSeriesMap, I3RecoPulseSeriesMapMask

# HoboTimeSplitter is Jakob's IC79 L3 class to split an event based on charge weighted mean time. 
# Uses HLC pulses only for getting the mean time but splits the entire uncleaned pulse series 

class HoboTimeSplitter(I3ConditionalModule):
	"""
	Split an event into halves on the charge-weighted mean time.
	"""
	def __init__(self, ctx):
		I3ConditionalModule.__init__(self, ctx)
		self.AddOutBox("OutBox")
		self.AddParameter("Pulses", "Pulses to split", "")
		self.AddParameter("Output", "Name of the mask in the output P frames", "")
		
	def Configure(self):
		self.splittable_pulses = self.GetParameter("Pulses")
		self.output_mask = self.GetParameter("Output")
		
	def maskem(self, frame, f):
		mask = I3RecoPulseSeriesMapMask(frame, self.splittable_pulses)
		pulsemap = I3RecoPulseSeriesMap.from_frame(frame, self.splittable_pulses)
		for om, pulses in pulsemap.iteritems():
			for p in pulses:
				mask.set(om, p, f(p))
		return mask
		
	def Physics(self, frame):
		refpulse_map = I3RecoPulseSeriesMap.from_frame(frame, self.splittable_pulses)
		w = 0.
		t = 0.
		LC = I3RecoPulse.PulseFlags.LC
		for pulses in refpulse_map.itervalues():
			w += sum([p.charge for p in pulses if p.flags & LC])
			t += sum([p.time*p.charge for p in pulses if p.flags & LC])
		tmean = t/w;
		
		frame[self.output_mask + '_0'] = self.maskem(frame, lambda p: p.time <= tmean)
		frame[self.output_mask + '_1'] = self.maskem(frame, lambda p: p.time > tmean)
		
		self.PushFrame(frame)
