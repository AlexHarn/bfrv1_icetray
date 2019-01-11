#!/usr/bin/env python

###
# A Class that can be used in union with MCHitSeperator to fill a set of pLots with interesting propperties regarding eventSplitting of single events
###

from icecube import icetray, dataclasses
from ROOT import gROOT, TCanvas, TH1D, TH2D, TFile

class histocounter(icetray.I3PacketModule):
  # Fills previous defined Histograms and plots them
  from icecube import icetray
  def __init__(self, ctx):
    icetray.I3PacketModule.__init__(self, ctx, icetray.I3Frame.DAQ)
    self.AddParameter("OutfileName", "Name to rootfile for output", "")
    self.AddParameter("SplitName", "Name of teh Splits", "toposplit")
    self.AddParameter("Source","Name to source the data originates (for weighting)", "DATA")
    self.AddOutBox("OutBox")
    
    self.histo_abs_noise = TH2D("TSNoise_abs","TS Noise pulses;nch_MC;nch_noise[]",100,0,100, 100, 0, 100)
    self.histo_abs_physics = TH2D("TSPhysics_abs","TS Physics pulses;nch_MC[];nch_pulses[]",100,0,100, 100, 0, 100)

    self.offhisto_abs_noise = TH2D("offlineNoise_abs","Offline Noise pulses;nch_MC[];nch_noise[]",100,0,100, 100, 0, 100)
    self.offhisto_abs_physics = TH2D("offlinePhysics_abs","Offline Physics pulses;nch_MC[];nch_pulses[]",100,0,100, 100, 0, 100)

    self.histo_abs_noise_weight = TH2D("TSNoise_abs_weight","TS Noise pulses;nch_MC_weight;nch_noise[]",100,0,100, 100, 0, 100)
    self.histo_abs_physics_weight = TH2D("TSPhysics_abs_weight","TS Physics pulses_weight;nch_MC[];nch_pulses[]",100,0,100, 100, 0, 100)

    self.offhisto_abs_noise_weight = TH2D("offlineNoise_abs_weight","Offline Noise pulses_weight;nch_MC[];nch_noise[]",100,0,100, 100, 0, 100)
    self.offhisto_abs_physics_weight = TH2D("offlinePhysics_abs_weight","Offline Physics pulses_weight;nch_MC[];nch_pulses[]",100,0,100, 100, 0, 100)

  def Configure(self):
    icetray.I3PacketModule.Configure(self)
    self.outfile = self.GetParameter("OutfileName")
    self.SplitName = self.GetParameter("SplitName")
    self.source = self.GetParameter("Source")
  def Finish(self):
    icetray.I3PacketModule.Finish(self)
    rootfile = TFile(self.outfile,"RECREATE")

    self.histo_abs_noise.Write()
    self.histo_abs_physics.Write()
    
    self.offhisto_abs_noise.Write()
    self.offhisto_abs_physics.Write()

    self.histo_abs_noise_weight.Write()
    self.histo_abs_physics_weight.Write()
    
    self.offhisto_abs_noise_weight.Write()
    self.offhisto_abs_physics_weight.Write()

    rootfile.Close()
  
  def FramePacket(self,frames):
    splitCount = 0
    reducedCount = 0
    multi = 0
    weight = 1
    for frame in frames:
      if frame.Stop == icetray.I3Frame.DAQ: #found DAQ
	if self.source == "DATA":
	  weight = 1.
    	if self.source == "CORSIKA":
	  if frame.Has("CorsikaWeightMap"):
	    weight = frame["CorsikaWeightMap"].Weight * ["CorsikaWeightMap"].Polygonato / ["CorsikaWeightMap"].TimeScale #/ (n_files)
	  else:
	    exit("can not locate 'CorsikaWeightMap'")
	elif  self.source == "NUGEN":
	  if frame.Has("CorsikaWeightMap"):
	    weight = frame["I3MCWeightDict"]["OneWeight"] #/ (n_files)
	  else:
	    exit("can not locate 'I3MCWeightDict'")
	elif self.source == "COINCNUGEN":
	  if frame.Has("CorsikaWeightMap"):
	    weight = frame["I3MCWeightDict"]["OneWeight"] #/ (n_files)
	  else:
	    exit("can not locate 'I3MCWeightDict'")
      
	#fill OfflinePulses
	nch = frame["particle_1_nch"].value
	nchnoise = frame["particle_0_nch"].value

	self.offhisto_abs_noise.Fill(nch, nchnoise)
	self.offhisto_abs_physics.Fill(nch, nch)

	self.offhisto_abs_noise_weight.Fill(nch, nchnoise, weight)
	self.offhisto_abs_physics_weight.Fill(nch, nch, weight)

      if frame.Stop == icetray.I3Frame.Physics:
	if frame.Has("I3EventHeader"):
	  if frame["I3EventHeader"].sub_event_stream == self.SplitName:
	    #fill noise
	    nch =  frame["FractionNCh"].value
	    nch_noise = frame["FractionNCh_Noise"].value
	    nch_matched = frame["FractionNCh_Matched"].value
	    nch_mc = frame["FractionNCh_MC"].value
	    weight = frame["I3MCWeightDict"]["OneWeight"] #/ (n_files)
	    #weight = frame["CorsikaWeightMap"].Weight * frame["CorsikaWeightMap"].Polygonato / (n_files *frame["CorsikaWeightMap"].TimeScale)

	    self.histo_abs_noise.Fill(nch_mc, nch_noise)
	    self.histo_abs_physics.Fill(nch_mc, nch_matched)
	    
	    self.histo_abs_noise_weight.Fill(nch_mc, nch_noise, weight)
	    self.histo_abs_physics_weight.Fill(nch_mc, nch_matched, weight)

      self.PushFrame(frame)
    return
