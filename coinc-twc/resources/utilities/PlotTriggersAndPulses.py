# This script shows what coinc-twc does. Coinc twc select the inice pulses around the correct inice trigger. This trigger is selected with according to the timing difference with the IceTop trigger.
# It produces two plots (in one canvas). One with all the pulses and all triggers, the other one with all triggers, the IceTop pulses and selected inice pulses.
# It needs both IceTop L2 and L3 files, since I3TriggerHierarchy can only be found in the L2 files, while the selected pulses are in L3. 
# You need to give an event and run number, and the path to the L2 and L3 files.
# An example event is filled out below, which has one muon bundle and two other inice triggers.

from icecube import icetray, dataclasses, dataio

runnr=118267
eventnr=16624632
infilename_L2='/data/exp/IceCube/2011/filtered/level2/0601/Level2_IC86.2011_data_Run00118267_Part00000030_IT.i3.bz2'
infilename_L3='/data/ana/CosmicRay/IceTop_level3/IceTop_InIce/IC86.2011/06_01/Level3_Run118267_Part3.i3.gz' # non-existent at this moment of commit, but should soon be there.

i3file_L2=dataio.I3File(infilename_L2)
i3file_L3=dataio.I3File(infilename_L3)

# Find the event in L2 to get the trigger Hierarchy
foundInL2=False
while i3file_L2.more():
    qfr_L2=i3file_L2.pop_daq()
    if qfr_L2['I3EventHeader'].run_id==runnr and qfr_L2['I3EventHeader'].event_id==eventnr:
        foundInL2=True
        break

if not foundInL2:
    print "Event not found in L2. Exit"
    exit(0)

# Get the trigger hierarchy
th=qfr_L2['I3TriggerHierarchy']

# Find the it trigger(s). 
nrITTriggers=0
for i, j in th.iteritems():
    if j.key.source==dataclasses.ICE_TOP and j.key.config_id==102:
        print j
        ittrigger=j
        nrITTriggers+=1

if nrITTriggers>1:
    print 'Multiple IT triggers. This script is not advanced enough'
    exit(0)

# Find the inice SMT triggers: 1006 is SMT8, 1010 SMT3
iitriggers=[]
for i, j in th.iteritems():
    if j.key.source==dataclasses.IN_ICE and j.key.type==dataclasses.SIMPLE_MULTIPLICITY:
        iitriggers.append(j)

# Find the event in L3 to get all the pulses
foundInL3=False
while i3file_L3.more():
    qfr_L3=i3file_L3.pop_daq()
    if qfr_L3['I3EventHeader'].run_id==runnr and qfr_L3['I3EventHeader'].event_id==eventnr:
        foundInL3=True
        break

if not foundInL3:
    print "Event not found in L3. Exit"
    exit(0)

pfr_L3=i3file_L3.pop_physics()  ## Only ice_top stream p frames in L3! 

# Get the it pulse times:
itpulses=qfr_L3['OfflineIceTopHLCTankPulses'] 
itpulsetimes=[]
for pulseArr in itpulses.values():
    for pulse in pulseArr:
        itpulsetimes.append(pulse.time)

# Get the inice pulse times
iipulses=qfr_L3['InIcePulses']
iipulsetimes=[]
for pulseArr in iipulses.values():
    for pulse in pulseArr:
        iipulsetimes.append(pulse.time)

# Get the coinc inice pulse times
coincpulses=pfr_L3['CoincPulses'].apply(pfr_L3)
coincpulsetimes=[]
for pulseArr in coincpulses.values():
    for pulse in pulseArr:
        coincpulsetimes.append(pulse.time)

# Take all pulse times to make the plot
allpulsetimes=itpulsetimes+iipulsetimes

# Plotting part
from ROOT import TCanvas, TH1F, gStyle, TLegend
import numpy as np

hist_itpulses=TH1F("itpulses","itpulses",100,np.amin(allpulsetimes)-10,np.amax(allpulsetimes)+10)
hist_iipulses=TH1F("iipulses","iipulses",100,np.amin(allpulsetimes)-10,np.amax(allpulsetimes)+10)
hist_coincpulses=TH1F("coincpulses","coincpulses",100,np.amin(allpulsetimes)-10,np.amax(allpulsetimes)+10)

