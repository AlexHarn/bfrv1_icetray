############################
# parser Options
############################
import optparse

parser= optparse.OptionParser()
parser.add_option('--gcdfile',type=str,dest='GCDFILE',
                  help='Input GCD file. Default file is given if you are in madison cluster',
                  default='/data/exp/IceCube/2011/filtered/level2/0731/Level2_IC86.2011_data_Run00118514_0731_GCD.i3.gz')

parser.add_option('--inputfile',type=str,dest='INPUTFILE',
                  help='Input i3 file. Default file is given if you are in madison cluster',
                  default='/data/exp/IceCube/2011/filtered/level2/0731/Level2_IC86.2011_data_Run00118514_Part00000085.i3.bz2')

options =parser.parse_args()[0]

##############################
# IceTray
#############################
import I3Tray
from icecube import dataio, dataclasses, icetray

# a global variable:
OUPUTNAME='OMSelectionOfflinePulses'

#Create an instance of I3Tray
tray= I3Tray.I3Tray()

# in dataio:
tray.AddModule("I3Reader", "reader", 
               FilenameList=[options.GCDFILE,options.INPUTFILE])

from icecube import DomTools
tray.AddModule('I3OMSelection<I3RecoPulseSeries>','I3OMSelection',
               InputResponse='OfflinePulses',
               OutputResponse=OUPUTNAME,
               OutputOMSelection='BadOMSelection',# This is the default name
               OmittedKeys=[icetray.OMKey(2,56,0), icetray.OMKey(3,43,0)],#any keys, just an example!
               #OmittedStrings= This is a list of Strings to omit. If you want to give it, the module do need the Geometry! 
                                     #(anyway, this is given due to this example takes the GCD file as parser)
               )

# Note: I3OMSelection is a template C++ class, therefore, you can run it like: I3OMSelection<SomeClass>, 
# this is configurated for: I3OMSelection<I3RecoPulseSeries> and I3OMSelection<I3DOMLaunchSeries>

#Check that the object that you put is indeed in the frame, 
#and calculate the COG (Center of Gravity) in Z. Well, this should be called Center of Charge
def CheckThings(frame):
    if not frame.Has(OUPUTNAME): # you can use the more python-style instead: if not OUPUTNAME in frame
        print "frame does not contain {0}, I skip this frame".format(OUPUTNAME)
        return False #skip frame

    pulsesmap=frame.Get(OUPUTNAME) # you can use the more python-style instead: frame[OUPUTNAME]

    geometry= frame.Get('I3Geometry')
    cog=0.
    charge=0.0
    for om, pulses in pulsesmap.items():
        if not geometry.omgeo.has_key(om) or len(pulses)==0: continue
        omgeo= geometry.omgeo.get(om)
        om_pos= omgeo.position
        z=om_pos.z
        for pulse in pulses:
            cog+=pulse.charge*z
            charge+=pulse.charge
            
    if charge==0.: 
        icetray.logging.log_warn('No Charge in this Frame')
    else:        
        cog=cog/charge
        print 'The COG-Z is: {0}'.format(cog)

tray.AddModule(CheckThings,'CheckThings')

tray.Execute()


