#!/usr/bin/env python

import os, sys

if __name__ == "__main__":
    print("this is an auxiliary script for rudimentary double-muon testing")
    sys.exit(0)

from icecube import icetray, dataclasses
from icecube.gulliver_modules.fortytwo import I3FrameChecker, isclose
import numpy as np

class PulseMapChecker(I3FrameChecker):
    """
    This checker checks all pulse times and charges of the given pulse maps.
    (If no pulse maps given: check all of them.)
    """
    def __init__(self,rpsmlist=[],maxnpm=4):
        I3FrameChecker.__init__(self)
        self.pulsemapnames = rpsmlist
        self.maxnpm = maxnpm
        self.ncheck=0
    def get_test_values(self,i,frame):
        if 0 == len(self.pulsemapnames):
            self.pulsemapnames = [ k for k in list(frame.keys()) if f.type_name(k) == 'I3RecoPulseSeriesMap' ]
            self.pulsemapnames += [ k for k in list(frame.keys()) if f.type_name(k) == 'I3RecoPulseSeriesMask' ]
            if len(self.pulsemapnames) > self.maxnpm:
                self.log_fatal("Pulse map check data is bulky. If you really want to check %d maps, "
                               "then raise the maximum with the 'maxnpm' constructor argument (currently %d)"
                               % (len(self.pulsemapnames) , self.maxnpm),unit="PulseMapChecker")
        list_of_testvalues=[]
        for pmname in self.pulsemapnames:
            pulsemap=dataclasses.I3RecoPulseSeriesMap.from_frame(frame,pmname)
            lostq=[] # List of OM, String, Time, Charge (not: "lost charge")
            for k,v in pulsemap.items():
                for pulse in v:
                    lostq.append([k.om,k.string,pulse.time,pulse.charge])
            aostq=np.array(lostq) # array of OM, String, Time, Charge
            list_of_testvalues.append(aostq)
        return list_of_testvalues
    def compare_test_values(self,val,ref):
        self.ncheck += 1
        icetray.logging.log_debug("PulseMapChecker.compare_test_values: frame nr. %d val[0]=%s ref[0]=%s" % (self.ncheck,val[0],ref[0]),unit="PulseMapChecker")
        if not (len(val) == len(self.pulsemapnames) and len(ref) == len(self.pulsemapnames) ):
            icetray.logging.log_debug("programming error!",unit="PulseMapChecker")
            return("ERROR: got #pulsemaps=%d, #refpulsemaps=%d, expected %d" % (len(val),len(ref),len(self.pulsemapnames)))
        problems=[]
        for v,r,name in zip(val,ref,self.pulsemapnames):
            if not np.shape(v)==np.shape(r):
                icetray.logging.log_debug("inconsistent number of pulses in frame %d" % self.ncheck,unit="PulseMapChecker")
                problems.append("%s: number of pulses don't match %s vs %s" % (name,np.shape(v),np.shape(r)))
                continue
            qt_close = isclose(v,r,rtol=0.0,atol=0.01)
            if np.all(qt_close):
                continue
            icetray.logging.log_debug("charges/times do not match in frame %d" % self.ncheck,unit="PulseMapChecker")
            problems.append("%s: times/charges don't match (%d mismatches)" % (name,np.sum(qt_close==False)))
        return problems

