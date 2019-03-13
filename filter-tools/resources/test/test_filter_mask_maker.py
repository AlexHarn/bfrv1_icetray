import numpy as np
from icecube import icetray,dataclasses,filter_tools
from I3Tray import I3Tray

object_name='FMask'
Nframes=1000
filters = [('FSS'    , 0),
           ('Muon'   , 1),
           ('IceTop' , 10),
           ('MinBias', 100),
           ]
dtype = [ (n,bool) for n,_ in filters]
condition_passed = np.zeros(Nframes,dtype=dtype)
prescale_passed = np.zeros(Nframes,dtype=dtype)
np.random.seed(0)

def generate_filters(frame):
    for filt,_ in filters:
        frame[filt]=icetray.I3Bool(np.random.randint(2))

N=0
def save_filter(frame):
    global N
    for name,result in frame[object_name].items():
        condition_passed[name][N]=result.condition_passed
        prescale_passed[name][N]=result.prescale_passed
    N+=1
        
tray=I3Tray()
tray.AddModule("I3InfiniteSource",Stream=icetray.I3Frame.Physics)
tray.Add(generate_filters)
tray.AddModule("I3FilterMaskMaker",
               OutputMaskName = object_name,
               FilterConfigs = dataclasses.I3MapStringInt(filters))
tray.Add(save_filter)
tray.Execute(Nframes)

for name,prescale in filters:

    cond_passed = condition_passed[name]
    pre_passed  = prescale_passed[name][cond_passed.astype(bool)]
    pre_notp    = prescale_passed[name][~(cond_passed.astype(bool))]

    cond_passed_frac = sum(cond_passed)/len(cond_passed)
    cond_sigma = (0.5-cond_passed_frac)*np.sqrt(len(cond_passed))

    if prescale:               
        prescale_passed_frac = len(pre_passed)/sum(pre_passed)
        prescale_sigma = (prescale-prescale_passed_frac)/np.sqrt(len(pre_passed))
        if prescale==1:
            assert prescale_passed_frac==1.0
    else:
        prescale_sigma=0
        prescale_passed_frac=0
        
    assert abs(cond_sigma)<2
    assert all(pre_notp==0)
    assert abs(prescale_sigma)<2
    
