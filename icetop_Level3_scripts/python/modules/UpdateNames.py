from icecube.icetray import I3ConditionalModule, I3Frame
from icecube import dataclasses
from icecube.dataclasses import I3RecoPulseSeriesMapMask, I3EventHeader
from icecube.icetray.i3logging import log_info, log_warn, log_debug, log_trace
from icecube.icetop_Level3_scripts.icetop_globals import names

class UpdateNames(I3ConditionalModule):
    """
    I3ConditionalModule to change names from older naming conventions to newer ones, to make life easier later.
    """
    def __init__(self, ctx):
        I3ConditionalModule.__init__(self, ctx)
        self.AddOutBox("OutBox")
        self.AddParameter("Detector", "Detector configuration (IC79 or IC86.20*).")
        
    def Configure(self):
        self.detector = self.GetParameter("Detector")
        self.l2_names = names[self.detector]
        self.l3_names = names['Level3']
        
        # Make a list of names which possibly need to be changed, if their name is different in L2 compared to L3.
        # For the subevent stream we make a boolean. 
        # Only for TankPulseMergerExcludedStations and ClusterCleaningExcludedStations (for IC79 and IC86.2011) we need to add them to another list, 
        # since they need to be converted from a station list to a tank list.
        import re
        self.names={}
        self.excluded_stations={}
        self.change_sub_event_stream=False
        for key in self.l3_names:
            if self.l2_names[key]!=self.l3_names[key]:
                if key=="sub_event_stream":
                    self.change_sub_event_stream=True
                if key=="TankPulseMergerExcluded" or key=="ClusterCleaningExcluded":
                    if re.search('station', self.l2_names[key].lower()):
                        self.excluded_stations[self.l2_names[key]]=self.l3_names[key]
                    else:
                        self.names[self.l2_names[key]]=self.l3_names[key]
                else:
                    self.names[self.l2_names[key]]=self.l3_names[key]

        # Maps to consider if one of the sources would have changed. Maybe we need IceTopPulses? 
        # We need to separate this between P and Q apparently, otherwise this seems (badly) repeated in the P frame.
        self.QMasksOrUnions=[self.l3_names["InIcePulses"]]
        self.PMasksOrUnions=[self.l3_names["CleanHLCPulses"]] 

    def Physics(self, frame):
        
        # rename sub_event_stream name
        if self.change_sub_event_stream:
            h = frame['I3EventHeader']
            h.sub_event_stream = self.l3_names["sub_event_stream"]
            frame.Delete('I3EventHeader')
            frame['I3EventHeader'] = h

        # rename pulse collections
        self._rename_collections_(frame, 'Physics')

        self.PushFrame(frame)
        

    def DAQ(self, frame):
        self._rename_collections_(frame, 'DAQ')

        # Do the station to tank conversion if needed.
        for l2_excluded in self.excluded_stations.keys():
            v = dataclasses.TankKey.I3VectorTankKey()
            for s in frame[l2_excluded]:
                v.append(dataclasses.TankKey(s, dataclasses.TankKey.TankA))
                v.append(dataclasses.TankKey(s, dataclasses.TankKey.TankB))
            frame[self.excluded_stations[l2_excluded]] = v
        
        # if InIcePulses is a union, make it a recoPulseMap. This happens in data.
        # Also, if it's a mask, make it a map. This seems to happen in simulations.
        if type(frame[self.l3_names['InIcePulses']])==dataclasses.I3RecoPulseSeriesMapUnion or type(frame[self.l3_names['InIcePulses']])==dataclasses.I3RecoPulseSeriesMapMask:
            iipulses=frame[self.l3_names['InIcePulses']].apply(frame)
            frame.Delete(self.l3_names['InIcePulses'])
            frame.Put(self.l3_names['InIcePulses'],iipulses)
            
        self.PushFrame(frame)


    def _rename_collections_(self, frame, tag):
        """
        Rename collections.
        """
        # rename (will not overwrite)
        
        for old, new in self.names.items():
            if old in frame and not new in frame:
                log_trace('%s: %s -> %s'%(tag, old, self.names[old]))
                frame.Rename(old, self.names[old])
        
        # check map mask sources
        toCheck=[]
        
        ## toDo: check tag instead!! 
        if frame.Stop==I3Frame.DAQ:
            toCheck=self.QMasksOrUnions
        if frame.Stop==I3Frame.Physics:
            toCheck=self.PMasksOrUnions
        for name in toCheck:
            try:
                frame[name]
            except:
                continue
            if type(frame[name]) == dataclasses.I3RecoPulseSeriesMapMask:
                source = frame[name].source
                if source in self.names.keys() and source!=self.names[source]:
                    log_trace('%s: re-sourcing reco pulse map mask %s: %s -> %s'%(tag, name, source, self.names[source]),"UpdateNames")
                    # self.names[source] needs to be there already
                    m2 = dataclasses.I3RecoPulseSeriesMapMask(frame, self.names[source])
                    # the following is instead of set_none()
                    pulseseries = dataclasses.I3RecoPulseSeriesMap.from_frame(frame, m2.source)
                    for k in pulseseries.keys():
                        for j in range(len(pulseseries[k])):
                            m2.set(k, j, False)
                    # now set the True ones
                    c = frame[name]
                    keys = pulseseries.keys()
                    for i in range(len(c.bits)):
                        for j in range(len(c.bits[i])):
                            m2.set(keys[i], j, c.bits[i][j])
                    # and replace
                    frame.Delete(name)
                    frame[name] = m2
                    
               
            elif type(frame[name])==dataclasses.I3RecoPulseSeriesMapUnion:
                log_trace('%s: re-sourcing reco pulse map union %s '%(tag, name),"UpdateNames")
                newSources=[]
                for source in frame[name].sources:
                    if source in self.names.keys():
                        log_trace('%s: re-sourcing reco pulse map union %s: source %s -> %s'%(tag, name, source, self.names[source]),"UpdateNames")
                        newSources.append(self.names[source])
                    else:
                        newSources.append(source)
                frame.Delete(name)
                newMapUnion=dataclasses.I3RecoPulseSeriesMapUnion(frame,newSources)
                frame[name]=newMapUnion
                
                
