##!/usr/bin/env python
#
#######################################
##              IC86 2011
## Cascade Filter - level3 processing
## Mariola Lesiak-Bzdak & Chang Hyon Ha
## Nov 2012
#######################################


from I3Tray import *
import sys, os, glob

#
from icecube.level3_filter_cascade.level3_TopoSplitter import TopologicalCounter
from icecube.level3_filter_cascade.level3_RunVeto import runVeto_Singles, runVeto_Coinc
from icecube.level3_filter_cascade.level3_MultiCalculator import multiCalculator
from icecube.level3_filter_cascade.level3_rateCalc import RateTracker
from icecube.level3_filter_cascade.level3_Globals import which_split, label
from icecube.level3_filter_cascade.level3_HighLevelFits import HighLevelFits # exclude monopod
from icecube.level3_filter_cascade.level3_Cuts import tagBranches, cutBranches
from icecube.level3_filter_cascade.level3_Recos import CascadeLlhVertexFit, TimeSplitFits, CoreRemovalFits
from icecube.level3_filter_cascade.level3_CalibrationExtra import preparePulses
from icecube.level3_filter_cascade.L3_monopod import L3_Monopod


def maskify(frame):
    if frame.Has('SplitInIcePulses'): 
        frame['OfflinePulses']=frame['SplitInIcePulses']  # In IC86-2013 'SplitInIcePulses' is used as 'OfflinePulses' in IC86-2011
	frame['OfflinePulsesTimeRange']=frame['SplitInIcePulsesTimeRange']
    else:
	return True
    if frame.Has('SRTInIcePulses'):
        frame['SRTOfflinePulses']=frame['SRTInIcePulses']
    else:
	return True
    return True