for itpulsetime in itpulsetimes:
    hist_itpulses.Fill(itpulsetime)

for iipulsetime in iipulsetimes:
    hist_iipulses.Fill(iipulsetime)

for coincpulsetime in coincpulsetimes:
    hist_coincpulses.Fill(coincpulsetime)

hist_ittrigger=TH1F("ittrigger","ittrigger",500,np.amin(allpulsetimes)-10,np.amax(allpulsetimes)+10)
hist_ittrigger.Fill(ittrigger.time,np.amax([hist_itpulses.GetMaximum(),hist_iipulses.GetMaximum()])+10)

hist_iitrigger=TH1F("iitrigger","iitrigger",500,np.amin(allpulsetimes)-10,np.amax(allpulsetimes)+10)
for iitrigger in iitriggers:
    hist_iitrigger.Fill(iitrigger.time,np.amax([hist_itpulses.GetMaximum(),hist_iipulses.GetMaximum()])+10)

hist_ittrigger.SetLineColor(2)
hist_iitrigger.SetLineColor(7)
hist_iipulses.SetLineColor(3)
hist_itpulses.SetLineColor(4)
hist_coincpulses.SetLineColor(3)

hist_ittrigger.SetLineWidth(2)
hist_iitrigger.SetLineWidth(2)
hist_iipulses.SetLineWidth(2)
hist_itpulses.SetLineWidth(2)
hist_coincpulses.SetLineWidth(2)

canv=TCanvas("canv","canv",1200,600)
canv.Divide(2)
canv.cd(1)
gStyle.SetOptStat(0)
hist_ittrigger.Draw("")
hist_iitrigger.Draw("SAME")
hist_iipulses.Draw("SAME")
hist_itpulses.Draw("SAME")

hist_ittrigger.SetTitle("Triggers, IceTop and uncleaned InIce pulses")
hist_ittrigger.GetYaxis().SetTitle("For pulses: number of pulses")
hist_ittrigger.GetXaxis().SetTitle("time [ns]")

hist_ittrigger.GetYaxis().SetRangeUser(0,np.amax([hist_itpulses.GetMaximum(),hist_iipulses.GetMaximum()])+10)
leg=TLegend(0.7,0.7,0.89,0.89)
leg.AddEntry(hist_ittrigger,"IT SMT", "l")
leg.AddEntry(hist_iitrigger,"II SMT", "l")
leg.AddEntry(hist_itpulses,"IT pulses", "l")
leg.AddEntry(hist_iipulses,"II pulses", "l")
leg.SetFillColor(10)
leg.SetBorderSize(0)
leg.Draw("SAME")

canv.cd(2)
gStyle.SetOptStat(0)
hist_iitrigger.Draw("")
hist_ittrigger.Draw("SAME")
hist_itpulses.Draw("SAME")
hist_coincpulses.Draw("SAME")

hist_iitrigger.SetTitle("Triggers, IceTop and selected InIce pulses")
hist_iitrigger.GetYaxis().SetTitle("For pulses: number of pulses")
hist_iitrigger.GetXaxis().SetTitle("time [ns]")
hist_iitrigger.GetYaxis().SetRangeUser(0,np.amax([hist_itpulses.GetMaximum(),hist_iipulses.GetMaximum()])+10)

leg2=TLegend(0.7,0.7,0.89,0.89)
leg2.AddEntry(hist_ittrigger,"IT SMT", "l")
leg2.AddEntry(hist_iitrigger,"II SMT", "l")
leg2.AddEntry(hist_itpulses,"IT pulses", "l")
leg2.AddEntry(hist_coincpulses,"II pulses", "l")
leg2.SetFillColor(10)
leg2.SetBorderSize(0)
leg2.Draw("SAME")

canv.Print("Run"+str(runnr)+"_Event"+str(eventnr)+"_triggersAndPulses.png")
