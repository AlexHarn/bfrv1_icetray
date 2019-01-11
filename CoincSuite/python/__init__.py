from icecube.load_pybindings import load_pybindings
load_pybindings(__name__,__path__)
del load_pybindings

from icecube import icetray, dataclasses

#we need to have this function (ORIGINAL: http://code.icecube.wisc.edu/svn/projects/filter-2012/trunk/python/Globals.py)
def which_split(frame, split_name=None, keep_q=False):
  """
  A function that does only returns true for frames of the specified stream.
  Use this for example to let Modules only execute on a specific stream

  :param split_name: Name of the sub_event_stream select
  :param keep_q: should the Q-Frame also return true?
  """
  if frame.Stop == icetray.I3Frame.Physics:
    if frame['I3EventHeader'].sub_event_stream == split_name:
      return 1
    else:
      return 0
  elif keep_q and frame.Stop == icetray.I3Frame.DAQ:
    return 1
  else:
    return 0

class Stepper(icetray.I3Module):
  """
  Convenience function for debugging:
  automatically prints out for every DAQ-frame a increasing count, its run_id and event_id
  """
  def __init__(self, ctx):
    icetray.I3Module.__init__(self, ctx)
    self.AddOutBox("OutBox")
    self.count=0
  def Configure(self):
    pass # icetray.I3Module.Configure(self)
  def DAQ(self,frame):
    self.count+=1
    icetray.logging.log_debug("Count:"+str(self.count)+" Run:"+str(frame["I3EventHeader"].run_id)+" Event:"+str(frame["I3EventHeader"].event_id))
    self.PushFrame(frame)

#import everything from the main segments
from .coincsuite import *
from .mcidentification import *
from .filters import *
from .resplit import *
