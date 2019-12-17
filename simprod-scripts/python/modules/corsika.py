#!/usr/bin/env python
################################ Tray 0 ##############################
#   
#   Code automatically generated by iceprod translator 
#   on 2009-07-21T16:33:23 
#   
######################################################################
import os, sys, time
from os.path import expandvars
import logging

from I3Tray import *
from .. import ipmodule
from ..util import ReadI3Summary, WriteI3Summary
# Instantiate parameters expression parser 


def SetAtmosphereWeight(frame, atmosphere = 13):
      if frame.Has("I3CorsikaInfo"):
         info = frame["I3CorsikaInfo"]   
         info.atmosphere = atmosphere
         frame.Delete("I3CorsikaInfo")
         frame.Put("I3CorsikaInfo",info)      
      if frame.Has("CorsikaWeightMap"):
         weightmap = frame["CorsikaWeightMap"]
         weightmap["Atmosphere"] = atmosphere
         frame.Delete("CorsikaWeightMap")
         frame.Put("CorsikaWeightMap",weightmap)
      return True

def weighted_choice(weights,rng):
    total     = sum(weights)
    intervals = [ sum(weights[0:i]) for i in range(len(weights)+1) ]
    r = rng.uniform(0,total)
    for i in range(len(intervals)):
        if r >= intervals[i] and r < intervals[i+1]:
           return i
    return 0
    


def CorsikaReaderTraySegment(tray,name,
      gcdfile, randomService,
      cors, oversampling, legacyoversampling=False,
      stats={}, 
      propagate_muons=False, 
      select_neutrino=False, 
      filenamelist=[],
      SimulateIceTop=True):

      """
      Tray Segment for basic CORSIKA simulation (generation)
      """

      # Load libraries 
      from icecube import icetray, dataclasses, interfaces, phys_services
      from icecube import simclasses, sim_services, corsika_reader, PROPOSAL 
      from ..weights import Corsika5CompWeightModule, CorsikaWeightModule, PolygonatoWeightModule
      from .. import segments

      if len(filenamelist) > 0: 
         inputfiles = filenamelist
      else:
         inputfiles = [cors.outputpath]
      logging.debug("CorsikaReader reading: %s"%",".join(inputfiles))
      logging.debug("CorsikaReader %u showers: "%cors.nevents)

      tray.AddSegment(segments.GenerateAirShowers,"ReadAirShowers",
         Files= inputfiles,
         GCDFile=gcdfile,
         NEvents=cors.nevents,
         OverSampling=oversampling,
         LegacyOverSampling=legacyoversampling,
         CylinderRadius = 500, CylinderHeight = 1000,
         SimulateIceTop = SimulateIceTop,
         TankResponse="param",
         KeepMCHits=False,
         RandomService="I3RandomService")


      if select_neutrino:
         tray.AddSegment(segments.SelectNeutrino, "SelectNeutrino",
              EnergyBiasPower = 1, 
              CylinderRadius = 500, 
              CylinderHeight = 1000,
              )
	
      if cors.ranpri == 3: # 5-component case

        tray.AddModule(Corsika5CompWeightModule,"5compCorsikaweight",
          name                       = "CorsikaWeightMap",
          nevents                    = cors.nevents,
          spric                      = cors.spric is True or cors.spric == 'T',
          ThetaMin                   = cors.cthmin*I3Units.degree,
          ThetaMax                   = cors.cthmax*I3Units.degree,
          cylinderLength             = cors.length*I3Units.meter,
          cylinderRadius             = cors.radius*I3Units.meter,
          energyprimarymin           = cors.emin*I3Units.GeV,
          energyprimarymax           = cors.emax*I3Units.GeV,

          PrimaryNormalizationH      =  cors.pnormh,
          PrimaryNormalizationHe     =  cors.pnormhe,
          PrimaryNormalizationCNO    =  cors.pnormn,
          PrimaryNormalizationMgAlSi =  cors.pnormal,
          PrimaryNormalizationFe     =  cors.pnormfe,

          PrimarySpectralIndexH      = -cors.pgamh,
          PrimarySpectralIndexHe     = -cors.pgamhe,
          PrimarySpectralIndexCNO    = -cors.pgamn,
          PrimarySpectralIndexMgAlSi = -cors.pgamal,
          PrimarySpectralIndexFe     = -cors.pgamfe,

          OverSamplingFactor         = oversampling
        )
        tray.AddModule(PolygonatoWeightModule,"polygonato")

      else:  # Standard CORSIKA
        mctree = "I3MCTree"
        if oversampling < 1:
               mctree = "I3MCTree_preSampling"
        tray.AddModule(CorsikaWeightModule,"corsikaweight",
        spric            = cors.spric is True or cors.spric == 'T',
        nevents          = cors.nevents,
        ThetaMin         = cors.cthmin*I3Units.degree,
        ThetaMax         = cors.cthmax*I3Units.degree,
        cylinderLength   = cors.length*I3Units.meter,
        cylinderRadius   = cors.radius*I3Units.meter,
        OverSamplingFactor = oversampling,
        MCTreeName = mctree
        )

      tray.AddModule(SetAtmosphereWeight,"atmo", atmosphere = cors.atmod,
          Streams = [icetray.I3Frame.Simulation,icetray.I3Frame.DAQ] ) 


