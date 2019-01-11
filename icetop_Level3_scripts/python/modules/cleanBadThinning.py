## TODO at some point : all cleaners of launches or pulses should output Masks...

from icecube import icetray,dataio,dataclasses
import math
################################################################################################
# 0) BUG in Thinned MC : unsaturated tanks close to the core (within a station!!)              #
#                        --> NOT seen in non-thinned MC (overlap Ebin)                         #
#                        --> NOT seen in data                                                  #
# Showed up in log(Q) vs R(to true core) plot as a separate distribution :                     #
# lower half of crocodile mouth                                                                #
# ==> Based on that plot : E0 dependent cut which removes this unnatural mouth and 'Fixes' MC  #
# ==> SHOULD be fixed in dethinning for topsimulator though...                                 #
# https://docushare.icecube.wisc.edu/dsweb/Get/Document-61578/ThinBug.pdf                      #
# https://docushare.icecube.wisc.edu/dsweb/Get/Document-61617/Dethinning.pdf                   #
################################################################################################
class cleanBadThinning(icetray.I3ConditionalModule):
    def __init__(self, context):
        icetray.I3ConditionalModule.__init__(self, context)
        self.AddParameter('InputITpulseName','Which IceTop pulses to clean out',0)
        self.AddParameter('OutputPulseName','Name of the resulting thinFix cleaned IT pulses',0)
        self.AddParameter('ExcludedPulseName','Name of the resulting removed buggy IT pulses',0)
        self.AddOutBox("OutBox")
        
    def Configure(self):
        self.itpulses = self.GetParameter('InputITpulseName')
        self.outpulseName = self.GetParameter('OutputPulseName')
        self.exclpulseName = self.GetParameter('ExcludedPulseName')

    def Geometry(self, frame):
        if 'I3Geometry' in frame:
            geo = frame['I3Geometry']
            self.omg = geo.omgeo
        else:
            print 'No geometry found'
        self.PushFrame(frame,"OutBox")
    
    def Physics(self, frame):
        if self.itpulses in frame:
            itpulses = frame[self.itpulses]
            geo = frame["I3Geometry"]
            mcprim = frame["MCPrimary"]
            outputPulses = dataclasses.I3RecoPulseSeriesMapMask(frame, itpulses.source)
            outputPulses.set_none()
            excludedPulses = dataclasses.I3RecoPulseSeriesMapMask(frame, itpulses.source)
            excludedPulses.set_none()
            if itpulses.__class__ == dataclasses.I3RecoPulseSeriesMapMask:
                itpulses = itpulses.apply(frame)
            for om,pulses in itpulses:
                  if om in geo.omgeo:
                      ompos = self.omg[om]
                      dist = math.sqrt( (mcprim.pos.x - ompos.position.x)*(mcprim.pos.x - ompos.position.x)
                                        +(mcprim.pos.y - ompos.position.y)*(mcprim.pos.y - ompos.position.y))
                      for ind in range(0,len(pulses)):
                          pulse=pulses[ind]
                          exclude = False
                          if (dist < 200):
                              if math.log10(mcprim.energy) < 8.0 and math.log10(mcprim.energy) >= 7.0 :
                                  if(math.log10(pulse.charge) < (-3.35/95. * dist +3.35)):
                                      exclude = True
                                  else:
                                      exclude = False
                              elif math.log10(mcprim.energy) >= 8.0 and math.log10(mcprim.energy) < 8.5:
                                  if(math.log10(pulse.charge) < (-2.85/150. * dist +3.35)):
                                      exclude = True
                                  else:
                                      exclude = False
                              elif math.log10(mcprim.energy) >= 8.5 and math.log10(mcprim.energy) < 9.0:
                                  if(math.log10(pulse.charge) < (-2.4/185. * dist +3.5)):
                                      exclude = True
                              elif math.log10(mcprim.energy) >= 9.0:
                                  if(math.log10(pulse.charge) < (-1.7/200. * dist +3.55)):
                                      exclude = True
                              else:
                                  exclude = False
                          else:
                              exclude = False
                          ## put them in their maps
                          if exclude:
                              excludedPulses.set(om, ind, True)
                          else:
                              outputPulses.set(om, ind, True)
                  else:
                      print "OM not found in omgeo, wtf? : ", om
                        
            # outputting!
            frame[self.outpulseName] = outputPulses
            frame[self.exclpulseName] = excludedPulses
        else:
            print "Missing inputITpulses : ", self.itpulses
        self.PushFrame(frame,"OutBox")
