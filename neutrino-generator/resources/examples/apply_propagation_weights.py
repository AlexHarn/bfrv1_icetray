#########################################################
#
# NuGen propagation weighter for DETECTOR dataset
# K.Hoshina (hoshina@icecuve.wisc.edu)
#
# This module applies propagation weight of neutrino
# during neutrino's propagation from the Earth's surface
# to detector, to the NuGen simulation set generated with
# simmode = "DETECTOR".
#
# Currently this script supports NuE flavor and NuMu flavor only.
# For NuEBar use it less than 1PeV where grashow resonance is not dominant.
#
# This module adds 
# "PrimaryNeutrinoEnergy[ParamSuffix]"
# "PropagationWeight[ParamSuffix]"
# "OneWeight[ParamSuffix]" 
# "TotalWeight[ParamSuffix]" 
# to include the effect of neutrino's absorption or energy loss 
# during propagation inside the Earth, with given earth model.
# If the ParamSuffix is empty, it replaces old OneWeight etc.
# Default model is the PREM, no oscillation, no sterile neutrinos.
#
# The module is aimed at simply replacing current NuMu
# production (with FULL simmode) to "DETECTOR" mode,
# so that does not support for NuTaus.
# 

from icecube import icetray, dataclasses 
from array import array
import os
import numpy as np
import copy
import math
import random 
import bisect 
import sys
if sys.version_info[0] >= 3:
    import pickle
else:
    import cPickle as pickle