def configure_corsika(params):
    """
    Configure corsika from paramters
    """
    from . import dcorsika

    atmod = params.atmospheres[params.procnum % len(params.atmospheres)]

    if params.corsikaname == 'corsika':
        cors = dcorsika.Corsika()
    elif params.corsikaname == 'dcorsika':
        cors = dcorsika.dCorsika()
    elif params.corsikaname == 'jcorsika':
        cors = dcorsika.Corsika()
        cors.name = 'jcorsika'
        cors.arrang = 0
        cors.nuaddi = True
    else:
        raise Exception('bad corsikaName')

    cutoff_type  = params.cutofftype
    if cutoff_type == "EnergyPerNucleon" : spric = True
    elif cutoff_type == "EnergyPerParticle" : spric = False
    else: raise Exception("Undefined CutoffType %s" % cutoff_type)

    cors.version  = str(params.corsikaversion)
    cors.platform = ''
    if cors.version.endswith('5comp'):
       cors.platform = os.uname()[4]

    if params.locut:
       cors.locut = "T %f" % params.locut
    else:
       cors.locut = "F 0"
    if params.kcut:
       cors.kcut    = params.kcut
    cors.ecuts1  = params.ecuts1
    cors.ecuts2  = params.ecuts2
    cors.ecuts3  = params.ecuts3
    cors.ecuts4  = params.ecuts4
    cors.emin    = params.eprimarymin
    cors.emax    = params.eprimarymax
    cors.crtype  = params.crtype

    cors.cthmin  = params.cthmin
    cors.cthmax  = params.cthmax
    cors.seed    = params.corsikaseed
    cors.radius  = params.radius
    cors.length  = params.length
    cors.atmod   = atmod
    cors.runnum  = params.runnum
    cors.cache   = 1
    cors.nevents = params.nshowers
    cors.dslope  = params.dslope
    cors.eslope  = params.eslope
    cors.ranpri  = params.ranpri
    cors.donkg   = 0
    cors.doegs   = 0

    cors.outfile = "DAT%06u" % cors.runnum
    if params.compress:
       cors.outfile += '.gz'

    cors.outdir  = "./" 
    cors.topdir  = expandvars("$PWD/")
    cors.tmpdir  = os.path.join(cors.topdir,"dcors%d"%cors.runnum)
    cors.outputpath  = os.path.join(cors.tmpdir,cors.outfile)
    cors.logfile = os.path.join(os.getcwd(),"CORSIKA000000.%s.log" % params.crtype)
    cors.url     = params.repourl
    cors.cvmfs   = params.cvmfs
    cors.depth   = 1950.
    cors.model   = params.model
    cors.lemodel = params.lemodel
    cors.spric   = spric
    cors.f2k     = 'T'
    cors.compress = params.compress
    cors.skipoptions  = params.skipoptions
    cors.usepipe  = params.usepipe

    if params.ranpri == 3:
       if len(params.pnorm) != 5:
          raise Exception("corsika: list of pnorm indices is the wrong size: %s!!!!" % len(params.pnorm))
       if len(params.pgam) != 5:
          raise Exception("corsika: list of pgam indices is the wrong size: %s!!!!" % len(params.pgam))

       pnorm = float(sum(params.pnorm))
       cors.pnormh  = params.pnorm[0]/pnorm
       cors.pnormhe = params.pnorm[1]/pnorm
       cors.pnormn  = params.pnorm[2]/pnorm
       cors.pnormal = params.pnorm[3]/pnorm
       cors.pnormfe = params.pnorm[4]/pnorm

       cors.pgamh   = -params.pgam[0]
       cors.pgamhe  = -params.pgam[1]
       cors.pgamn   = -params.pgam[2]
       cors.pgamal  = -params.pgam[3]
       cors.pgamfe  = -params.pgam[4]

    return cors


