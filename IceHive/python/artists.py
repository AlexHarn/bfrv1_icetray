from icecube.shovelart import *
from icecube import dataclasses
from icecube.dataclasses import I3Constants

class ExplodingSpheres( PyArtist ):
    """
    A steamshovel artist to plot exploding spheres
    """
    def __init__(self):
        PyArtist.__init__(self)
        self.defineSettings( { "Transparancy":RangeSetting(0,255,255,50), #set a transperency value
                               "Boundary": RangeSetting(0,500, 500, 300), #the maximum boundary
                               "PosTimeRes":RangeSetting(0,1000,1000,300), #the negative Time Residual
                               "NegTimeRes":RangeSetting(0,1000,1000,200), #the positive Time Residual
                               "C/C_n": True, #use expansion with vacuum or medium specific lightspeed 
                               } )
    #numRequiredKeys = 1
    #def isValidKey( self, frame, key_idx, key ):
        #'''Accept any of the simple I3 data types with a .value property'''
        #try:
           #fobj = frame[key]
        #except (TypeError, RuntimeError):
           #return False;

        #if( isinstance( fobj,
            #( dataclasses.I3RecoPulseSeriesMapMask, dataclasses.I3RecoPulseSeriesMap,
              ##dataclasses.I3MCHit, dataclasses.I3MCPulse 
              #) )):
            #return True;

    requiredTypes = [ dataclasses.I3Geometry, dataclasses.I3RecoPulseSeriesMap ]

    def description(self):
        return "Exploding Pulses"

    def create( self, frame, output ):
        transperency = int(self.setting("Transparency"))
        boundary = float(self.setting("Boundary"))
        PTR = self.setting("PosTimeRes")
        NTR = self.setting("NegTimeRes")
        if self.setting("C/C_n"): #fix the expansion speed
          exp_speed=I3Constants.c
        else:
          exp_speed=I3Constants.c/I3Constants.n_ice_group_speed
        boundary_t = int(boundary/exp_speed) #time, when boundary is reached
        death_t = int(boundary_t+NTR) #time, when spheres have to die
      
        omgeo = frame["I3Geometry"].omgeo #frame[self.keys()[0]].omgeo
        dkey = self.keys()[1]
        domhits = dataclasses.I3RecoPulseSeriesMap.from_frame(frame, dkey)
        key_times = [ (omkey, int(launchlist[0].time) ) for omkey, launchlist in domhits ]
        for omkey, time in key_times:
            s = output.addSphere( 10., omgeo[omkey].position )
            s.setColor( PyQColor( 100, 100, 200, int(transperency)) ) #TimeWindowColor( output, time, colormap )
            sizefunc = LinterpFunctionFloat(0)
            sizefunc.add( 0., time-1)
            sizefunc.add( 0.+PTR*exp_speed, time)
            sizefunc.add( boundary, time + boundary_t)
            sizefunc.add( boundary, time + death_t)
            sizefunc.add( 0., time + death_t+1)
            s.setSize( sizefunc ) #<-there LinterpFunctionFloat()
            s.setSelectionContent( omkey )

