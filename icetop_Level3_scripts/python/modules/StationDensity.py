from icecube import icetray,dataclasses
import math

##############################################################################################
# Ratio of hit stations to total nstations in circle around COG (with R to furthest tank) #
##############################################################################################
class StationDensity(icetray.I3Module):
    def __init__(self, context):
        icetray.I3Module.__init__(self, context)
        self.AddParameter('InputITpulses','Which IceTop Pulses to use',0)
        self.AddParameter('InputCOG','Which IceTop COG to use',0)
        self.AddOutBox("OutBox")
        
    def Configure(self):
        self.pulses = self.GetParameter('InputITpulses')
        self.cog = self.GetParameter('InputCOG')

    def Geometry(self, frame):
        if 'I3Geometry' in frame:
            geom = frame['I3Geometry']
            self.stageo = geom.stationgeo
        else:
            print 'No geometry found'
        self.PushFrame(frame,"OutBox")
    
    def Physics(self, frame):
        if self.cog in frame:
            cog = frame[self.cog]
            if 'I3EventHeader' in frame:
                evhead = frame['I3EventHeader']
                run = evhead.run_id
                event = evhead.event_id
                R_om = []
                if self.pulses in frame:
                    itvem = frame[self.pulses]
                    if itvem.__class__ == dataclasses.I3RecoPulseSeriesMapMask:
                        itvem = itvem.apply(frame)
                    
                    largeDist = 0
                    nstation = 0
                    nTotalSta = 0
                    for sta, stageos in self.stageo:
                        #stageos = [tankA,tankB]
                        #dist = (stageos[0].position.calc_distance(cog.pos) + stageos[1].position.calc_distance(cog.pos))/2. #calc_distance seemed to ahve disappeared from most recent dataclasses...
                        dist_A=math.sqrt(math.pow(stageos[0].position.x-cog.pos.x,2)+math.pow(stageos[0].position.y-cog.pos.y,2)+math.pow(stageos[0].position.z-cog.pos.z,2))
                        dist_B=math.sqrt(math.pow(stageos[1].position.x-cog.pos.x,2)+math.pow(stageos[1].position.y-cog.pos.y,2)+math.pow(stageos[1].position.z-cog.pos.z,2))
                        dist=(dist_A+dist_B)/2.0
                        R_om.append((dist,sta))
                        # if station is hit => one of the DOMs in tank is hit
                        if icetray.OMKey(sta,61) in itvem or icetray.OMKey(sta,62) in itvem:
                            nstation +=1
                            if dist > largeDist:
                                largeDist = dist
                                
                    #print sorted(R_om)
                    #print nstation, largeDist
                    withinRlist = []
                    for tup in sorted(R_om):
                        if tup[0] <= largeDist+0.1:   #+epsilon with e=0.1
                            withinRlist.append(tup[1])
                    #print withinRlist
                    #print set(withinRlist)
                    nTotalSta = len(set(withinRlist))
                    if nstation <= 0 or nTotalSta <= 0:
                        #print 'Oh no, nstation %i ntotalSta %i, dist %f' % (nstation,nTotalSta,largeDist)
                        staDens = -1.
                    else :
                        staDens = float(nstation)/float(nTotalSta)
                    if staDens > 1.:
                        print nTotalSta
                        print nstation, largeDist
                        print sorted(R_om)
                    #print 'StationDensity is ',staDens
                    frame["StationDensity"] = dataclasses.I3Double(staDens)
        self.PushFrame(frame,"OutBox")
