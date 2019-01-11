from icecube import icetray, dataclasses
from icecube.icetray.i3logging import log_info, log_warn, log_debug, log_trace

class MakeQualityCuts(icetray.I3Module):
    '''
    Module which actually does the cuts: Look whether they survived the quality cuts. If not: remove OR put bools in a common frameObject which you can evaluate later.
    '''
    def __init__(self, ctx):
        icetray.I3Module.__init__(self, ctx)
        self.AddOutBox("OutBox")
        self.AddParameter("CutOrder","Contains all cuts in the order that you want.",[])
        self.AddParameter("CutsToEvaluate", "Which keys do we need to check? And how?", {})
        self.AddParameter("CutsNames", "Names of Cuts that we put in the bool, they should all be true for good quality.", {})
        self.AddParameter("CollectBools", "Name of output I3MapStringBool", "IT73AnalysisIceTopQualityCuts")
        self.AddParameter("RemoveEvents","Remove the events or keep?",False)

    def Configure(self):
        self.cutOrder=self.GetParameter("CutOrder")
        self.listOfCuts = self.GetParameter("CutstoEvaluate")
        self.listOfBoolNames=self.GetParameter("CutsNames")
        self.output_name = self.GetParameter("CollectBools")
        self.remove=self.GetParameter("RemoveEvents")
        #List of counters, to see how many events survived.
        self.counters=dict((key,0) for key in self.cutOrder)
        self.ntotal=0

    def Physics(self,frame):
        # If the output map already exists, just add the key and delete it. Otherwise make a new map, add all the keys. Then delete them.
        if self.output_name in frame:
            outputStringBool=frame[self.output_name]
            frame.Delete(self.output_name)
        else:
            outputStringBool=dataclasses.I3MapStringBool()
        
        # Add keys
        self.ntotal+=1
        cut = False ## To see whether event was already cut... (simply for counter)
        for key in self.cutOrder:
            cut_func=self.listOfCuts[key]
            outputStringBool[self.listOfBoolNames[key]]=cut_func(frame)
            if cut_func(frame) and not cut:
                self.counters[key]+=1
            else:
                cut=True

        # Remove or add bool list        
        if self.remove:
            if all(outputStringBool.values()):
                frame[self.output_name]=outputStringBool   # Still book this, it is needed further on!
                self.PushFrame(frame,"OutBox")
        else:
            frame[self.output_name]=outputStringBool
            self.PushFrame(frame,"OutBox")
                
    def Finish(self):
        log_info( "Number of events at beginning of QualityCuts : %i"%self.ntotal,"MakeQualityCuts")
        if self.ntotal > 0:
            for key in self.cutOrder:
                log_info("Number of events after %s cut: %i" %(key, self.counters[key]),"MakeQualityCuts")
