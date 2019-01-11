#include <I3Test.h>
#include "icetray/I3TrayHeaders.h"
#include "icetray/I3Tray.h"
#include "dataclasses/I3Time.h"
#include <icetray/I3Units.h>
#include <icetray/I3Frame.h>
#include <icetray/I3ConditionalModule.h>
#include <phys-services/I3Splitter.h>


class DSTTestGenerator: public I3ConditionalModule, public I3Splitter
{
  private:
          double dtheta0_;
          double dphi0_;
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif
          bool headerWritten_;
          int coordDigits_;
          uint64_t currentTime_;
          uint64_t timesum_;
          uint64_t starttime_;
#ifdef __clang__
#pragma clang diagnostic pop
#endif
          std::string i3reco_1_;
          std::string i3reco_2_;
          std::string i3reco_2params_;
          std::string inicerawdata_name_;
          std::string icetoprawdata_name_;
          std::string icrecoseries_name_;
          std::string trigger_name_;

          double centerX_;
          double centerY_;
          double centerZ_;

          I3PositionPtr detectorCenter_;
          I3RandomServicePtr rand_ ; 
          uint64_t daqtime_;
          uint32_t eventId_;
          unsigned nEvents_;
          unsigned nsubframes_;

  public:

         DSTTestGenerator(const I3Context& context); 
            
         void Configure(); 

         void DAQ(I3FramePtr frame); 
         
         void SetTrigger(I3TriggerHierarchyPtr& triggers, uint16_t triggertag);
  };
  
