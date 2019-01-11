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

# a global variable:
OUPUTNAME='I3CoincifyOfflinePulses'

#Create an instance of I3Tray
tray= I3Tray.I3Tray()

# in dataio:
tray.AddModule("I3Reader", "reader", 
               Filename=options.INPUTFILE)

from icecube import DomTools
tray.AddModule('I3Coincify<I3RecoPulse>','I3Coincify',
               InputName='OfflinePulses',   
               OutputName=OUPUTNAME,
               CoincidenceWindow=800.*icetray.I3Units.ns)#Time window for local coincidence. 800 is the defalut value    

# Note: I3Coincify is a template C++ class, therefore, you can run it like: I3Coincify<I3RecoPulse>, I3Coincify<I3RecoHit>, 
# I3Coincify<I3MCHit> or I3Coincify<I3DOMLaunch>

#Check that the object that you put is indeed in the frame, 
#and calculate the total charge in the event
def CheckThings(frame):
    if not frame.Has(OUPUTNAME): # you can use the more python-style instead: if not OUPUTNAME in frame
        print "frame does not contain {0}, I skip this frame".format(OUPUTNAME)
        return False #skip frame

    chargelist=[]# standard python list
    pulsesmap=frame.Get(OUPUTNAME) # you can use the more python-style instead: frame[OUPUTNAME]
    for pulses in pulsesmap.values():       # "pulses" is a list of Pulses that live in dataclasses, called: I3RecoPulseSeries
        chargelist+= [pulse.charge for pulse in pulses] # You store all the charge pulses of the event (frame) in a large python list
        
    if len(chargelist)!=0: 
        print 'The total number of pulses in this frame is {0} and the total charge is {1}'.format(len(chargelist),sum(chargelist))
    else: # just for fun:
        icetray.logging.log_warn('No pulses in %s' % OUPUTNAME)

tray.AddModule(CheckThings,'CheckThings')

tray.Execute()


