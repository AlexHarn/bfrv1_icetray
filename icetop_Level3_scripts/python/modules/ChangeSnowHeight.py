from icecube import icetray, dataclasses
from icecube.icetray.i3logging import log_info, log_warn, log_debug, log_trace

class ChangeSnowHeight(icetray.I3ConditionalModule):
    def __init__(self, ctx):
        icetray.I3ConditionalModule.__init__(self,ctx)
        self.AddParameter('SnowTablesFile','File that contains snow tables')
        self.AddOutBox("OutBox")

    def Configure(self):
        import csv
        import os
        self.snowfile = self.GetParameter('SnowTablesFile')
        if not os.path.exists(self.snowfile):
            raise Exception('The snow tables file, %s, does not exist.'%self.snowfile)
        log_debug("Using snow tables from %s"%self.snowfile)
        self.newheights = None
        self.year = None
        self.month = None

    def Geometry(self, frame):
        if 'I3Geometry' in frame:
            geom = frame['I3Geometry']
            t = geom.start_time
            if t.utc_year != self.year or t.utc_month != self.month:
                log_debug( "Updating snow heights for %s"%t.get_utc_string('%B %d, %Y'))
                self._readTable(t.utc_year, t.utc_month)
            stageo = geom.stationgeo
            for e,st in stageo:
                updated_heights = dataclasses.I3StationGeo()
                if e not in self.newheights:
                    log_warn('Did not find station %s in new snowheight dict'%e)
                    continue
                #ok we have I3TankGeo here... look it up
                ## assume first is A and second is B
                st[0].snowheight = max((self.newheights[e][0],0.))
                st[1].snowheight = max((self.newheights[e][1],0.))
                updated_heights.append(st[0])
                updated_heights.append(st[1])
                stageo[e] = updated_heights
            frame.Delete('I3Geometry')
            frame['I3Geometry'] = geom
        else:
            log_warn('No geometry found')
        self.PushFrame(frame,"OutBox")


    def _readTable(self, year, month):
        import csv
        f = open(self.snowfile)
        ## error catching should go here...

        newheights = {}
        for row in csv.reader(f,delimiter=' '):
            if len(row) > 1 :        ## skips first line
                height = float(row[len(row)-1])
                tank = row[0]
                station = int(tank[:2])
                id = tank[2]
                if station not in newheights:
                    newheights[station] = [0,0]
                if id == 'A':
                    newheights[station][0] = height
                elif id == 'B':
                    newheights[station][1] = height
                else:
                    log_warn('unknown tank id: %s'%id)

        self.newheights = newheights
        self.year = year
        self.month = month
