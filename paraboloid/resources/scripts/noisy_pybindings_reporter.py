#!/usr/bin/env python

import sys
from icecube.icetray import I3Units

if __name__ == "__main__":
    print("this is an auxiliary script for rudimentary paraboloid testing")
    sys.exit(0)

from icecube import icetray,dataclasses,paraboloid

class noisy_pybindings_reporter(icetray.I3ConditionalModule):
    def __init__(self,ctx):
        super(noisy_pybindings_reporter,self).__init__(ctx)
        self.Pcount=0
        self.PBFcount=0
        self.OKPBFcount=0
        self.statusdict={
          paraboloid.I3ParaboloidFitParams.PBF_UNDEFINED:"PBF_UNDEFINED",
          paraboloid.I3ParaboloidFitParams.PBF_NO_SEED:"PBF_NO_SEED",
          paraboloid.I3ParaboloidFitParams.PBF_INCOMPLETE_GRID:"PBF_INCOMPLETE_GRID",
          paraboloid.I3ParaboloidFitParams.PBF_FAILED_PARABOLOID_FIT:"PBF_FAILED_PARABOLOID_FIT",
          paraboloid.I3ParaboloidFitParams.PBF_SINGULAR_CURVATURE_MATRIX:"PBF_SINGULAR_CURVATURE_MATRIX",
          paraboloid.I3ParaboloidFitParams.PBF_SUCCESS:"PBF_SUCCESS",
          paraboloid.I3ParaboloidFitParams.PBF_NON_POSITIVE_ERRS:"PBF_NON_POSITIVE_ERRS",
          paraboloid.I3ParaboloidFitParams.PBF_NON_POSITIVE_ERR_1:"PBF_NON_POSITIVE_ERR_1",
          paraboloid.I3ParaboloidFitParams.PBF_NON_POSITIVE_ERR_2:"PBF_NON_POSITIVE_ERR_2",
          paraboloid.I3ParaboloidFitParams.PBF_TOO_SMALL_ERRS:"PBF_TOO_SMALL_ERRS",
        }
    def Configure(self):
        pass
    def Physics(self,frame):
        self.Pcount+=1
        icetray.logging.log_notice("%dth P frame" % self.Pcount, unit="noisy_pybindings_reporter")
        for o in frame.keys():
            if frame.type_name(o) == 'I3ParaboloidFitParams':
                pbfparams=frame[o]
                self.PBFcount+=1
                deg=I3Units.degree
                icetray.logging.log_notice("%12s status=%s Z,A=(%.1f,%.1f) err12a=(%.2g,%.2g,%.2f) chi2=%.3g" %
                (o, self.statusdict.get(pbfparams.pbfStatus,"?"),
                    pbfparams.pbfZen/deg, pbfparams.pbfAzi/deg,
                    pbfparams.pbfErr1/deg, pbfparams.pbfErr2/deg, pbfparams.pbfRotAng/deg, pbfparams.pbfChi2),
                    unit="noisy_pybindings_reporter")
                self.OKPBFcount+=1
    def Finish(self):
        icetray.logging.log_notice("Got %d frames, %d PBF objects, %d were successful" % (self.Pcount,self.PBFcount,self.OKPBFcount), unit="noisy_pybindings_reporter")
        if 0 == self.PBFcount or self.OKPBFcount < 0.5*self.PBFcount:
            icetray.logging.log_warn("That does not look quite right.",unit="noisy_pybindings_reporter")
