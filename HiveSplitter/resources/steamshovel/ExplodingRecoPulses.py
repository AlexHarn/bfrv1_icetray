from icecube.shovelart import *
from icecube import dataclasses

class ExplodingRecoPulses( PyArtist ):
    """
    A steamshovel artist to plot exploding sphere
    """
    def __init__(self):
        PyArtist.__init__(self)
        self.defineSettings( { "trans":RangeSetting(0,255,255,50),
                               "PTR":RangeSetting(0,1000,1000,300),
                               "NTR":RangeSetting(0,1000,1000,200),
                               "Lightspeed": True} )
        #self.overlayScalingFactor(output) = 0.1

    requiredTypes = [ dataclasses.I3Geometry, dataclasses.I3RecoPulseSeriesMapMask ]

    def description(self):
        return "Exploding Pulses"

    def create( self, frame, output ):
        PTR = self.setting("PTR")
        NTR = self.setting("NTR")
        if self.setting("Lightspeed"):
          c=0.3
        else:
          c=0.3/1.3
        boundary = 300
        boundary_t = int(300/c)
        death_t = int(boundary_t+NTR)
      
        omgeo = frame[self.keys()[0]].omgeo
        dkey = self.keys()[1]
        domlaunches = frame[dkey].apply(frame)
        key_times = [ (omkey, int(launchlist[0].time) ) for omkey, launchlist in domlaunches ]
        for omkey, time in key_times:
            s = output.addSphere( 10., omgeo[omkey].position )
            s.setColor( PyQColor( 100, 100, 200, int(self.setting("trans"))) ) #TimeWindowColor( output, time, colormap )
            sizefunc = LinterpFunctionFloat(0)
            sizefunc.add( 0., time-1)
            sizefunc.add( 0.+PTR*c, time)
            sizefunc.add( boundary, time + boundary_t)
            sizefunc.add( boundary, time + death_t)
            sizefunc.add( 0., time + death_t+1)
            s.setSize( sizefunc ) #<-there LinterpFunctionFloat()
            s.setSelectionContent( omkey )