class apply_propagation_weights(icetray.I3ConditionalModule):

    def __init__(self, context):

        icetray.I3ConditionalModule.__init__(self, context)

        self.AddParameter( 'WeightFilePaths',
                           'path to the weight files',
                           [])

        self.AddParameter( 'WeightFilePrefixes',
                           'prefix of weight file name',
                           ['PREM_CSMS'])

        self.AddParameter( 'PrimaryGammaIndex',
                           'gamma index of primary neutrino of your NuGen simulation, positive value',
                           1.0
                           )

        self.minElog = 2.0
        self.AddParameter( 'EnergyLogMin',
                           'min log10(energy) that you want to reweight',
                           self.minElog
                           )

        self.maxElog = 0
        self.AddParameter( 'EnergyLogMax',
                           'max log10(energy) that you want to reweight',
                           self.maxElog
                           )

        self.AddParameter( 'ParamSuffixes',
                           'suffix to add "OneWeight" and "PrimaryNeutrinoEnergy. If empty, it replaces old OneWeight etc."',
                           [])


        self.AddOutBox("OutBox")

    def GetDict(self, filename):
        # make it a function to minimize memory usage...
        file = open(filename, 'r')
        alldict = pickle.load(file)
        adict = {}
        adict["minElog"] = copy.copy(alldict["minElog"])
        adict["maxElog"] = copy.copy(alldict["maxElog"])
        adict["dElog"] = copy.copy(alldict["dElog"])
        adict["minzen"] = copy.copy(alldict["minzen"])
        adict["maxzen"] = copy.copy(alldict["maxzen"])
        adict["dzen"] = copy.copy(alldict["dzen"])
        adict["enebins"] = copy.copy(alldict["energy"][0])
        adict["weight"] = copy.copy(alldict["weight"])
        adict["count3D_w"] = copy.copy(alldict["count3D_w"])
        file.close()
        #print("file %s is loaded" % (filename))
        return adict

    def Configure(self):

        self.fileprefixes = self.GetParameter('WeightFilePrefixes')
        self.filepaths = self.GetParameter('WeightFilePaths')
        I3_DATA = os.environ["I3_DATA"]
        if len(self.filepaths) == 0 :
            self.filepaths = ["%s/%s" % (I3_DATA, "neutrino-generator/propagation_weights")]

        self.gamma = float(self.GetParameter('PrimaryGammaIndex'))
        self.minElog = float(self.GetParameter('EnergyLogMin'))
        self.maxElog = float(self.GetParameter('EnergyLogMax'))
        self.suffix = self.GetParameter('ParamSuffixes')

        gammatag = "W1.0"
        if (self.gamma != 1.0) :
            gammatag = "W%1.1f" % (self.gamma)

        self.ptypes = ["NuE","NuEBar","NuMu","NuMuBar"]
        self.dicts = {}

        # save dicts to another dict
        for i,  prefix in enumerate(self.fileprefixes):

            # open pickles for nu and nubar
            filepath = self.filepaths[i]
            nufilename = "%s/%s_E%d_%d_%s_Nu.pickles" % (filepath, prefix, int(self.minElog), int(self.maxElog), gammatag)
            nudict = self.GetDict(nufilename)
            nubarfilename = "%s/%s_E%d_%d_%s_NuBar.pickles" % (filepath, prefix, int(self.minElog), int(self.maxElog), gammatag)
            nubardict = self.GetDict(nubarfilename)
     
            for ptype in self.ptypes :
                adict = nudict
                if "Bar" in ptype :
                    adict = nubardict
                self.dicts[(prefix, ptype)] = adict

    def EstimatePrimaryEnergy(self, adict, izen, ielog, enebins, inject_e) :
        count3D_w = adict["count3D_w"]

        # primaries is the histogram of energy
        # of primary that contributed to bin 
        # izen and ielog of inice particles
        primaries = count3D_w[izen][ielog]
        accumprim = []
        totalcount = 0
        debug = []
        edge_bin = -1
        for index, prim_count in enumerate(primaries) :
            totalcount += prim_count
            accumprim.append(totalcount)
            debug.append([index, (enebins[index], enebins[index+1]), prim_count, totalcount])
            if edge_bin < 0 and totalcount > 0 :
                edge_bin = index


        if totalcount == 0 :
            '''
            Couldn't find any candidate of primary energy.
            With a given zenith angle, inice neutrino can't have 
            such a high energy.
            Return 0 that means we should not use the event 
            (anyway the propagation weight should be zero
            so that does not affect to the weighted plots.)
            '''
            return 0

        r = random.uniform(0, totalcount)
        i = bisect.bisect_left(accumprim, r)

        # find primary energy for index i
        # If index is 0, that means (mostly) it didn't have any NC interaction.
        # Take lowest energy.
        if i == edge_bin :
            prim_e = inject_e
        else :
            prim_hilogE = enebins[i+1]
            prim_lowlogE = enebins[i]

            '''
            primloge = random.uniform( prim_lowlogE, prim_hilogE)
            prim_e = 10**primloge
            '''
            # select an energy within the given energy bin with the
            # given gamma index.
            if self.gamma == 1.0 :
                primloge = random.uniform( prim_lowlogE, prim_hilogE)
                prim_e = 10**primloge
            else :
                r2 = random.uniform(0., 1.);
                prim_lowE = 10**prim_lowlogE
                prim_hiE  = 10**prim_hilogE
                energyP = (1-r2)*(prim_lowE**(1-self.gamma)) + r2*(prim_hiE**(1-self.gamma))
                prim_e = energyP**(1./(1-self.gamma))


        #print debug
        #print("r %f index %f accumprim[index] %f, primloge %f, iniceloge %f" %(r, i, accumprim[i], math.log10(prim_e), math.log10(inject_e)))

        return prim_e

    def DAQ(self, frame):

        if not frame.Has('I3MCWeightDict') :
            self.PushFrame(frame,"OutBox");
            return True

        weights = copy.copy(frame.Get( 'I3MCWeightDict' ))
        if not 'TotalWeight' in weights :
            # somehow this event failed to make interaction
            self.PushFrame(frame,"OutBox");
            return True

        simmode = weights["SimMode"]
        if simmode < 2 :
            # 0 : Full 1:InEarth 2:Detector
            # reweighting is applicable for data generated with SimMode == "Detector" only
            self.PushFrame(frame,"OutBox");
            return True

        gen_max_elog = weights["MaxEnergyLog"]

        # get primary particle
        mctree = frame["I3MCTree"]
        primaries = mctree.get_primaries()
        injected_nus = []

        for p in primaries :
            if p.type_string in self.ptypes :
                injected_nus.append(p)

        if len(injected_nus) > 1 :
            print("your frame contains multiple neutrinos! TotalWeight may be nonsense.")

        injected_nu = injected_nus[0]
        injected_e  = injected_nu.energy
        injected_zen = math.degrees(injected_nu.dir.zenith)

        # get info from dictionary
        for i, prefix in enumerate(self.fileprefixes) :

            adict = self.dicts[(prefix, injected_nu.type_string)]
            minElog = adict["minElog"]
            maxElog = adict["maxElog"]
            dElog   = adict["dElog"]
            minzen  = adict["minzen"]
            maxzen  = adict["maxzen"]
            dzen    = adict["dzen"]

            # energy range check. Default tables supports energy 10^2 
            # to 10^7GeV, and it's not good using the table to weight
            # simulations generated above 10^7GeV.
            if (gen_max_elog > maxElog) :
                print("Your input simlation contains injected energy higher than supported energy range of InEarth table. Use different table. Input minElog %f, table minElog %f" % (gen_max_elog, minElog))

            # range protection:
            if injected_zen < minzen :
                injected_zen = minzen
            if injected_zen > maxzen :
                injected_zen = maxzen

            injected_elog = math.log10(injected_e)
            if injected_elog < minElog:
                injected_elog = minElog 
            if injected_elog > maxElog:
                injected_elog = maxElog

            # find a bin in PropagationWeight table
            ielog = int((injected_elog - minElog) / dElog) 
            if injected_elog == maxElog:
                # avoid ielog fall into outside of suported ielog range
                ielog -= 1

            eloglow = minElog + ielog * dElog
            elogratio = (injected_elog - eloglow) / (dElog)
            izen  = int((injected_zen - minzen) / dzen) 
            if injected_zen == maxzen :
                # avoid ielog fall into outside of suported ielog range
                izen -= 1
            #print("minelog %f, maxelog %f, elog = %f, ielog %d, zen = %g, izen %d" % (minElog, maxElog, injected_elog, ielog, injected_zen, izen))

            # get propagation weight
            prop_w = adict['weight'][izen,ielog]
            #print("prop_w = %g" %  (prop_w))

            # estimate primary energy.
            prim_e = -1
            if (math.log10(injected_e) < minElog) :
                # inice energy is too small. primary energy and inice energy should be same.
                prim_e = injected_e
            else :
                enebins = adict["enebins"]
                prim_e = self.EstimatePrimaryEnergy(adict, izen, ielog, enebins, injected_e)
                #print("estimated primary E = %f" % (prim_e))

            #old_totalweight = weights["TotalInteractionProbabilityWeight"]
            old_totalweight = weights["TotalWeight"]
            totalweight = 0
            onew = 0
            onewpertype = 0

            if prim_e > 0 :
                #
                # Found a candidate of primary energy.
                # recalculate OneWeight
                #
                solidangle = weights["SolidAngle"]
                injectionareaCGS = weights["InjectionAreaCGS"]
                totalweight = old_totalweight * prop_w

                # energy integral
                emax = 10**weights["MaxEnergyLog"]
                emin = 10**weights["MinEnergyLog"]
                eintg = 0
                if (self.gamma == 1) :
                    eintg = math.log(emax / emin)
                else :
                    eintg = (pow(emax , (1.- self.gamma)) - pow(emin, (1.- self.gamma))) / (1.- self.gamma)

                onew = totalweight * eintg / (prim_e**(-self.gamma)) * solidangle * injectionareaCGS
                typew = weights["TypeWeight"]
                onewpertype = onew / typew

            # update parameters
            newprim = dataclasses.I3Particle(dataclasses.I3Particle.Null, dataclasses.I3Particle.unknown)
            store_primary = False
            if frame.Has("NuGPrimary") :
                prim = copy.copy(frame.Get("NuGPrimary"))
                newprim = prim
                frame.Delete("NuGPrimary")
                frame.Put("NuGPrimary_NoEarth", prim)
                store_primary = True

            if len(self.suffix) == 0 and len(self.fileprefixes) == 1:
                # replace old parameters in frame.
                weights["PrimaryNeutrinoEnergy_NoEarth"] = weights["PrimaryNeutrinoEnergy"] 
                weights["PropagationWeight_NoEarth"] = weights["PropagationWeight"]
                weights["OneWeight_NoEarth"] = weights["OneWeight"]
                weights["TotalWeight_NoEarth"] = weights["TotalWeight"]
                weights["PrimaryNeutrinoEnergy"] = prim_e
                weights["PropagationWeight"] = prop_w
                weights["OneWeight"] = onew
                weights["OneWeightPerType"] = onewpertype 
                weights["TotalWeight"] = totalweight
                newprim.energy = prim_e
                if store_primary :
                    frame.Put("NuGPrimary", newprim)
                
            elif len(self.suffix) == len(self.fileprefixes):
                # for EarthCore or Cross Section analysis.
                # just add new parameters.
                weights["PrimaryNeutrinoEnergy"+self.suffix[i]] = prim_e
                weights["PropagationWeight"+self.suffix[i]] = prop_w
                weights["OneWeight"+self.suffix[i]] = onew
                weights["OneWeightPerType"+self.suffix[i]] = onewpertype
                weights["TotalWeight"+self.suffix[i]] = totalweight

            else :
                print("Sizes of WeigtFilePrefixes and ParamSuffixes don't match. Nothing is saved.")

        frame.Delete("I3MCWeightDict")
        frame.Put("I3MCWeightDict", weights)

        self.PushFrame(frame,"OutBox");
        return True

    def Finish(self):
        return True




