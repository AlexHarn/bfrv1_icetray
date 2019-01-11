############################
# parser Options
############################
import optparse

parser= optparse.OptionParser()
parser.add_option('--inputfile',type=str,dest='INPUTFILE',
                  help='Input i3.bz2 file. Default file is given if you are in madison cluster',
                  default='/data/exp/IceCube/2011/filtered/level2/0731/Level2_IC86.2011_data_Run00118514_Part00000085.i3.bz2')

options =parser.parse_args()[0]

##############################
# IceTray
#############################
import I3Tray
from icecube import dataio, dataclasses, icetray

#Create an instance of I3Tray
tray= I3Tray.I3Tray()

# in dataio:
tray.AddModule("I3Reader", "reader", 
               Filename=options.INPUTFILE)

from icecube import DomTools
tray.AddModule('I3LCCleaning','I3LCCleaning',
               InIceInput='InIceRawData',           # InIceRawData is one of the first objects obtained from the South-Pole
               InIceOutput='MyHLCInIceLaunches',    # high local coincidence launches
               InIceOutputSLC='MySLCInIceLaunches') # soft local coincidence launches

#Note: InIceRawData, MyHLCInIceLaunches and MySLCInIceLaunches are objects of type: I3DOMLaunchSeriesMap, which lives in dataclasses

#Check that the object that you put is indeed in the frame, 
#and calculate a delta time between the first and last launch of the event
def CheckThings(frame):
    if not frame.Has('MyHLCInIceLaunches'): # you can use the more python-style instead: if not 'MyHLCInIceLaunches' in frame
        print "frame does not contain MyHLCInIceLaunches, I skip this frame"
        return False #skip frame

    timelist=[]# standard python list
    launchesmap=frame.Get('MyHLCInIceLaunches') # you can use the more python-style instead: frame['MyHLCInIceLaunches']
    for launches in launchesmap.values():       # "launches" is a list of Launches that live in dataclasses, called: I3DOMLaunchSeries
        timelist+= [launche.time for launche in launches] # You store all the time launches of the event (frame) in a large python list
        
    if len(timelist)!=0: 
        timelist.sort()
        firstlaunch=timelist[0]
        lastlaunch=timelist[-1]
        print 'The total number of launches in this frame is {0} and the delta time is {1}'.format(len(timelist),lastlaunch-firstlaunch)
    else: # just for fun:
        icetray.logging.log_warn('No launches in MyHLCInIceLaunches')

tray.AddModule(CheckThings,'CheckThings')

tray.Execute()


