from icecube import icetray,dataio,phys_services,dataclasses, simclasses
import os.path
from icecube.polyplopia import I3MapStringString


class PoissonPEMerger(icetray.I3Module):
        framebuffer  = []
        delta_t_     = 0

        def __init__(self,context):
                icetray.I3Module.__init__(self,context)
                self.AddParameter('BackgroundFile','File to inject events from','')
                self.AddParameter("I3MCPESeriesName","Name of I3MCPESeriesMap object in frame","I3MCPESeriesMap")
                self.AddParameter("MCTreeName","Name of I3MCTree object in frame", "I3MCTree")
                self.AddParameter("MMCTrackName","Name of MMCTrackList object in frame", "MMCTrackList")
                self.AddParameter('TimeWindow','Time window for randomized events (in the center will be the neutrino. A big timewindow should be taken (200000 ??) - see \
      	      	      	      	  http://wiki.icecube.wisc.edu/index.php/Coincident_Simulation_Fixes#Delta-t_timestructures_with_this_new_scheme',200000*icetray.I3Units.ns)
                self.AddParameter("Rate", "Event rate (zero if polyplopia should get this from summary service)", 0.0);
                self.AddParameter('Seed','RNG Seed',0)
                self.AddParameter('RandomService','RNG Service (superceeds Seed)',None)
                self.AddOutBox('OutBox')
                self.stats = {}

        def Configure(self):
                fileName                    = self.GetParameter('BackGroundFile')
                self.timewindow             = self.GetParameter('TimeWindow')
                sampling_rate               = self.GetParameter("Rate")
                self.rand                   = self.GetParameter("RandomService")
                if not self.rand:
                   self.rand                = phys_services.I3GSLRandomService(self.GetParameter("Seed"))

                if  not os.path.exists(fileName): 
                    raise Exception("no such file %s"%fileName)
                self.file                   = dataio.I3File(fileName)

                if (sampling_rate > 0):
                    self.tau_                   = 1.0/sampling_rate
                else: 
                    raise Exception("sampling_rate < 0")

                self.counter                = 0
                

        def DAQ(self,frame):
                from icecube import polyplopia
                from icecube.sim_services import MergeMCPEs
                
                # determine how many CR muons in static window
                mean            = self.timewindow/self.tau_
                events_to_merge = self.rand.poisson(mean)
                mergedframe     = frame
                hitseries       = mergedframe.Get(self.GetParameter("I3MCPESeriesName"))
                firsthit        = polyplopia.GetFirstHitTime(hitseries);

                mctreename = self.GetParameter("MCTreeName")
                mmctrackname = self.GetParameter("MMCTrackName")
                if not mctreename in frame:
                   raise Exception("no MCTree object '%s' found in frame" % mctreename)
                mctree          = frame.Get(mctreename)
                weights         = dataclasses.I3MapStringDouble()


                if "CorsikaWeightMap" in frame:
                   weights = frame.Get("CorsikaWeightMap")

                weights["DiplopiaWeight"] = 1.0
                primary         = mctree.primaries[0]

                bgtree = dataclasses.I3MCTree()
                bgmcpes = simclasses.I3MCPESeriesMap()
                bgmcpeInfo = None
                bgmmctrack = simclasses.I3MMCTrackList()

                for event_i in range(events_to_merge) :
		    
                    bgframe   = self.file.pop_frame(icetray.I3Frame.DAQ)
                    if not bgframe: # If we run out of background before signal, this is bad!
                       raise Exception("Out of background events!!!!")

                    # Get a time centered around start of event
                    ti = self.rand.uniform(firsthit-0.5*self.timewindow,firsthit+0.5*self.timewindow) 
                    ctree = bgframe[mctreename]
                    polyplopia.MergeMCTrees(bgtree,ctree, ti)
                    if mmctrackname in bgframe:
                       cmmctrack = bgframe[mmctrackname]
                       polyplopia.MergeMMCInfo(bgmmctrack,cmmctrack, ti)

                    cmcpes = bgframe["I3MCPESeriesMap"]
                    cmcpeInfo = None
                    if bgframe.Has("I3MCPESeriesMapParticleIDMap"):
                       cmcpeInfo = bgframe["I3MCPESeriesMapParticleIDMap"]
                    bgmcpeInfo=MergeMCPEs(bgmcpes,cmcpes,ti,bgmcpeInfo,cmcpeInfo)
		    
                mergedframe.Put("BackgroundMMCTrackList",bgmmctrack)
                mergedframe.Put("BackgroundI3MCTree",bgtree)
                mergedframe.Put("BackgroundI3MCPESeriesMap",bgmcpes)
                mergedframe.Put("PolyplopiaPrimary",primary)
                if "CorsikaWeightMap" in mergedframe:
                    del mergedframe["CorsikaWeightMap"]
                mergedframe.Put("CorsikaWeightMap",weights)

                self.PushFrame(mergedframe)
                self.counter += 1
                
                return True
 
        def Finish(self):
            self.file.close()
            print("processed %d events" % self.counter)
          

