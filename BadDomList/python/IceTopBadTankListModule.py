from icecube import dataclasses, icetray
from .utils import is_ice_top

class IceTopBadTankListModule(icetray.I3Module):
    def __init__(self, context):
        super(IceTopBadTankListModule, self).__init__(context)
        
        self.AddParameter('IceTopBadDOMListName',
                          'Name of the IceTop bad DOM list',
                          'IceTopBadDOMs')
        
        self.AddParameter('ListName',
                          'Name of the output bad tanks list',
                          'IceTopBadTanks')
        
        self.AddOutBox('OutBox')
    
    def Configure(self):
        self.bad_dom_list = self.GetParameter('IceTopBadDOMListName')
        self.output_list  = self.GetParameter('ListName')
    
    def DetectorStatus(self, frame):
        '''
        Reproduce what I3IceTopSanityChecks did here:
        check that there is at least a high-gain DOM in the tank!
        (I3IceTopSanityChecks in addition tested if the geometry was sane (i.e.
        the DOMs are really inide the tank, etc. Hopefully this is not needed any more)

        Args:
            frame (icecube.icetray.I3Frame): The detector status frame
        '''

        bad_doms = list()
        if not frame.Has(self.bad_dom_list):
            icetray.logging.log_warn('{0} not in frame.'.format(self.bad_dom_list))
        else:
            bad_doms = frame[self.bad_dom_list]
        
        dom_status = frame['I3DetectorStatus'].dom_status
        tank_map = dict()
        for om_key, stat in dom_status:
            if not is_ice_top(om_key):
                continue
            if om_key in bad_doms:   # ignore bad DOMs
                continue
            tank_key = dataclasses.TankKey(om_key)
            if not tank_key in tank_map.keys():
                tank_map[tank_key] = list()
            tank_map[tank_key].append(stat)
        
        bad_tanks = dataclasses.I3VectorTankKey()
        for tank_key in tank_map.keys():
            stats = tank_map[tank_key]
            if len(stats) != 2:
                icetray.logging.log_warn('{0} good DOMs found in tank {1}.'.format(len(stats), tank_key))
            
            count_HG = 0
            for s in stats:
                if s.dom_gain_type == dataclasses.I3DOMStatus.High:
                    count_HG += 1
            if count_HG != 1:
                icetray.logging.log_error('{0} high-gain DOMs found in tank {1}. Adding to bad tank list.'.format(count_HG, tank_key))
                bad_tanks.append(tank_key)
        
        frame.Put(self.output_list, bad_tanks)
        self.PushFrame(frame)
