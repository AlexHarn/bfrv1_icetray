from icecube.production_histograms.histograms.histogram import Histogram
from icecube import icetray, dataclasses
import math
class Livetime(Histogram):
    def __init__(self):
        Histogram.__init__(self, 0., 2., 100, "Livetime")
        self.lastTime=0
        self.lastEvh=dataclasses.I3EventHeader
    def DAQ(self,frame):
        evh=frame["I3EventHeader"]
        time=evh.start_time.mod_julian_day+evh.start_time.mod_julian_sec/(24.*3600.)+(evh.start_time.mod_julian_nano_sec*math.pow(10,-9))/(24.*3600.)
        deltaT = (time - self.lastTime)*24.*3600. # in seconds                                                                                                                                              
        if deltaT<0:
            if self.lastEvh.start_time.is_leap_second:
                deltaT +=1
                if deltaT<0:
                    icetray.logging.log_fatal("negative deltaT")
            else:
                icetray.logging.log_fatal("negative deltaT")
        self.lastTime=time
        self.lastEvh=evh
        self.fill(deltaT)
