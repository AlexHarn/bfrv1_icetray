########################################################
# Fix selection weight of NuGen that exists up to 
# IceSim 5.1 (from 3years ago)
# This bug affects to NuTau and NuE grashow resonance only
# 
from icecube import icetray, dataclasses 

class fix_inice_selection_weight(icetray.I3ConditionalModule):
    def __init__(self, ctx):
        icetray.I3ConditionalModule.__init__(self, ctx)
        self.AddParameter("WeightDictName","I3MCWeightDict name","I3MCWeightDict")
        self.AddParameter("MCTreeName","name of MCTree","I3MCTree")
        self.AddOutBox("OutBox")
	
    def Configure(self):
        self.wmapname = self.GetParameter("WeightDictName")
        self.mctreename = self.GetParameter("MCTreeName")

    def DAQ(self, frame) :

        if not frame.Has(self.mctreename) :
           log_error("Q-frame doesn't contain %s", self.mctreename)

        if not frame.Has(self.wmapname) :
           log_error("Q-frame doesn't contain %s", self.wmapname)

        mctree = frame[self.mctreename]

        # copy weight map. 
        wmap = dataclasses.I3MapStringDouble(frame[self.wmapname])

        ninicenus = 0
        ntotalinicenus = 0

        for p in mctree.fast_iter():
           if p.is_neutrino:
               ntotalinicenus += 1

               #-------------
               # omit InIce particles generated inside the detection volume
               #-------------

               pstart = p.pos
               if pstart.magnitude == 0 :
                   #print("pstart magnitude is zero, started inside detector. omit.")
                   continue

               pdir = p.dir
               if pstart*pdir > 0 :
                   #print("start point is behind the impact position, started inside detector. omit.")
                   continue

               # To be an InIce neutrino candidate, the neutrino must start from 
               # outside of detection volume. The requirement is distance to pstart from IceCube origin 
               # must be larger than the distance to entrance point to detection volume from origin.
               # If pstart is within detection volume, that has a parent particle
               # or the primary neutrino started within the detection volume by less than ~1e-5[m], due to round-off error.
               # The latter one is InIce neutrino, so we reject neutrinos started inside detector AND not primary.
               impact = wmap["ImpactParam"]
               active_len_before = wmap["TrueActiveLengthBefore"]
               to_entrance2 = impact**2 + active_len_before**2
               if pstart.mag2 < to_entrance2 and mctree.has_parent(p):
                   #print("startpoint is inside detection volume: pstart_mag2 %f, to_entrance2 %f,  energy %f, zenith %f deg" % (pstart.mag2, to_entrance2, p.energy, pdir.zenith*180/3.1415))
                   continue

               pend = pstart + p.length*pdir
               if pend.mag2 > to_entrance2 and pend*pdir < 0 :
                   #print("particle ended before reaching to detection volume")
                   continue

               # this neutrino is a candidate of final interaction neutrino
               ninicenus += 1

        # now multiply ninicenus to weights. 
        wmap["PropagationWeight_Bug"] = wmap["PropagationWeight"]
        wmap["PropagationWeight"] = wmap["PropagationWeight"]*ninicenus
        wmap["TotalWeight_Bug"] = wmap["TotalWeight"]
        wmap["TotalWeight"] = wmap["TotalWeight"]*ninicenus
        wmap["OneWeight_Bug"] = wmap["OneWeight"]
        wmap["OneWeight"] = wmap["OneWeight"]*ninicenus
        if "OneWeightPerType" in wmap:
            wmap["OneWeightPerType_Bug"] = wmap["OneWeightPerType"]
            wmap["OneWeightPerType"] = wmap["OneWeightPerType"]*ninicenus
        if "TotalInteractionProbabilityWeight" in wmap:
            wmap["TotalInteractionProbabilityWeight_Bug"] = wmap["TotalInteractionProbabilityWeight"]
            wmap["TotalInteractionProbabilityWeight"] = wmap["TotalInteractionProbabilityWeight"]*ninicenus

        frame.Delete(self.wmapname)
        frame.Put(self.wmapname, wmap)

        self.PushFrame(frame,"OutBox")

        return True

