#!/usr/bin/env python

"""
A very useful script for visualization purposes (use for example on FixedRate-trigger frames):
from a sequence of P frames trailing a single Q frame the specified Fit and Pulse objects
are taken and catangated into a new object reciding in the sentinell Q-frame.
"""

from icecube import icetray, dataclasses
from copy import deepcopy
import numpy

#______________________________________________________________
class Smash(icetray.I3PacketModule):
	def __init__(self, ctx):
		icetray.I3PacketModule.__init__(self, ctx, icetray.I3Frame.DAQ)
		self.recoMapName = "MaskedOfflinePulses"
		self.AddParameter("recoMapName","The name of the first input particle frame object.",self.recoMapName)
		self.splitName = ""
		self.AddParameter("SplitName","The name of the TTriggerModule.",self.splitName)
		self.fitName = "LineFit_Masked"
		self.AddParameter("FitName","A list of fits", self.fitName)
		self.pulseSourceName = "UncleanedInIcePulses" #HACK HARDCODING
		self.AddOutBox("OutBox")

	def Configure(self):
		self.recoMapName = self.GetParameter("RecoMapName")
		self.splitName = self.GetParameter("SplitName")
		self.fitName = self.GetParameter("FitName")

	def FramePacket(self,frames):
		mctree = dataclasses.I3MCTree()
		mask = dataclasses.I3RecoPulseSeriesMapMask(frames[0], self.pulseSourceName, dataclasses.I3RecoPulseSeriesMap())
		for frame in frames:
			if (frame.Stop == icetray.I3Frame.Physics=:
				if (frame["I3EventHeader"].sub_event_stream == self.splitName):
					smallmctree = dataclasses.I3MCTree()
					#need to have the time range
					if (not frame.Has(self.recoMapName+"TimeRange")):
						time_pulses = []
						pulses = dataclasses.I3RecoPulseSeriesMap.from_frame(frame, self.recoMapName)
						for item in pulses:
							for pulse in item[1]:
								time_pulses.append(pulse.time)
						time_pulses = numpy.array(time_pulses)
						frame.Put(self.recoMapName+"TimeRange", dataclasses.I3TimeWindow(time_pulses.min(), time_pulses.max()))
					#now do the composition work
					tr = frame[self.recoMapName+"TimeRange"]
					part = frame[self.fitName]
					primary = deepcopy(frame[self.fitName])
					inparticle = deepcopy(frame[self.fitName])
					outparticle = deepcopy(frame[self.fitName])
					#primary.pos = part.shift_along_track((tr.start-part.time)*part.speed)
					primary.type = dataclasses.I3Particle.NuMuBar #NuMuBar
					primary.shape = dataclasses.I3Particle.Primary #InfiniteTrack #ContainedTrack #StoppingTrack
					#primary.time = tr.start
					primary.energy = 1
					#primary.length = part.speed*(tr.start-tr.stop)
					mctree.add_primary(primary)
					smallmctree.add_primary(primary)
					inparticle.type = dataclasses.I3Particle.MuMinus
					inparticle.shape = dataclasses.I3Particle.ContainedTrack
					inparticle.energy = 1
					inparticle.length = part.speed*(tr.start-tr.stop)
					inparticle.time = tr.start
					#inparticle.pos = part.shift_along_track((tr.start-part.time)*part.speed)
					mctree.append_child(primary, inparticle)
					smallmctree.append_child(primary, inparticle)
					mask = mask | frame[self.recoMapName]

					frame.Put("smallTree"+self.fitName, smallmctree)
		frames[0].Put("coll"+self.recoMapName, mask)
		frames[0].Put("coll"+self.fitName, mctree)

		for frame in frames:
			self.PushFrame(frame)

#___________________IF STANDALONE__________________________
if (__name__=='__main__'):
  from optparse import OptionParser

  usage = 'usage: %prog [options]'
  parser = OptionParser(usage)

  parser.add_option("-i", "--input", action="store", type="string", default="", dest="INPUT", help="Input i3 file to process")
  parser.add_option("-o", "--output", action="store", type="string", default="", dest="OUTPUT", help="Output i3 file")
  parser.add_option("-g", "--gcd", action="store", type="string", default="", dest="GCD", help="GCD file for input i3 file")
  parser.add_option("-n", "--nevents", action="store", type="int", default=0, dest="NEVENTS", help="Number of Events to process")

  (options,args) = parser.parse_args()

  from icecube import icetray, dataio
  from I3Tray import *
  tray = I3Tray()

  tray.AddModule("I3Reader","reader",
    filenamelist = [options.GCD, options.INPUT],)

  tray.AddModule(Smash, "HULK_smash",
    RecoMapName = "MaskedOfflinePulses",
    SplitName = "Resplit",
    FitName = "LineFit_Masked")

  tray.AddModule("I3Writer","writer",
    streams = [icetray.I3Frame.DAQ,icetray.I3Frame.Physics],
    filename = options.OUTPUT)

  

  if (options.NEVENTS==0):
    tray.Execute()
  else:
    tray.Execute(options.NEVENTS)

  