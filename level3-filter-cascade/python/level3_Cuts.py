from icecube import icetray, dataclasses
from I3Tray import load
import math
from math import cos, log10

def checkCont(frame, pulses):
    if frame[pulses].depthFirstHit <  430. and frame[pulses].depthFirstHit > -430. and frame[pulses].earliestLayer <=4:
        return True
    else:
        return False

def tagBranches(frame, InIceCscd = lambda frame: True):
    """
    Tag events according to number of TopoSplits and Veto module output
    """
    pulses_Veto = 'Veto_SRTOfflinePulses'
    pulses_TopoSplitted = 'Veto_CscdL2_Topo_HLC'
    numberOfTopoSplits = 'CscdL2_Topo_HLCSplitCount'

    ContTag = 'CscdL3_Cont_Tag'
    cont_tag = -1
    if frame.Has(pulses_Veto) and frame[numberOfTopoSplits].value==1:  
        if checkCont(frame,pulses_Veto):
            cont_tag = 1
        else:
            cont_tag = 0
    elif frame.Has('Veto_CscdL2_Topo_HLC0') and frame.Has('Veto_CscdL2_Topo_HLC1') and frame[numberOfTopoSplits].value==2:
        if checkCont(frame,"%s%d" % (pulses_TopoSplitted,0)) or checkCont(frame,"%s%d" % (pulses_TopoSplitted,1)):
            cont_tag = 2
    else:
        cont_tag = -1
    frame[ContTag] = dataclasses.I3Double(cont_tag)
        
def cutBranches(frame,year):
    from icecube import fill_ratio, cscd_llh
    
    contTag    = frame['CscdL3_Cont_Tag'].value
    if not year == "2011":
        fill_ratio = frame['CascadeFillRatio_L2'].fill_ratio_from_mean
    else:
        fill_ratio = frame['CascadeFillRatio'].fill_ratio_from_mean
    
    L3_Branch0 = False
    L3_Branch1 = False
    L3_Branch2 = False
    L3_Branch3 = False

    if contTag == 1 :
        nstr       = frame['NString_OfflinePulsesHLC_noDC'].value
        rloglCscd  = frame['CascadeLlhVertexFit_ICParams'].ReducedLlh
        if (fill_ratio>0.6) and (nstr >= 3) and (rloglCscd<9.0)  and (rloglCscd>0.0) :
            L3_Branch1 = True
        else:
            L3_Branch1 = False

    elif contTag == 0 :
        nch = frame['NCh_OfflinePulses'].value

        if fill_ratio>0.6 and nch>=120 :
            L3_Branch0 = True
        else:
            L3_Branch0 = False

    elif contTag == 2 :
        rlogL_0   = frame['CascadeLlhVertexFit_IC_Coincidence0Params'].ReducedLlh
        rlogL_1   = frame['CascadeLlhVertexFit_IC_Coincidence1Params'].ReducedLlh
        if ( (rlogL_0>0.0 and rlogL_0<8.5) or (rlogL_1>0.0 and rlogL_1<8.5) ) :
            L3_Branch2 = True
        else :
            L3_Branch2 = False
          
    else :
        L3_Branch3 = False

    # The actual cut is here
    if L3_Branch0 or L3_Branch1 or L3_Branch2 or L3_Branch3 :
        frame['CscdL3'] = icetray.I3Bool(True)
        return True
    else :
        frame['CscdL3'] = icetray.I3Bool(False)
        return False
        #return True

