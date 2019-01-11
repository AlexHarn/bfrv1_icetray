from icecube.icetray import load, traysegment
from I3Tray import *
from icecube import CascadeVariables

load("CascadeVariables")

@traysegment
def runVeto_Singles(tray, suffix, pulses, If = lambda frame: True):
    tray.AddModule("I3VetoModule", "Veto_Common"+suffix,
                   HitmapName=pulses,
                   OutputName="Veto"+suffix,
                   DetectorGeometry=86,
                   useAMANDA=False,
                   FullOutput=True,
                   If = If,
                   )

@traysegment
def runVeto_Coinc(tray, suffix, pulses, If = lambda frame: True):
    for i in xrange(2):
        tray.AddModule("I3VetoModule", "Veto_Coinc%d" % i,
                       HitmapName='CscdL2_Topo_HLC%d' % i,
                       OutputName='Veto_%s%d' % (suffix,i),
                       DetectorGeometry=86,
                       useAMANDA=False,
                       FullOutput=True,
                       If = If,
                       )