@icetray.traysegment
def CascadeL3(tray, name, gcdfile, infiles, output_i3, AmplitudeTable, TimingTable, MCbool, MCtype,Year):
   
   
    files = [gcdfile]

    if isinstance(infiles,str):
        files.append(infiles)
    else:
        for f in infiles:files.append(f)
    
    tray.AddModule("I3Reader", FilenameList=files)

    if not Year == "2011":
	split_name = 'InIceSplit'
	tray.AddModule(maskify,'maskify')

    else:
	split_name = 'in_ice'

    tray.AddModule(label,'label_CascadeL2Stream',year=Year)
        
    # selects cascade L2 stream w/o removing IceTop p-frames.
    tray.AddModule(lambda frame: frame['CscdL2'].value and which_split(frame, split_name),'SelecCscdL2')

    # count multiplicity of the events
    tray.AddSegment(TopologicalCounter,'CscdL2_Topo_HLC', 
                    pulses='OfflinePulsesHLC',
                    InIceCscd = lambda frame: which_split(frame, split_name) and frame['CscdL2'].value
                    )

    # run I3Veto on single events
    tray.AddSegment(runVeto_Singles,
                    '_SRTOfflinePulses',
                    pulses='SRTOfflinePulses',
                    If= lambda frame: which_split(frame, split_name) and frame['CscdL2'].value and frame['CscdL2_Topo_HLCSplitCount'].value==1
                    )
    # run I3Veto on coincident events
    tray.AddSegment(runVeto_Coinc,
                    'CscdL2_Topo_HLC',
                    pulses='CscdL2_Topo_HLC',
                    If = lambda frame: which_split(frame, split_name) and frame['CscdL2'].value and frame['CscdL2_Topo_HLCSplitCount'].value==2
       )
    
    # tagging events 
    tray.AddModule(tagBranches,'Branchtagging', InIceCscd= lambda frame: which_split(frame, split_name) and frame['CscdL2'].value)

    # calculating NCh, NStrings for singles branches but run all events (no harm done)
    tray.AddSegment(multiCalculator,'multiHLC',
                    pulses="OfflinePulsesHLC",
                    InIceCscd = lambda frame: which_split(frame, split_name) and frame['CscdL2'].value,
                    )

    # general cascade llh w/o DC for singles branches but run all events (no harm done)
    tray.AddSegment(CascadeLlhVertexFit, 'CascadeLlhVertexFit_IC',
                    Pulses = 'OfflinePulsesHLC_noDC',
                    If = lambda frame: which_split(frame, split_name) and frame['CscdL2'].value,
                    )
    # calculating NCh, NStrings for singles branches but run all events (no harm done)
    tray.AddSegment(multiCalculator,'multi',
                    pulses="OfflinePulses",
                    InIceCscd = lambda frame: which_split(frame, split_name) and frame['CscdL2'].value,
                    )

    # calculating NCh, NStrings QTot etc for doubles branch but only doubles
    tray.AddSegment(multiCalculator,'multiHLC_TS0',
                    pulses="CscdL2_Topo_HLC0",
                    InIceCscd = lambda frame: which_split(frame, split_name) and frame['CscdL2'].value and frame['CscdL2_Topo_HLCSplitCount'].value==2,
                    )
    tray.AddSegment(multiCalculator,'multiHLC_TS1',
                    pulses="CscdL2_Topo_HLC1",
                    InIceCscd = lambda frame: which_split(frame, split_name) and frame['CscdL2'].value and frame['CscdL2_Topo_HLCSplitCount'].value==2,
                    )

    # calculate two CscdLlhVertexFits w/o DC for doubles contained branch
    tray.AddSegment(CascadeLlhVertexFit, 'CascadeLlhVertexFit_IC_Coincidence0',
                    Pulses = 'CscdL2_Topo_HLC0_noDC',
                    If = lambda frame: which_split(frame, split_name) and frame['CscdL2'].value and frame['CscdL2_Topo_HLCSplitCount'].value==2,
                    )
    tray.AddSegment(CascadeLlhVertexFit, 'CascadeLlhVertexFit_IC_Coincidence1',
                    Pulses = 'CscdL2_Topo_HLC1_noDC',
                    If = lambda frame: which_split(frame, split_name) and frame['CscdL2'].value and frame['CscdL2_Topo_HLCSplitCount'].value==2,
                    )

    # L3 cut happens here
    tray.AddModule(cutBranches,'Level3_Cut',year=Year)

    tray.AddModule('I3OrphanQDropper', 'drop_unwanted_q')

    # multi calc for all L3 events
    tray.AddSegment(multiCalculator,'multiCalc_AllL3',
                    pulses='SRTOfflinePulses',
                    InIceCscd = lambda frame: which_split(frame, split_name) and frame['CscdL2'].value,
                    )

    ## prepare pulses for monopod and crdo w/o satur. DOMs
    if MCbool :
        print "This is Monte Carlo"
        tray.AddSegment(preparePulses, 'newPulses', Simulation=True,InIceCscd = lambda frame: which_split(frame, split_name) and frame['CscdL2'].value)
    else:
        print "This is real data"
        tray.AddSegment(preparePulses, 'newPulses',InIceCscd = lambda frame: which_split(frame, split_name) and frame['CscdL2'].value)

    # fitting SPE32/CscdLLH/Bayesian32/Monopod8 for all
    tray.AddSegment(HighLevelFits, 'CscdL3_HighLevelFits_IC',
                    Pulses='SRTOfflinePulses',
                    InIceCscd = lambda frame: which_split(frame, split_name) and frame['CscdL2'].value,
                    )

    tray.AddSegment(L3_Monopod, 'monopod',
                    Pulses='OfflinePulses',
		    year=Year,
		    AmplitudeTable = AmplitudeTable,
	            TimingTable = TimingTable
                    )
    # Agreed to exclude the fit at L3
    #    tray.AddSegment(MonopodReco, 'monopod_l3',
    #                    Pulses='SRTOfflinePulses',
    #                    )

    tray.AddSegment(TimeSplitFits, 'TimeSplit', Pulses="OfflinePulses")
    tray.AddSegment(CoreRemovalFits, 'CoreRemoval', Pulses="OfflinePulses",Vertex="L3_MonopodFit4_AmptFit")

    tray.AddModule('Delete', 'DeleteNotNeeded', Keys=[ 'CalibratedWaveforms'])

    # No root files for L3a
    #    from icecube.rootwriter import I3ROOTWriter
    #    tray.AddSegment(I3ROOTWriter, 'scribe',
    #                    Output=rootFile,
    ##                    Keys=['CscdL3_Monopod_SpiceMie_LikelihoodMonopod-SpiceMie', 'CscdL3_Monopod_SpiceMie_CscdL3_Monopod_SpiceMie_likelihood','CscdL3_Monopod_SpiceMieFitParams'], #exclude monopod
    #                    bookeverything = True,
    #                    SubEventStreams=[split_name,'nullsplit', 'ice_top'],
    #                    )

    tray.AddModule("I3Writer", "EventWriter",
                   FileName=output_i3,
                   Streams=[icetray.I3Frame.TrayInfo,
                            icetray.I3Frame.DAQ,
                            icetray.I3Frame.Physics],
                   DropOrphanStreams=[icetray.I3Frame.DAQ],
                   )

