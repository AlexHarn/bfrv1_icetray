#include <icetray/I3Bool.h>
#include <icetray/I3ConditionalModule.h>
#include <dataclasses/physics/I3FilterResult.h>
#include <phys-services/I3GSLRandomService.h>

/**
 * @brief  A module to create a FilterResultMap (aka FilterMask) based on 
 * I3Bools in the frame with configurable prescales.
 *
 * Generate a FilterMask (stored at OutputMask parameter name)
 * based on an input list of Filter names (expected as I3Bools in frame)
 * and prescales (as input FilterConfigs list, expected as a I3MapStringInt):
 *
 * @code{.py}
 * filter_pairs = I3MapStringInt([('MuonFilter_11',1),
 *                                ('CascadeFilter_11',1),
 *                                ....
 *                                ]
 * @endcode
 * 
 * Can a use provided random service, or will generate one on it's own.
*/
class FilterMaskMaker : public I3ConditionalModule
{
  std::string maskname_;
  I3MapStringInt filtconfigs_;
  I3RandomServicePtr randserv_;
public:
  FilterMaskMaker(const I3Context& context);
  void Configure();
  void Physics(I3FramePtr frame);
};

using namespace std;

FilterMaskMaker::FilterMaskMaker(const I3Context& context):
  I3ConditionalModule(context)
{
  AddParameter("OutputMaskName", "Filter mask output name", "FilterMask");
  AddParameter("FilterConfigs", "I3MapStringInt of filter names and prescales", filtconfigs_);
  AddParameter("RandomService", "RandomService to use with prescale calculation", randserv_);
}

void FilterMaskMaker::Configure()
  {
    GetParameter("OutputMaskName",maskname_);
    GetParameter("FilterConfigs",filtconfigs_);
    if (filtconfigs_.size()==0){
      log_fatal("No filter configurations provided");
    }
    GetParameter("RandomService", randserv_);
    if (!randserv_){
      log_info("No random service given, taking a basic one internally");
      randserv_ = I3RandomServicePtr(new I3GSLRandomService(13337));
    }
  }
  
void FilterMaskMaker::Physics(I3FramePtr frame)
{
  string  filter_name;
  uint64_t filter_prescale;
  I3BoolConstPtr filt_decision;
  
  // A place to store ouputs
  I3FilterResultMapPtr mymask(new I3FilterResultMap);   
  
  // Loop over all keys in FilterConfigs, look for bools, apply prescales and add entry to FilterMask    
  for (auto filt: filtconfigs_) {      
    tie(filter_name,filter_prescale)=filt;
    I3FilterResult myfiltobj;
    filt_decision  = frame->Get<I3BoolConstPtr>(filter_name);
    if (filt_decision){
      //assign the bool value to condition_passed AND prescale_passed for the filt obj
      if (filt_decision->value){         
        myfiltobj.conditionPassed = true;
        if (filter_prescale == 0){ //Filter off by definition
          myfiltobj.prescalePassed = false;
        } else{
          if (filter_prescale == 1){//Filter on, no prescale
            myfiltobj.prescalePassed = true;
          }else{
            if (randserv_->Uniform(1.0)*filter_prescale < 1.0){
              myfiltobj.prescalePassed = true;
          } else{
            myfiltobj.prescalePassed = false;
          }
        }
      }
    }else{
      myfiltobj.conditionPassed = false;
      myfiltobj.prescalePassed = false;
    }                
  }else{
    // filter bool not in frame, didn't pass or run:
    myfiltobj.conditionPassed = false;
    myfiltobj.prescalePassed = false;
  }
  //store the filt obj in the mask
  (*mymask)[filter_name] = myfiltobj;
}
// Wedge your new filter mask into the frame
frame->Put(maskname_,mymask);
//Don't forget to push frames!
PushFrame(frame);
}

I3_MODULE(FilterMaskMaker);