from .. import ipmodule

class CorsikaGenerator(ipmodule.ParsingModule):
   """
   Wrapper class that runs CORSIKA
   """
   def __init__(self):
        ipmodule.ParsingModule.__init__(self)

        self.AddParameter('nshowers','Number of generated CR showers',1)
        self.AddParameter('procnum','process number',0)
        self.AddParameter('seed','RNG seed',1)
        self.AddParameter('nproc','Number of processes for (RNG)',1)
        self.AddParameter('gcdfile','GeoCalibDetStatus filename','')
        self.AddParameter('outputfile','Output filename','corsika.i3')
        self.AddParameter('inputfile',"Input filename (only if you are not generating file)",'')
        self.AddParameter('RunCorsika','Run CORSIKA or only generate INPUTS file',True)
        self.AddParameter('RunNum','Run Number',0)
        self.AddParameter('summaryfile','JSON Summary filename','summary.json')
        self.AddParameter('atmospheres','Atmospheric models',[11,12,13,14]) # mar, jul, oct, dec
        self.AddParameter('eslope','CR spectral index (only if ranpri=0)',-2.7)
        self.AddParameter('ranpri','CR spectrum: 0=individual-nuclei, 1=Wiebel-Sooth, 2=Hoerandel, 3=5-component',2)
        self.AddParameter('crtype','CR Particle Type (only if not dcorsika)',0)
        self.AddParameter('pnorm','5-component relative contribution H,He,N,Al,Fe',[10.,5.,3.,2.,1.])
        self.AddParameter('pgam','5-component spectral indices H,He,N,Al,Fe',[2.0,2.0,2.0,2.0,2.0])
        self.AddParameter("locut","Enables skew angle cutfoff",1.58)
        self.AddParameter("kcut","minimum neutrino energy required to keep the shower",0.0)

        self.AddParameter('CORSIKAseed','CORSIKA seed',1)
        self.AddParameter("dslope","Change in spectral index",0)
        self.AddParameter("eprimarymax",'CR max energy',1.0e5)
        self.AddParameter("eprimarymin","CR min energy",600.0)
        self.AddParameter("fluxsum","", 0.131475115)
        self.AddParameter("length","",1600.0)
        self.AddParameter("OverSampling","Number of times to sample each shower",1)
        self.AddParameter("LegacyOverSampling","Legacy mode for oversmapling ",False)
        self.AddParameter("radius","",800.0)
        
        self.AddParameter("model","corsika model","sibyll")
        self.AddParameter("lemodel","corsika low-energy model","gheisha")
        self.AddParameter('corsikaVersion','version of corsika to run','v6900')
        self.AddParameter("corsikaName","Corsika binary name","corsika")

        self.AddParameter('cthmin','Min theta of injected cosmic rays',0.0)  
        self.AddParameter('cthmax','Max theta of injected cosmic rays',89.99)  
  
        self.AddParameter('ecuts1','hadron min energy (see corsika docs)',273.)  
        self.AddParameter('ecuts2','muon min energy (see corsika docs)',273.)  
        self.AddParameter('ecuts3','electron min energy (see corsika docs)',0.003)  
        self.AddParameter('ecuts4','photon min energy (see corsika docs)',0.003)  
        self.AddParameter("CutoffType","Sets SPRIC=T (EnergyPerNucleon) or F (EnergyPerParticle) ","EnergyPerNucleon")
        self.AddParameter("UpperCutoffType","Upper cutoff type (defaults to CutoffType)",None)
        self.AddParameter("RepoURL","URL of repository containing corsika tarballs","http://prod-exe.icecube.wisc.edu")
        self.AddParameter('CVMFS', 'Path to CVMFS repository', '/cvmfs/icecube.opensciencegrid.org/') 
        self.AddParameter('SimulateIceTop','Simulate IceTop detector',False)
        self.AddParameter('SelectNeutrino','Randomly select CORSIKA neutrino and force interaction',False)
        self.AddParameter('UsePipe', 'Use pipe for corsika output', False)

        self.AddParameter('compress','compress corsika output',False)
        self.AddParameter('skipoptions','Options to skip',[])

        self.AddParameter('HistogramFilename', 'Histogram filename.', None)
        self.AddParameter('EnableHistogram', 'Write a SanityChecker histogram file.', False)
        self.AddParameter("UseGSLRNG","Use I3GSLRandomService",False) 

   def Execute(self,stats):
        self.stats=stats
        if not ipmodule.ParsingModule.Execute(self,self.stats): return 0

        if self.legacyoversampling:
            self.logger.warn("You are using a deprecated mode for oversampling.")
            if self.oversampling > 1 and self.usepipe: 
               self.logger.fatal("LegacyOversampling is not compatible with UsePIPE at this time")
               sys.exit(1)

        from I3Tray import I3Tray
        from icecube import icetray,phys_services, dataio, dataclasses
        from ..util import BasicCounter
        from ..segments.GenerateNeutrinos import GenerateAtmosphericNeutrinos
        from ..segments import PropagateMuons
        from . import dcorsika
        from .. import weights
   
        if self.cutofftype == "EnergyPerNucleon" : self.spric = True
        elif self.cutofftype == "EnergyPerParticle" : self.spric = False
        else: raise Exception("Undefined CutoffType %s" % cutoff_typ)
        if not self.uppercutofftype:
            self.uppercutofftype = self.cutofftype

        # cast lists
        self.pnorm=[float(x) for x in self.pnorm]
        self.pgam=[float(x) for x in self.pgam]
        self.atmospheres=[int(x) for x in self.atmospheres]
        
        cors = configure_corsika(self)
        cors.f2k = "F"
        outpath = cors.outputpath
        if self.runcorsika and not self.inputfile:
            # Remove output
            if os.path.exists(outpath):
                os.remove(outpath)
            
            if self.usepipe:
                cwd = os.getcwd()
                cors.stage()
                   
                if os.fork(): # parent
                     retval = cors.Execute(self.stats)
                     os.chdir(cwd)
                     if retval:
                         raise Exception("dCorsika exited with return value %s" % retval)
                else: # child
                    os.chdir(cwd)
                    time.sleep(10)
                    self.run_tray(cors)
                    sys.exit()
            else:
                # Run corsika 
                retval = cors.Execute(self.stats) 
                if retval: 
                    raise Exception("dCorsika exited with return value %s" % retval)
                self.run_tray(cors)
        elif self.inputfile:
            cors.outputpath = self.inputfile
            self.run_tray(cors)
        elif not self.runcorsika:
            cors.configure()
            cors.write_steering()
        return 0
    
   def run_tray(self, cors):
        from icecube import icetray,phys_services, dataio, dataclasses
        from .. import segments
        # Instantiate a tray 
        tray = I3Tray()

        # Configure IceTray services
        summary = dataclasses.I3MapStringDouble()
        tray.context['I3SummaryService'] = summary
        
        if self.usegslrng:
            randomService = phys_services.I3GSLRandomService(self.seed)
        else:
            randomService = phys_services.I3SPRNGRandomService(self.seed, self.nproc, self.procnum)
        tray.context["I3RandomService"] = randomService

        tray.AddSegment(CorsikaReaderTraySegment,
                  gcdfile=self.gcdfile, randomService=randomService,
                  SimulateIceTop=self.simulateicetop,
                  select_neutrino=self.selectneutrino, 
                  legacyoversampling=self.legacyoversampling,
                  oversampling=self.oversampling, cors=cors, stats=self.stats)


        if self.enablehistogram and self.histogramfilename:         
            from icecube.production_histograms import ProductionHistogramModule
            from icecube.production_histograms.histogram_modules.simulation.mctree_primary import I3MCTreePrimaryModule
        
            tray.AddModule(ProductionHistogramModule, 
                           Histograms = [I3MCTreePrimaryModule],
                           OutputFilename = self.histogramfilename)
       
        tray.AddModule("I3Writer", 
                       filename = self.outputfile, 
                       Streams=[icetray.I3Frame.TrayInfo,
                                icetray.I3Frame.DAQ,
                                icetray.I3Frame.Stream('S'),
                                icetray.I3Frame.Stream('M')])
         
        # Execute the Tray
        tray.Execute()

        # Save stats
        summary = tray.context['I3SummaryService']

        for k in tray.Usage():
            self.stats[str(k.key())+":usr"] = k.data().usertime
            summary[str(k.key())+":usr"] = k.data().usertime
            self.stats[str(k.key())+":sys"] = k.data().systime
            summary[str(k.key())+":sys"] = k.data().systime
            self.stats[str(k.key())+":ncall"] = k.data().systime
            summary[str(k.key())+":ncall"] = k.data().systime

        WriteI3Summary(summary, self.summaryfile)

