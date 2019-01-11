from icecube import dataclasses, icetray
from .utils import is_ice_top

class IceTopBadDomListModule(icetray.I3Module):
    def __init__(self, context):
        super(IceTopBadDomListModule, self).__init__(context)
        
        self.AddParameter('BadDomListName',
                          'The full bad DOMs list, including IceTop and in-ice',
                          'BadDomsList')
        
        self.AddParameter('ListName',
                          'Name of the output bad DOMs list, IceTop only',
                          'IceTopBadDOMs')
        
        self.AddOutBox('OutBox')
    
    def Configure(self):
        self.input_list  = self.GetParameter('BadDomListName')
        self.output_list = self.GetParameter('ListName')
    
    def DetectorStatus(self, frame):
        """
        Adds a list of bad iceTop doms to the D frame.

        The source is usually the already created BadDomsList (see BadDomListModule).
        It just filters the IceTop doms out and stores them in this list.

        Args:
            frame (icecube.icetray.I3Frame): The detector status frame
        """

        if not frame.Has(self.input_list):
            icetray.logging.log_warn('{0} not in frame. Not generating {1}.'.format(self.input_list, self.output_list))
            self.PushFrame(frame)
            return
        
        bd_all = frame[self.input_list]
        bd_it = dataclasses.I3VectorOMKey()

        for key in bd_all:
            if is_ice_top(key):
                bd_it.append(key)

        frame.Put(self.output_list, bd_it)
        self.PushFrame(frame)
