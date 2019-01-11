/**
 * copyright  (C) 2004
 * the icecube collaboration
 * $Id: I3OpheliaConvertFirstGuessTrack.cxx 19641 2006-05-10 14:03:22Z dule $
 *
 * @file I3OpheliaConvertFirstGuessTrack.cxx
 * @version $Revision: 1.7 $
 * @date $Date: 2006-05-10 23:03:22 +0900 (水, 10  5月 2006) $
 * @author mase
 */

#include "icetray/I3TrayHeaders.h"
#include "recclasses/I3OpheliaFirstGuessTrack.h"
#include "ophelia/util/I3OpheliaConvertFirstGuessTrack.h"

#include <iostream>
//#include <boost/iterator/filter_iterator.hpp>

using namespace std;

I3_MODULE(I3OpheliaConvertFirstGuessTrack);


/* ******************************************************************** */
/* Constructor                                                          */
/* ******************************************************************** */
I3OpheliaConvertFirstGuessTrack::I3OpheliaConvertFirstGuessTrack(const I3Context& ctx) : I3ConditionalModule(ctx)
{
  AddOutBox("OutBox");

  inputOpheliaFGTrack_ = "OpheliaFirstGuess";
  AddParameter("InputOpheliaFGTrackName", 
	       "Input name of I3OpheliaFirstGuessTrack", inputOpheliaFGTrack_);

  outputParticle_ = "OpheliaFirstGuessConverted";
  AddParameter("OutputParticleName",
	       "Output name of I3Particle", outputParticle_);
}


/* ******************************************************************** */
/* Destructor                                                            */
/* ******************************************************************** */
I3OpheliaConvertFirstGuessTrack::~I3OpheliaConvertFirstGuessTrack(){
}



/* ******************************************************************** */
/* Configure                                                            */
/* ******************************************************************** */
void I3OpheliaConvertFirstGuessTrack::Configure()
{
  GetParameter("InputOpheliaFGTrackName",inputOpheliaFGTrack_);
  GetParameter("OutputParticleName",outputParticle_);

  log_info ("Input:  InputOpheliaFGTrackName = %s",inputOpheliaFGTrack_.c_str());
  log_info ("Output: OutputParticleName = %s",outputParticle_.c_str());

}


/* ******************************************************************** */
/* Physics                                                              */
/* ******************************************************************** */
void I3OpheliaConvertFirstGuessTrack::Physics(I3FramePtr frame)
{
  log_debug("Entering Physics...");

  // Get I3OpheliaFirstGuessTrack info from the frame.
  I3OpheliaFirstGuessTrackConstPtr opheliaFGPtr = 
    frame->Get<I3OpheliaFirstGuessTrackConstPtr>(inputOpheliaFGTrack_);

  if (!opheliaFGPtr)
    {
      log_info("Couldn't find input I3OpheliaFirstGuessTrack.");

    }

  else
    {
      // Create the result particle so we can fill it later and put it in frame.
      I3ParticlePtr track(
              new I3Particle( I3Particle::InfiniteTrack, I3Particle::MuMinus )
              );

      // Set track members.
      track->SetPos(opheliaFGPtr->GetConstI3Particle().GetPos());
      track->SetDir(opheliaFGPtr->GetConstI3Particle().GetDir());
      track->SetFitStatus(opheliaFGPtr->GetConstI3Particle().GetFitStatus());
      track->SetSpeed(opheliaFGPtr->GetConstI3Particle().GetSpeed());
      track->SetTime(opheliaFGPtr->GetConstI3Particle().GetTime());

      // Put it to the frame
      frame->Put(outputParticle_, track);
    }

  PushFrame(frame,"OutBox");

  log_debug("Exiting Physics.");
}
