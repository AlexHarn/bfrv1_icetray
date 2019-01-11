/**
 * Copyright (C) 2009
 * The IceCube collaboration
 * ID: $Id: $
 *
 * @file I3HGLGPairSelector.h
 * @version $Rev: $
 * @date $Date: $
 * @author tilo
 */


#ifndef _VEMCAL_I3HGLGPAIRSELECTOR_H_
#define _VEMCAL_I3HGLGPAIRSELECTOR_H_


#include <icetray/I3ConditionalModule.h>
#include <string>


class I3HGLGPairSelector : public I3ConditionalModule 
{
public:
    I3HGLGPairSelector(const I3Context& context);
    
    ~I3HGLGPairSelector();
    
    /// Re-implementation of Configure method
    void Configure();
    
    /// Re-implementation of DAQ method
    void DAQ(I3FramePtr frame);
    
    
private:
    
    // Module parameters
    std::string inputName_;
    std::string outputName_;
    
    SET_LOGGER("I3HGLGPairSelector");
};

#endif
