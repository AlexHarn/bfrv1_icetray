from icecube import icetray

# split the sub event stream
def which_split(frame,split_name=None):
    if frame.Stop == icetray.I3Frame.Physics:
        if frame['I3EventHeader'].sub_event_stream == split_name:
#            print '------------InIce Split = ',frame['I3EventHeader'].sub_event_stream
            return True
        else:
            return False
    else:
        return False

## Here probably need to check with in_ice split    
def label(frame,year):
    if year == "2012":
	filter_name = 'CascadeFilter_12'
    elif year == "2011":
	filter_name = 'CascadeFilter_11'
    else:
	filter_name = 'CascadeFilter_13'

    if frame.Has('FilterMask'):
	
        if frame['FilterMask'][filter_name].condition_passed==1: # setting for 2015 data
	#if frame['FilterMask']['CascadeFilter_12'].condition_passed==1: # for test purpose
            frame['CscdL2'] = icetray.I3Bool(True)
        else:
            frame['CscdL2'] = icetray.I3Bool(False)
#            return True
    else:
        frame['CscdL2'] = icetray.I3Bool(False)


	
