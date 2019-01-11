#include <I3Test.h>
#include <icetray/I3TrayHeaders.h>
#include "dataclasses/I3Constants.h"

#include "recclasses/I3OpheliaFirstGuessTrack.h"
#include "I3OpheliaTestModule.h"

//using namespace std;

I3_MODULE(I3OpheliaTestModule);

I3OpheliaTestModule::I3OpheliaTestModule(const I3Context& context) : I3Module(context)
{
  AddOutBox("OutBox");
}

void
I3OpheliaTestModule::Configure()
{}

void
I3OpheliaTestModule::Physics(I3FramePtr frame)
{

  I3OpheliaFirstGuessTrackConstPtr opheliaFG_ptr 
    = frame->Get<I3OpheliaFirstGuessTrackConstPtr>("OpheliaFirstGuess");

  ENSURE(bool(opheliaFG_ptr));

  if(opheliaFG_ptr->IsFitSuccessful()){
  ENSURE_DISTANCE(opheliaFG_ptr->GetConstI3Particle().GetZenith(), M_PI/2., M_PI/2., "Reconstructed zenith angle is not reasonable value");
  ENSURE_DISTANCE(opheliaFG_ptr->GetConstI3Particle().GetAzimuth(), M_PI, M_PI, "Reconstructed azimth angle is not reasonable value");
  }

  PushFrame(frame,"OutBox");
}
