"""
IceProd Module for Simple Muon Production
"""

import os,sys
from os.path import expandvars
import logging

from I3Tray import I3Units, I3Tray
from icecube import icetray, dataio, dataclasses

from .. import ipmodule
from ..util import ReadI3Summary, WriteI3Summary

class StartingMuon(ipmodule.ParsingModule):
   '''
   Injects a muon at the center of the detector.
   '''
   
   def __init__(self):
      ipmodule.ParsingModule.__init__(self)
      
      self.AddParameter('outputfile','Output filename','')
      self.AddParameter("seed","RNG seed",0)
      self.AddParameter("procnum","RNG stream number",0)
      self.AddParameter("nproc","Number of RNG streams",1)
      self.AddParameter('nevents','Number of events',0)
      self.AddParameter('FromEnergy','Minimum energy',1.*I3Units.TeV)
      self.AddParameter('ToEnergy','Maximum energy',10.*I3Units.PeV)
      self.AddParameter('PROPOSALParams','any other parameters for proposal',dict())
      self.AddParameter('HistogramFilename', 'Histogram filename.', None)
      
   def Execute(self,stats):
      if not ipmodule.ParsingModule.Execute(self,stats):
         return 0

      import random
      from math import pi

      # Load libraries 
      from icecube import PROPOSAL, cmc, phys_services
      from .. import segments 

      random.seed(self.seed)
      
      # support json ParamsMap, so we can get our dict from the iceprod config file
      try:
         import json
      except ImportError:
         json = None
         
      if isinstance(self.proposalparams,str):
         if not json:
            raise Exception('ProposalParams provided as string, but python does not understand json')
         self.proposalparams = json.loads(self.proposalparams) 	
 
      # Instantiate a tray 
      tray = I3Tray()
      
      randomServiceForPropagators = phys_services.I3SPRNGRandomService(
         seed = self.seed,
         nstreams = self.nproc*2,
         streamnum = self.nproc + self.procnum)

      tray.context['I3PropagatorRandomService'] = randomServiceForPropagators
      
      tray.AddModule("I3InfiniteSource",
                     Stream=icetray.I3Frame.DAQ)

      def Generator(frame, FromEnergy = 1*I3Units.TeV, ToEnergy = 1*I3Units.TeV):
         p = dataclasses.I3Particle()
         p.energy = random.uniform(FromEnergy, ToEnergy)
         p.pos = dataclasses.I3Position(0,0,0)
         
         zenith = random.uniform(0., pi)
         azimuth = random.uniform(0., 2*pi)
         p.dir = dataclasses.I3Direction(zenith, azimuth)
         p.length = 500 * I3Units.m
         p.type = dataclasses.I3Particle.ParticleType.MuMinus
         p.location_type = dataclasses.I3Particle.LocationType.InIce
         p.time = 0. * I3Units.ns
         
         tree = dataclasses.I3MCTree()
         tree.add_primary(p)
                       
         frame["I3MCTree_preMuonProp"] = tree
         
      tray.Add(Generator,
               FromEnergy = self.fromenergy,
               ToEnergy = self.toenergy,
               Streams = [icetray.I3Frame.DAQ]
      )
        
      tray.Add(segments.PropagateMuons, 
               RandomService= randomServiceForPropagators,
               **self.proposalparams
      ) 
        
      if self.histogramfilename:         
         from icecube.production_histograms import ProductionHistogramModule
         from icecube.production_histograms.histogram_modules.simulation.mctree_primary import I3MCTreePrimaryModule
         from icecube.production_histograms.histogram_modules.simulation.mctree import I3MCTreeModule
        
         tray.AddModule(ProductionHistogramModule, 
                        Histograms = [I3MCTreePrimaryModule, I3MCTreeModule],
                        OutputFilename = self.histogramfilename)

      tray.Add("I3Writer", 
               filename=self.outputfile,
               Streams=[icetray.I3Frame.TrayInfo,
                        icetray.I3Frame.DAQ,
                        icetray.I3Frame.Stream('S'),
                        icetray.I3Frame.Stream('M')])

      # Execute the Tray
      tray.Execute(self.nevents)
      
      # Free memory
      del tray
      return 0


