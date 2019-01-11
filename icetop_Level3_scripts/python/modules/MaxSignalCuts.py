from icecube.icetray import I3ConditionalModule, I3ConditionalModule, I3Frame
from icecube import dataclasses
from icecube.dataclasses import I3RecoPulseSeriesMapMask, I3EventHeader
from icecube.icetray.i3logging import log_warn, log_fatal
import math

class MaxSignalCuts(I3ConditionalModule):
    """
    I3Module to do simple standard cuts based on the maximum signal. The main check is that the largest signal is not on the edge of the array.
    """
    def __init__(self, ctx):
        try:
            I3ConditionalModule.__init__(self, ctx)
            self.AddParameter("Pulses", "Pulse lists to look for the largest signal", ["CleanedHLCTankPulses"])
            self.AddParameter("MinMaxSignal", "Minimum value for the largest signal (in VEM).", 6)
            self.AddParameter("MinSignalNeighbourTankMax", "Minimum value for the signal in the neighbouring tank of the largest signal (in VEM).", 4)
            self.AddParameter("Detector", "Detector configuration (IC79 or IC86). It uses the GCD to determine it if nothing is specified.", None)
            self.interactive = False
            self.AddOutBox("OutBox")
        except:
            self.interactive = True
            self.detector = None
            self.pulses = ["CleanedHLCTankPulses"]
            self.min_max = 6
        
    def Configure(self):
        self.pulses = self.GetParameter("Pulses")
        self.detector = self.GetParameter("Detector")
        self.min_max = self.GetParameter("MinMaxSignal")
        self.staGeo=None
        self.edge=None
        
    def Geometry(self, frame):
        # this method gets called only when there is a G frame instead of calling it for all P frames
        self.staGeo = frame['I3Geometry'].stationgeo
        if len(self.staGeo) == 81 or self.detector == "IC86":
            self.edge = [1, 2, 3, 4, 5, 6, 7, 13, 14, 21, 22, 30, 31, 40, 41, 50, 51, 59, 60, 67, 68, 72, 73, 74, 75, 76, 77, 78]
        elif len(self.staGeo) == 73 or self.detector == "IC79":
            # this also contains the edge of I81 since if the closest string is an IT81 string, then it is not contained in IT73
            self.edge = [1, 2, 3, 4, 5, 6, 7, 8, 13, 14, 15, 21, 22, 23, 30, 31, 32, 40, 41, 50, 51, 59, 60, 67, 68, 72, 73, 74, 75, 76, 77, 78]
        else:
            raise RuntimeError("MaxSignalCuts module only knows how to handle IC79 and IC86")

        self.PushFrame(frame)

    def Physics(self, frame):
        from icecube import icetray

        if (self.edge is None or self.staGeo is None):
            raise RuntimeError("Geometry not set!")
      
        largest_signal = 1e-30
        largest_om = None
        for pulses_name in self.pulses:
            if pulses_name in frame:
                pulses = dataclasses.I3RecoPulseSeriesMap.from_frame(frame,pulses_name)
                if len(pulses.keys())>0:   ## is 0 for the Snow corrected pulses if the reconstruction failed.
                    for k,l in pulses.iteritems():
                        for p in l:
                            # We will neglect what happens with NaNs... these should be rare, no?
                            if p.charge!=p.charge:
                                continue
                            if p.charge > largest_signal:
                                largest_signal = p.charge
                                largest_om = k

        if not largest_om is None:

            largest_tank=dataclasses.TankKey(largest_om)
            # Now get the other tank's charge in that station, if there. (For the possible cut that this has to be bigger than 4).
            # If the tank is not in the above pulse series, put 0.
            # If the charge in the tank is NaN, we'll put the charge of the largest tank in there (so the other tank in the station). This is not perfect, but the best we can do? We don't want these events to be cut because of NaN. Be careful with this.
            tank_neighbour=dataclasses.TankKey.TankA
            if largest_tank.tank==dataclasses.TankKey.TankA:
                tank_neighbour=dataclasses.TankKey.TankB
            neighbour_largest_tank=dataclasses.TankKey(largest_tank.string,tank_neighbour)
            oms_in_tank=[]
            if tank_neighbour==dataclasses.TankKey.TankA:
                oms_in_tank=[icetray.OMKey(largest_tank.string,61,0),icetray.OMKey(largest_tank.string,62,0)]
            elif tank_neighbour==dataclasses.TankKey.TankB:
                oms_in_tank=[icetray.OMKey(largest_tank.string,63,0),icetray.OMKey(largest_tank.string,64,0)]
            else:
                log_fatal("Something wrong here.")

            # Find pulse. 
            neighbour_largest_tank_charge=largest_signal
            all_oms= [om_here for pulses_name in self.pulses for om_here in dataclasses.I3RecoPulseSeriesMap.from_frame(frame,pulses_name).keys()]
            for om in oms_in_tank:  # Only one of the two should be there.
                if om in all_oms:
                    for pulses_name in self.pulses:
                        pulses = dataclasses.I3RecoPulseSeriesMap.from_frame(frame,pulses_name)
                        for k, l in pulses.iteritems():
                            if k==om:
                                for p in l:
                                    if p.charge==p.charge:
                                        neighbour_largest_tank_charge=p.charge
                                    else:
                                        neighbour_largest_tank_charge=largest_signal
                                break
                    break

            frame.Put("IceTopMaxSignalTank", dataclasses.I3String(largest_tank.tank.name))
            frame.Put("IceTopMaxSignalString", icetray.I3Int(largest_om.string))
            frame.Put("IceTopMaxSignal", dataclasses.I3Double(largest_signal))
            frame.Put("IceTopMaxSignalInEdge", icetray.I3Bool(largest_om.string in self.edge))
            frame.Put("IceTopNeighbourMaxSignalTank", dataclasses.I3String(neighbour_largest_tank.tank.name))
            frame.Put("IceTopNeighbourMaxSignal", dataclasses.I3Double(neighbour_largest_tank_charge))

        if not self.interactive:
            self.PushFrame(frame)
