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
tray.AddModule('I3FirstPulsifier','I3FirstPulsifier',
               InputPulseSeriesMapName='OfflinePulses',   
               OutputPulseSeriesMapName='OutFirstPulsifierOfflinePulses')  

#Note: Input/OutputPulseSeriesMapName are objects of type: I3RecoPulseSeriesMap, which lives in dataclasses 

#Check that the object that you put is indeed in the frame, 
#and calculate the total charge in the event
def CheckThings(frame):
    if not frame.Has('OutFirstPulsifierOfflinePulses'): # you can use the more python-style instead: if not 'OutFirstPulsifierOfflinePulses' in frame
        print "frame does not contain OutFirstPulsifierOfflinePulses, I skip this frame"
        return False #skip frame

    chargelist=[]# standard python list
    pulsesmap=frame.Get('OutFirstPulsifierOfflinePulses') # you can use the more python-style instead: frame['OutFirstPulsifierOfflinePulses']
    for pulses in pulsesmap.values():       # "pulses" is a list of Pulses that live in dataclasses, called: I3RecoPulseSeries
        chargelist+= [pulse.charge for pulse in pulses] # You store all the charge pulses of the event (frame) in a large python list
        
    if len(chargelist)!=0: 
        print 'The total number of pulses in this frame is {0} and the total charge is {1}'.format(len(chargelist),sum(chargelist))
    else: # just for fun:
        icetray.logging.log_warn('No pulses in OutFirstPulsifierOfflinePulses')

tray.AddModule(CheckThings,'CheckThings')

tray.Execute()


