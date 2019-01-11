/**
 * Copyright (C) 2009
 * The IceCube collaboration
 * ID: $Id: $
 *
 * @file I3HGLGPairSelector.cxx
 * @version $Rev: $
 * @date $Date: $
 * @author tilo
 */


#include <vemcal/I3HGLGPairSelector.h>
#include <dataclasses/physics/I3RecoPulse.h>
#include <dataclasses/I3MapOMKeyMask.h>


I3_MODULE(I3HGLGPairSelector);


I3HGLGPairSelector::I3HGLGPairSelector(const I3Context& context): I3ConditionalModule(context)
{
    inputName_ = "IceTopVEMPulses";
    AddParameter("InputPulses", "Name of input I3RecoPulseSeriesMap", inputName_);
    
    outputName_ = "IceTopHGLGData";
    AddParameter("OutputPulseMask", "Name of output I3RecoPulseSeriesMapMask which contains only HG-LG pairs.", outputName_);
}


I3HGLGPairSelector::~I3HGLGPairSelector()
{

}

 
void I3HGLGPairSelector::Configure()
{
    GetParameter("InputPulses", inputName_);
    GetParameter("OutputPulseMask", outputName_);
}


void I3HGLGPairSelector::DAQ(I3FramePtr frame)
{
    I3RecoPulseSeriesMapConstPtr input = frame->Get<I3RecoPulseSeriesMapConstPtr>(inputName_);
    if(!input)
    {
	PushFrame(frame);
	return;
    }
    
    // Create new output map to store the HG-LG pairs
    I3RecoPulseSeriesMapMaskPtr output(new I3RecoPulseSeriesMapMask(*frame, inputName_));
    output->SetNone();
    
    // Loop over input and search for HG-LG pairs
    for(I3RecoPulseSeriesMap::const_iterator iter1 = input->begin();
	iter1 != input->end(); ++iter1)
    {
	const OMKey& omKey1 = iter1->first;
	int om1 = omKey1.GetOM();
	if (om1 != 61 && om1 != 63) continue;

	int om2 = om1+1;
	OMKey omKey2(omKey1.GetString(), om2);
	
	// If partner DOM exists store both I3DOMLaunchSeries in output map
	I3RecoPulseSeriesMap::const_iterator iter2 = input->find(omKey2);
	if(iter2 != input->end())
	{
	    output->Set(omKey1, true);
	    output->Set(omKey2, true);
	}
    }
    
    // Write selected launches to frame
    frame->Put(outputName_, output);
    
    PushFrame(frame);
}
