from icecube import VHESelfVeto, DomTools, icetray
import math
from copy import copy

@icetray.traysegment
def add_hese_tag(tray, name,
                 pulses_split       = "InIcePulses",
                 If                 = lambda f: True ):
    '''
    apply the HESE selection and store HESE_passed as bool
    '''
    tray.AddModule('HomogenizedQTot', 'qtot_total', Pulses=pulses_split, Output="QTot")
    # run the veto modules
    tray.AddModule('I3LCPulseCleaning', 'cleaning',
                   OutputHLC='HLCPulses',
                   OutputSLC='',
                   Input=pulses_split)
    tray.AddModule('VHESelfVeto', 'selfveto',
                   Pulses='HLCPulses',
                   Geometry="I3Geometry")
    tray.AddModule('HomogenizedQTot', 'qtot_causal',
                   Pulses=pulses_split,
                   Output='CausalQTot',
                   VertexTime='VHESelfVetoVertexTime')

    # HESE-cuts combined into a single bool
    def HESE_passed(fr):
        qtot = fr["CausalQTot"].value
        fr.Put("Passed_HESE", icetray.I3Bool(False))
        if "VHESelfVeto" not in fr.keys():
            ## very weird notation in VHESelfVeto: If not 250PE found, it doesnt
            ## write the key --> because no Veto could be defined, it certainly doesnt pass.
            fr["Passed_HESE"].value = False
        elif qtot > 6000. and fr["VHESelfVeto"].value==False:
            fr["Passed_HESE"].value = True
        else:
            fr["Passed_HESE"].value = False
        return True

    tray.AddModule(HESE_passed, 'HESE_passed')

