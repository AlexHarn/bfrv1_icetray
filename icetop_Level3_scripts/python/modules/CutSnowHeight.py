from icecube import icetray, dataclasses

class CutSnowHeight(icetray.I3Module):
    """
    Module to cut the snow height in case we suffered from the observation level bug
    """
    def __init__(self, ctx):
        icetray.I3Module.__init__(self, ctx)
        self.AddOutBox("OutBox")
        self.AddParameter("ObservationHeight", "Observation height setting in CORSIKA", 2834.)
        
    def Configure(self):
        self.obsHeight=self.GetParameter("ObservationHeight")
    
    def Geometry(self,frame):
        geo=frame["I3Geometry"]
        frame.Delete("I3Geometry")
        maxHeight=self.obsHeight-dataclasses.I3Constants.OriginElev
        for station, stageo in geo.stationgeo:
            updated_heights = dataclasses.I3StationGeo()
            for tank in stageo:
                height_tank=tank.position.z+(tank.tankheight-tank.fillheight)
                
                # If the tank height is above the maximum level, put the snow height to 0. 
                # If the snow is above maxHeight, make the snow go maximally up to the maximal height. 
                if height_tank>maxHeight:
                    newSnowHeight=0
                elif height_tank+tank.snowheight>maxHeight and height_tank<maxHeight:
                    newSnowHeight=maxHeight-height_tank
                else:
                    newSnowHeight=tank.snowheight
                tank.snowheight=round(newSnowHeight,3)   # round to 1 cm
                updated_heights.append(tank)
            geo.stationgeo[station]=updated_heights
            
        frame.Put("I3Geometry",geo)

        self.PushFrame(frame,"OutBox")
        
