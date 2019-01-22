#!/usr/bin/env python
################################ Tray 1 ##############################
#   
#   IceProd classes for processing hit-generation in IceSim.
#   Author: juancarlos@icecube.wisc.edu
#   
######################################################################
import os
import tempfile

from os.path import expandvars, join

from I3Tray import *
from .. import ipmodule

from ..util import ReadI3Summary, WriteI3Summary

def PPCTraySegment(tray,
                   name,
                   efficiency,
                   oversize,
                   gpulib,
                   volumecyl,
                   IceModelLocation=expandvars("$I3_BUILD/ppc/resources/ice"),
                   keep_empty_events=False,
                   IceModel="SpiceMie",
                   InputMCTree="I3MCTree",
                   mcpeseries = "I3MCPESeriesMap",
                   UseGPUs=True,
                   GPU=-1,
                   tempdir = None):

        """
        PPC Photon Propagation Code TraySegment (supports CUDA/OpenCL) 
        """

        # Load libraries 
        from icecube import icetray, interfaces ,dataclasses ,simclasses,sim_services
        from icecube import polyplopia
        from icecube import ppc
        
        # Do one or the other
        UseCPU = not UseGPUs

        ppcIceModel = None
        if IceModelLocation is None:
             IceModelLocation = expandvars("$I3_SRC/ice-models/resources/models")

        if IceModel == "SpiceMie":
             ppcIceModel = expandvars(IceModelLocation+"/spice_mie")
        elif IceModel == "SpiceLea":
             ppcIceModel = expandvars(IceModelLocation+"/spice_lea")
        elif IceModel == "Spice3":
             ppcIceModel = expandvars(IceModelLocation+"/spice_3")
        elif IceModel == "Spice3.1":
             ppcIceModel = expandvars(IceModelLocation+"/spice_3.1")
        elif IceModel == "Spice3.2":
             ppcIceModel = expandvars(IceModelLocation+"/spice_3.2")
        elif IceModel == "Spice3.2.1":
             ppcIceModel = expandvars(IceModelLocation+"/spice_3.2.1")
        elif os.path.exists(expandvars(IceModelLocation+"/"+IceModel)):
             ppcIceModel = expandvars(IceModelLocation+"/"+IceModel)
        else:
             raise RuntimeError("Unknown ice model: %s", IceModel)

        if not tempdir:
            tempdir = tempfile.mkdtemp(dir = join(os.getcwd()))
                    
        x_ppc_tables_dir = join(tempdir, 'xppc_ice')
        if not os.path.exists(x_ppc_tables_dir):
                os.mkdir(x_ppc_tables_dir)                
        
        icetray.logging.log_debug('cp -r -L %s/* %s' % (ppcIceModel, x_ppc_tables_dir))
        if os.system('cp -r -L %s/* %s' % ( ppcIceModel, x_ppc_tables_dir )):
           raise Exception("XPPC cannot copy ice model parameters")

        ppc_resource_path = expandvars("$I3_SRC/ppc/resources/ice/")
        for filename in ['rnd.txt', 'as.dat', 'wv.dat']:
                source_file = ppc_resource_path + filename
                icetray.logging.log_debug('cp %s %s' % ( source_file, x_ppc_tables_dir ))
                if os.system('cp %s %s' % ( source_file, x_ppc_tables_dir )):
                        raise Exception("cannot copy %s" % filename)

        ppc_cfg_in  = open(join(ppcIceModel,'cfg.txt'), 'r')
        x_ppc_tables_dir_cfg = join(x_ppc_tables_dir, 'cfg.txt')
        ppc_cfg_out = open(x_ppc_tables_dir_cfg,'w')
        for line in ppc_cfg_in.readlines():
            toks = list(map(str.strip,line.split('#')))
            if len(toks) > 1:
               if toks[1].startswith("over-R:"):
                  toks[0] = str(oversize)
               if toks[1].startswith("overall DOM efficiency"):
                  icemodel_efficiency_factor = float(toks[0]) 
                  toks[0] = str(efficiency*icemodel_efficiency_factor)
            icetray.logging.log_debug("  #".join(toks))
            ppc_cfg_out.write("  #".join(toks)+'\n')
        ppc_cfg_in.close()
        ppc_cfg_out.close()
        
        os.putenv("PPCTABLESDIR", x_ppc_tables_dir)

        if UseGPUs:
            os.putenv("OGPU","1")
        else:
            os.putenv("OCPU","1")
        if GPU >= 0 and UseGPUs:
            os.putenv("CUDA_VISIBLE_DEVICES",str(GPU))
            os.putenv("COMPUTE",":0."+str(GPU))
            os.putenv("GPU_DEVICE_ORDINAL",str(GPU))

        tray.AddModule("i3ppc", "ppc", 
                  If = lambda f: f[InputMCTree].size() or keep_empty_events,
                  #gpu=GPU, 
                  cyl=volumecyl,
                  keep=keep_empty_events,
                  MCTree=InputMCTree)

        # PPC does not have an option for setting the name of the PE map.
        # If the default name of PE map changes, this needs to be updated.
        if mcpeseries != "MCPESeriesMap":
           tray.AddModule("Rename", keys=["MCPESeriesMap",mcpeseries])





###### IP Modules ###########