class Corsika5ComponentGenerator(ipmodule.ParsingModule):
   """
   Wrapper that generates 5-component CORSIKA. Runs CORSIKA once for each primary type.
   """
   def __init__(self):
        ipmodule.ParsingModule.__init__(self)
        self.logger = logging.getLogger(self.__class__.__name__)

        self.AddParameter('nshowers','Number of generated CR showers',10)
        self.AddParameter('procnum','process number',0)
        self.AddParameter('seed','RNG seed',1)
        self.AddParameter('nproc','Number of processes for (RNG)',1)
        self.AddParameter('gcdfile','GeoCalibDetStatus filename',
                          os.path.expandvars('$I3_DATA/GCD/GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz'))
        self.AddParameter('outputfile','Output filename','corsika.i3')
        self.AddParameter('inputfile',"Input filename (only if you are not generating file)",'')
        self.AddParameter('RunCorsika','Run CORSIKA or only generate INPUTS file',True)
        self.AddParameter('RunNum','Run Number',0)
        self.AddParameter('summaryfile','Summary filename','summary.json')
        self.AddParameter('atmospheres','Atmospheric models',[11,12,13,14]) # mar, jul, oct, dec
        self.AddParameter('eslope','CR spectral index (only if ranpri=0)',-2.7)
        self.AddParameter('pnorm','5-component relative contribution H,He,N,Al,Fe',[5.0, 2.25, 1.1, 1.2, 1.0])
        self.AddParameter('pgam','5-component spectral indices H,He,N,Al,Fe',[2.65,2.60,2.60,2.60,2.60])
        self.AddParameter("locut","Enables skew angle cutfoff",1.58)
        self.AddParameter("kcut","minimum neutrino energy required to keep the shower",0)

        self.AddParameter('CORSIKAseed','CORSIKA seed',1)
        self.AddParameter("eprimarymax",'CR max energy',1.0e9)
        self.AddParameter("eprimarymin","CR min energy",600.0)
        self.AddParameter("fluxsum","", 0.131475115)
        self.AddParameter("length","",1600.0)
        self.AddParameter("OverSampling","Oversampling factor",1)
        self.AddParameter("LegacyOverSampling","Legacy mode Oversampling",False)
        self.AddParameter("radius","",800.0)
        
        self.AddParameter("model","corsika model","sibyll")
        self.AddParameter("lemodel","corsika low-energy model","gheisha")
        self.AddParameter('corsikaVersion','version of corsika to run','74000')
        self.AddParameter("corsikaName","Corsika binary name","corsika")

        self.AddParameter('cthmin','Min theta of injected cosmic rays',0.0)  
        self.AddParameter('cthmax','Max theta of injected cosmic rays',89.99)  
  
        self.AddParameter('ecuts1','hadron min energy (see corsika docs)',273.)  
        self.AddParameter('ecuts2','muon min energy (see corsika docs)',273.)  
        self.AddParameter('ecuts3','electron min energy (see corsika docs)',0.003)  
        self.AddParameter('ecuts4','photon min energy (see corsika docs)',0.003)  
        self.AddParameter("dslope","Change in spectral index",0)
        self.AddParameter("CutoffType","Sets SPRIC=T (EnergyPerNucleon) or F (EnergyPerParticle) ","EnergyPerParticle")
        self.AddParameter("UpperCutoffType","Upper cutoff type (defaults to CutoffType)",None)
        self.AddParameter("RepoURL","URL of repository containing corsika tarballs","http://prod-exe.icecube.wisc.edu")
        self.AddParameter('CVMFS', 'Path to CVMFS repository', '/cvmfs/icecube.opensciencegrid.org/') 
        self.AddParameter('SimulateIceTop','Simulate IceTop detector',False)
        self.AddParameter('UsePipe', 'Use pipe for corsika output', False) 
        self.AddParameter('Polyplopia','Produce coincident showers',False)
        self.AddParameter('BackgroundFile','pre-generated coincident showers file',"")
        self.AddParameter('compress','compress corsika output', False)
        self.AddParameter('skipoptions','Options to skip',[])
        self.AddParameter('HistogramFilename', 'Histogram filename.', None)
        self.AddParameter('EnableHistogram', 'Write a SanityChecker histogram file.', False)
        self.AddParameter('SelectNeutrino','Randomly select CORSIKA neutrino and force interaction',False)
        self.AddParameter("UseGSLRNG","Use I3GSLRandomService",False) 

   def Execute(self,stats):
        self.stats=stats
        if not ipmodule.ParsingModule.Execute(self,self.stats): return 0


        from I3Tray import I3Tray
        from icecube import icetray,phys_services, dataio, dataclasses
        from ..util import BasicCounter
        from .. import segments
        from ..segments.GenerateNeutrinos import GenerateAtmosphericNeutrinos
        from ..segments import PropagateMuons
        from .. import weights
        from . import dcorsika
        import re
   
        if not self.uppercutofftype:
            self.uppercutofftype = self.cutofftype

        # cast lists
        self.pnorm=[float(x) for x in self.pnorm]
        self.pgam=[float(x) for x in self.pgam]
        self.atmospheres=[int(x) for x in self.atmospheres]
        
        p = re.compile('[0-9]+')
        m = p.match(str(self.corsikaversion))
        if m and int(m.group()) < 74000:
                raise Exception('need a newer version of corsika with AtmosphericNeutrinos got %s' % m.group())
        
        if self.pgam[0] > 0:
           self.pgam = [-1*x for x in self.pgam]

        if self.usegslrng:
            randomService = phys_services.I3GSLRandomService(self.seed)
        else:
            randomService = phys_services.I3SPRNGRandomService(self.seed, self.nproc, self.procnum)

        corsika_settings = weights.FiveComponent(self.nshowers,
                                                 self.eprimarymin,self.eprimarymax,
                                                 normalization=self.pnorm,
                                                 gamma=self.pgam,
                                                 LowerCutOffType=self.cutofftype,
                                                 UpperCutOffType=self.uppercutofftype)
        self.ranpri = 0
        self.crtype = corsika_settings[0].PrimaryType
        self.eslope = corsika_settings[0].PrimarySpectralIndex
        self.eprimarymin = corsika_settings[0].EnergyPrimaryMin
        self.eprimarymax = corsika_settings[0].EnergyPrimaryMax
        self.nshowers = corsika_settings[0].NEvents
                
        def convert_outfile(f,i):
             return os.path.join(os.path.dirname(f), '%d_%s'%(i,os.path.basename(f)))
        
        firstevent = 1
        i3files = [self.gcdfile]
        counter = 0
        
        if not os.path.exists(os.path.expandvars("$PWD/")+'outputCors5Comp/'):
              os.makedirs(os.path.expandvars("$PWD/")+'outputCors5Comp')
        OUTDIR= os.path.expandvars("$PWD/")+'outputCors5Comp/'

        for setting in corsika_settings:
               crtype = setting.PrimaryType
               eslope = setting.PrimarySpectralIndex
               eprimarymin = setting.EnergyPrimaryMin
               eprimarymax = setting.EnergyPrimaryMax
               nshowers = setting.NEvents
               cors = configure_corsika(self)
               cors.f2k = "F"
               cors.firstevent = firstevent
               self.logger.info("PType:%s, showers:%s, eslope:%s"% (crtype,nshowers,eslope))
               cors.logfile = os.path.join(os.getcwd(),"CORSIKA000000.%s.log" % setting.PrimaryType)
               i3outfile = OUTDIR+"corsika-%s.i3.gz" % setting.PrimaryType
               i3files.append(i3outfile)

               self.logger.info("Atmospheric model :%s"% cors.atmod)

               generator = CorsikaGenerator()
               generator.SetParameter('nshowers',setting.NEvents)
               generator.SetParameter('procnum',self.procnum*5+counter)
               generator.SetParameter('seed',self.seed)
               generator.SetParameter('nproc',self.nproc*5)
               generator.SetParameter('gcdfile',self.gcdfile)
               generator.SetParameter('outputfile',i3outfile)
               generator.SetParameter('RunCorsika',True)
               generator.SetParameter('RunNum',self.runnum)
               generator.SetParameter('summaryfile',OUTDIR+'summary_%u.json' % counter)
               generator.SetParameter('atmospheres',[cors.atmod])
               generator.SetParameter('ranpri',0)
               generator.SetParameter('eslope', setting.PrimarySpectralIndex)
               generator.SetParameter('crtype',setting.PrimaryType)
               generator.SetParameter("eprimarymax",setting.EnergyPrimaryMax)
               generator.SetParameter("eprimarymin", setting.EnergyPrimaryMin)
               generator.SetParameter('CORSIKAseed',self.corsikaseed*5+counter)
               generator.SetParameter('length',self.length)
               generator.SetParameter('radius',self.radius)
               generator.SetParameter('model',self.model)
               generator.SetParameter('lemodel',self.lemodel)
               generator.SetParameter('corsikaVersion',self.corsikaversion)
               generator.SetParameter('corsikaName',self.corsikaname)
               generator.SetParameter('cthmin',self.cthmin)
               generator.SetParameter('cthmax',self.cthmax)
               generator.SetParameter('ecuts1',self.ecuts1)
               generator.SetParameter('ecuts2',self.ecuts2)
               generator.SetParameter('ecuts3',self.ecuts3)
               generator.SetParameter('ecuts4',self.ecuts4)
               generator.SetParameter("CutoffType",self.cutofftype)
               generator.SetParameter("UpperCutoffType",self.uppercutofftype)
               generator.SetParameter("RepoURL",self.repourl)
               generator.SetParameter("CVMFS",self.cvmfs)
               generator.SetParameter('OverSampling',self.oversampling)
               generator.SetParameter('LegacyOverSampling',self.legacyoversampling)
               generator.SetParameter('SimulateIceTop',self.simulateicetop)
               generator.SetParameter('SelectNeutrino',self.selectneutrino)
               generator.SetParameter('UsePipe', self.usepipe)
               generator.SetParameter('compress', self.compress)
               generator.SetParameter('skipoptions', self.skipoptions)
               generator.SetParameter('gcdfile', self.gcdfile)
               generator.SetParameter('usegslrng', self.usegslrng)
               generator.Execute(self.stats)
               counter += 1


        # Instantiate a tray 
        tray = I3Tray()

        # Instantiate a SummaryService if required
        if self.summaryfile:
           summary_in  = self.summaryfile
           summary_out = self.summaryfile
           if not os.path.exists(self.summaryfile):
              summary_in  = ''

        if summary_in:
           summary = ReadI3Summary(summary_in)
        else: 
           summary = dataclasses.I3MapStringDouble()
        tray.context['I3SummaryService'] = summary

        
        tray.context["I3RandomService"] = randomService

 
        # Configure TraySegment that actually does stuff
        nevents = sum([x.NEvents for x in corsika_settings])

        # Read and combine events
        tray.AddModule("I3Reader","reader", filenamelist=i3files)
        tray.AddModule(BasicCounter,"count_g", Streams = [icetray.I3Frame.DAQ], 
              name = "Generated Events", Stats = self.stats)


        if self.polyplopia:
           tray.AddSegment(segments.PolyplopiaSegment,"coincify",
                    RandomService=randomService,
                    mctype='CORSIKA',
                    timewindow = 40.*I3Units.microsecond,
                    rate = 5.0*I3Units.kilohertz,
           ) 




        if self.enablehistogram and self.histogramfilename:         
            from icecube.production_histograms import ProductionHistogramModule
            from icecube.production_histograms.histogram_modules.simulation.mctree_primary import I3MCTreePrimaryModule
            from icecube.production_histograms.histogram_modules.simulation.mctree import I3MCTreeModule
        
            tray.AddModule(ProductionHistogramModule, 
                           Histograms = [I3MCTreePrimaryModule, I3MCTreeModule],
                           OutputFilename = self.histogramfilename)

        tray.AddModule("I3Writer","writer", 
                       filename = self.outputfile, 
                       Streams=[icetray.I3Frame.TrayInfo,
                                icetray.I3Frame.DAQ,
                                icetray.I3Frame.Stream('S'),
                                icetray.I3Frame.Stream('M')])

        # Execute the Tray
        tray.Execute()

        # Save stats
        summary = tray.context['I3SummaryService']
        for k in tray.Usage():
            self.stats[str(k.key())+":usr"] = k.data().usertime
            summary[str(k.key())+":usr"] = k.data().usertime

            self.stats[str(k.key())+":sys"] = k.data().systime
            summary[str(k.key())+":sys"] = k.data().systime

            self.stats[str(k.key())+":ncall"] = k.data().ncall
            summary[str(k.key())+":ncall"] = k.data().ncall
        WriteI3Summary(summary, self.summaryfile)

        del tray
        return 0



