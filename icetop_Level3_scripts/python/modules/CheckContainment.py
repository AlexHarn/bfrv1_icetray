from icecube.icetray import I3ConditionalModule, I3Frame
from icecube import dataclasses, phys_services
from icecube.dataclasses import I3RecoPulseSeriesMapMask, I3EventHeader
from math import isnan


class CheckContainment(I3ConditionalModule):
    """
    I3Module to check if a list of I3Particles is contained within IceTop.
    Two kinds of containment are defined: OnionContainment (number) and FractionContainment (number). + InFillContainment (bool). 
    """
    def __init__(self, ctx):
        try:
            I3ConditionalModule.__init__(self, ctx)
            self.AddParameter("Particles", "List of IDs of I3Particles", ["LaputopStandard","LaputopSmallShower"])
            self.AddParameter("Detector", "Detector configuration (IC79 or IC86). It uses the GCD to determine it if nothing is specified.", None)
            self.AddOutBox("OutBox")
            self.interactive = False
        except:
            self.detector = None
            self.interactive = True
            self.particle = ["LaputopStandard","LaputopSmallShower"]
        self.geometry = None
        self.regular_grid = None
        self.infill_grid = None
        
    def Configure(self):
        self.particle = self.GetParameter("Particles")
        self.detector = self.GetParameter("Detector")
        self.scaling=0

    def Geometry(self, frame):
        import numpy
        # this method gets called only when there is a G frame instead of calling it for all P frames
        geometry = frame['I3Geometry'].stationgeo
        a = [(string, tank, geometry[string][tank].position.x, geometry[string][tank].position.y, geometry[string][tank].position.z) for string in geometry.keys() for tank in range(2)]
        self.geometry = numpy.array(a, dtype=([('string',int), ('tank', int), ('x', float), ('y', float), ('z', float) ]))
        self.infill_strings = [26, 27, 36, 37, 46, 79, 80, 81]
        self.infill_grid = self.geometry[(self.geometry['string']>=79)*(self.geometry['string']==26)*(self.geometry['string']==36)*(self.geometry['string']==46)]
        self.regular_grid = self.geometry[self.geometry['string']<79]
        self.mean_z = numpy.mean(self.geometry['z'])
        if len(geometry) == 81 or self.detector == "IC86":
            self.edge = [[1, 2, 3, 4, 5, 6, 7, 13, 14, 21, 22, 30, 31, 40, 41, 50, 51, 59, 60, 67, 68, 72, 73, 74, 75, 76, 77, 78],
                         [8, 15, 23, 32, 42, 52, 61, 69, 70, 71, 64, 65, 66, 58, 49, 39, 29, 20, 12, 11, 10, 9],
                         [16, 24, 33, 43, 53, 62, 63, 55, 56, 57, 48, 38, 28, 19, 18, 17],
                         [25, 34, 44, 54, 45, 46, 47, 37, 27, 26],
                         [35, 36]
                         ]
        elif len(geometry) == 73 or self.detector == "IC79":
            # this also contains the edge of I81 since if the closest string is an IT81 string, then it is not contained in IT73
            self.edge = [[1, 2, 3, 4, 5, 6, 7, 8, 13, 14, 15, 21, 22, 23, 30, 31, 32, 40, 41, 50, 51, 59, 60, 67, 68, 72, 73, 74, 75, 76, 77, 78],
                         [16, 24, 33, 42, 52, 61, 69, 70, 71, 64, 65, 66, 58, 49, 39, 29, 20, 12, 11, 10, 9],
                         [25, 34, 43, 53, 62, 63, 55, 56, 57, 48, 38, 28, 19, 18, 17],
                         [35, 44, 54, 45, 46, 47, 37, 27, 26],
                         [36]
                         ]
        else:
            raise RuntimeError("CheckContainment module only knows how to handle IC79 and IC86 (%s)"%len(geometry))
        # For FractionContainment
        self.scaling = phys_services.I3ScaleCalculator(frame['I3Geometry'])
        
        self.PushFrame(frame)

    def Physics(self, frame):
        from icecube import icetray

        if self.regular_grid is None:
            raise RuntimeError("Geometry not set!")

        for particle in self.particle:
            if not particle in frame:
                continue
            part = frame[particle]
            if isnan(part.pos.x) or isnan(part.pos.y):
                # particle has no core, return dummy values
                frame.Put(particle+"_NearestStation", icetray.I3Int(0))
                frame.Put(particle+"_OnionContainment", icetray.I3Int(0))
                if self.detector!='IC79':
                    frame.Put(particle+"_NearestStationIsInfill", icetray.I3Bool(False))
                frame.Put(particle+"_FractionContainment", dataclasses.I3Double(float("nan")))
            else:
                # particle is ok, do containment checks
                string = self.GetNearestString(part, self.regular_grid)
                frame.Put(particle+"_NearestStation", icetray.I3Int(string))
                layer = None
                for i in range(len(self.edge)):
                    if string in self.edge[i]:
                        layer = i
                        break
                if layer is None:
                    log_warn('Closest string (%s) is not in any edge. How is that?'%string)
                    frame.Put(particle+"_OnionContainment", icetray.I3Int(0))
                else:
                    frame.Put(particle+"_OnionContainment", icetray.I3Int(layer))
                if self.detector!='IC79':
                    frame.Put(particle+"_NearestStationIsInfill", icetray.I3Bool(string in self.infill_strings))
                frame.Put(particle+"_FractionContainment",dataclasses.I3Double(self.scaling.scale_icetop(part)))
        if not self.interactive:
            self.PushFrame(frame)


    def GetNearestString(self, particle, geometry):
        from icecube.icetray import I3Units
        import numpy
        pos = particle.pos
        if pos.z < 1944.5*I3Units.m or pos.z > 1950.1*I3Units.m:
            # this is roughly the point where the axis intersects the IceTop plane
            dz = (pos.z - self.mean_z)/particle.dir.z
            pos.x = pos.x - dz*particle.dir.x
            pos.y = pos.y - dz*particle.dir.y
            pos.z = self.mean_z

        # try to reduce the set of tanks first.
        closest = geometry[(numpy.abs(geometry['x']-particle.pos.x)<150*I3Units.m)*(numpy.abs(geometry['y']-particle.pos.y)<150*I3Units.m)]
        if len(closest) == 0:
            closest = geometry
        i = numpy.argmin((closest['x']-particle.pos.x)**2+(closest['y']-particle.pos.y)**2)
        return int(closest[i]['string'])

