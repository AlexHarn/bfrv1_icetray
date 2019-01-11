#!/usr/bin/env python

"""
A very useful script for visualization purposes (use for example on FixedRate-trigger frames):
from a sequence of P frames trailing a single Q frame the specified Fit and Pulse objects
are taken and catangated into a new object reciding in the sentinell Q-frame.
"""

from icecube import icetray, dataclasses
from copy import deepcopy

#______________________________________________________________
class Smash(icetray.I3PacketModule):
	def __init__(self, ctx):
		icetray.I3PacketModule.__init__(self, ctx, icetray.I3Frame.DAQ)
		self.recoMapName = ""
		self.AddParameter("recoMapName","The name of the first input particle frame object.",self.recoMapName)
		self.splitName = ""
		self.AddParameter("SplitName","The name of the SubEventStream.",self.splitName)
		self.fitName = ""
		self.AddParameter("FitName","The name of the Fit", self.fitName)
		self.pulseSourceName = "UncleanedInIcePulses"
		self.AddParameter("PulseSourceName", "The Name of the lowest unobstructed map of I3RecoPulses", self.pulseSourceName)
		self.outputPrefix="coll"
		self.AddParameter("OutputPrefix", "A prefix to the output objects", self.outputPrefix)
		self.AddOutBox("OutBox")

	def Configure(self):
		self.recoMapName = self.GetParameter("RecoMapName")
		self.splitName = self.GetParameter("SplitName")
		self.fitName = self.GetParameter("FitName")
		self.pulseSourceName = self.GetParameter("PulseSourceName")
		self.outputPrefix = self.GetParameter("OutputPrefix")
		if (self.recoMapName=="" or self.splitName=="" or self.fitName==""):
			icetray.logging.log_fatal("Specify all parameters: RecoMapName, FitName, SplitName")
	def FramePacket(self,frames):
		partvec = dataclasses.I3VectorI3Particle()
		mask = dataclasses.I3RecoPulseSeriesMapMask(frames[0], self.pulseSourceName, dataclasses.I3RecoPulseSeriesMap())
		for frame in frames:
			if (frame.Stop == icetray.I3Frame.Physics):
				if (frame["I3EventHeader"].sub_event_stream == self.splitName):

					primary = deepcopy(frame[self.fitName])
					primary.type = dataclasses.I3Particle.MuMinus
					primary.shape = dataclasses.I3Particle.ContainedTrack
					primary.length = 500 #part.speed*(tr.start-tr.stop)
					partvec.append(frame[self.fitName])
					mask = mask | frame[self.recoMapName]

					frame.Put("Primary", primary)
		frames[0].Put(self.outputPrefix+self.recoMapName, mask)
		frames[0].Put(self.outputPrefix+self.fitName, partvec)

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

  tray.AddModule("Delete", "delete",
    Keys= ["collLineFit_Masked", "collMaskedOfflinePulses"])

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

  