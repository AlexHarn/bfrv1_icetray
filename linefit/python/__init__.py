# -*- coding: utf-8 -*-
# 
# copyright  (C) 2010
# The Icecube Collaboration
# 
# $Id: __init__.py 78987 2011-08-16 14:10:46Z kislat $
# 
# @version $Revision: 78987 $
# @date $LastChangedDate: 2011-08-16 10:10:46 -0400 (Tue, 16 Aug 2011) $
# @author Jakob van Santen <vansanten@wisc.edu> $LastChangedBy: kislat $
# 

# pull in dependencies
import icecube.icetray
import icecube.recclasses
icecube.icetray.load("linefit", False)
try:
    import icecube.tableio
    import converters
except ImportError:
    pass

@icecube.icetray.traysegment
def simple(tray, name, inputResponse ="Pulses_cleaned", fitName = "linefit_improved", If = lambda f: True):
    """
    Computes a robust first-pass reconstruction on a data set.   
    
    :param inputResponse: Name of the data series to get pulses from.
    :param fitName: Name of the output reconstruction.
    """
    tray.AddModule("DelayCleaning", name+"_DelayCleaning", InputResponse =
    inputResponse, OutputResponse=name+"_Pulses_delay_cleaned", If=If)

    tray.AddModule("HuberFit", name+"_HuberFit", Name = name+"_HuberFit",
    InputRecoPulses = name+"_Pulses_delay_cleaned", If=If)

    tray.AddModule("Debiasing", name+"_Debiasing", OutputResponse =
    name+"_debiasedPulses", InputResponse = name+"_Pulses_delay_cleaned", Seed =
    name+"_HuberFit", If=If)

    tray.AddModule("I3LineFit",name+"_linefit_final", Name = fitName,
    InputRecoPulses = name+"_debiasedPulses", LeadingEdge= "ALL", AmpWeightPower=
    0.0, If=If)

    tray.AddModule("Delete", name+"_cleanup", keys =
        [name+"_debiasedPulses", name+"_HuberFit",
         name+"_linefit_final_rusage", name+"_Pulses_delay_cleaned" ], If=If )