class PPC(ipmodule.ParsingModule):
   """
    GPU Photon propagation
   """
   def __init__(self):
        ipmodule.ParsingModule.__init__(self)
        self.configured = False

        self.AddParameter('gcdfile','GeoCalibDetStatus filename','')
        self.AddParameter('inputfilelist','list of input filenames',[])
        self.AddParameter('outputfile','Output filename','')
        self.AddParameter('seed','RNG seed',0)
        self.AddParameter('procnum','job number (RNG stream number)',0)
        self.AddParameter('nproc','Number of jobs (Number of RNG streams)',1)
        self.AddParameter('summaryfile','JSON Summary filename','summary.json')
        self.AddParameter('MJD','MJD (0=do not modify)',0)
        self.AddParameter("GPU", 
                          "Graphics Processing Unit number (shoud default to environment if None)",
                          -1)
        self.AddParameter("UseGPUs", "Use Graphics Processing Unit",False)
        self.AddParameter('RunMPHitFilter',"Run polyplopia's mphitfilter",True)
        self.AddParameter("oversize","over-R: DOM radius oversize scaling factor",5)
        self.AddParameter("efficiency","overall DOM efficiency correction",1.00)
        self.AddParameter("gpulib","set gpu library to load (defaults to cuda)","opencl")
        self.AddParameter("volumecyl","set volume to regular cylinder (set to False for 300m spacing from the DOMs)",True)
        self.AddParameter("PhotonSeriesName","Photon Series Name","I3MCPESeriesMap") 
        self.AddParameter("IceModelLocation","Location of ice model param files", expandvars("$I3_BUILD/ice-models/resources/models")) 
        self.AddParameter("IceModel","ice model subdirectory", "SpiceMie") 
        self.AddParameter("MCTreeName","Name of MCTree frame object", "I3MCTree") 
        self.AddParameter("KeepEmptyEvents","Don't discard events with no MCPEs", False) 
        self.AddParameter('HistogramFilename', 'Histogram filename.', None)
        self.AddParameter('EnableHistogram', 'Write a SanityChecker histogram file.', False)
        self.AddParameter('PropagateMuons', 'Run PROPOSAL to do in-ice propagation', True)
        self.AddParameter('PROPOSALParams','any other parameters for proposal',dict())
        self.AddParameter('TempDir', 'Temporary working directory with the ice model', None)
        self.configured = False

   def Execute(self,stats):
        if not ipmodule.ParsingModule.Execute(self,stats):
                return 0
        from icecube import icetray, dataclasses, dataio, phys_services, interfaces
        from .ppc import PPCTraySegment
        from I3Tray import I3Tray,I3Units

        # Instantiate a tray 
        tray = I3Tray()

        summary_in  = self.summaryfile
        summary_out = self.summaryfile
        if not os.path.exists(self.summaryfile):
           summary_in  = ''

        # Configure IceTray services
        if summary_in:
           summary = ReadI3Summary(summary_in)
        else: 
           summary = dataclasses.I3MapStringDouble()
        tray.context['I3SummaryService'] = summary

        # Configure IceTray modules 
        tray.AddModule("I3Reader", "reader",filenamelist=[self.gcdfile]+self.inputfilelist)

        from .. import segments
        if self.propagatemuons:
        	randomServiceForPropagators = phys_services.I3SPRNGRandomService(
             		seed = self.seed,
             		nstreams = self.nproc*2,
             		streamnum = self.nproc + self.procnum)
        	tray.context['I3PropagatorRandomService'] = randomServiceForPropagators

        	tray.AddModule("Rename","rename_corsika_mctree",Keys=['I3MCTree','I3MCTree_preMuonProp'])
        	tray.AddSegment(segments.PropagateMuons, 'propagator',
                        RandomService= randomServiceForPropagators,
                        **self.proposalparams
        	) 


        
        tray.AddSegment(PPCTraySegment,"ppc_photons",
			gpu = self.gpu,
			usegpus = self.usegpus,
			efficiency = self.efficiency,
			oversize = self.oversize,
			IceModelLocation = self.icemodellocation,
			IceModel = self.icemodel,
			volumecyl = self.volumecyl,
			gpulib = self.gpulib,
			InputMCTree = self.mctreename,
			keep_empty_events = self.keepemptyevents,
			mcpeseries = self.photonseriesname,
                        tempdir = self.tempdir)

        if self.runmphitfilter:
           tray.AddModule("MPHitFilter","hitfilter",
              HitOMThreshold=1,
              RemoveBackgroundOnly=False,
              I3MCPESeriesMapName=self.photonseriesname)

        if self.enablehistogram and self.histogramfilename:         
            from icecube.production_histograms import ProductionHistogramModule
            from icecube.production_histograms.histogram_modules.simulation.mcpe_module import I3MCPEModule
        
            tray.AddModule(ProductionHistogramModule, 
                           Histograms = [I3MCPEModule],
                           OutputFilename = self.histogramfilename)


        tray.AddModule("I3Writer","writer", 
            filename=self.outputfile,
            Streams=[icetray.I3Frame.TrayInfo,
                     icetray.I3Frame.DAQ,
                     icetray.I3Frame.Stream('S'),
                     icetray.I3Frame.Stream('M')])

        # Execute the Tray
        tray.Execute()

        summary = tray.context['I3SummaryService']

        # Save stats
        for k in tray.Usage():
            stats[str(k.key())+":usr"] = k.data().usertime
            summary[str(k.key())+":usr"] = k.data().usertime

            stats[str(k.key())+":sys"] = k.data().systime
            summary[str(k.key())+":sys"] = k.data().systime

            stats[str(k.key())+":ncall"] = k.data().ncall
            summary[str(k.key())+":ncall"] = k.data().ncall

        WriteI3Summary(summary, summary_out)

        del tray
        return 0


   def Finish(self,stats={}):
        self.logger.info("finish %s: %s" % (self.__class__.__name__,
                                            self.GetParameter("execute")))
        return 0




