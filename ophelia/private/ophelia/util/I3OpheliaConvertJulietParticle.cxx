/**
 * copyright  (C) 2004
 * the icecube collaboration
 * $Id: I3OpheliaConvertFirstGuessTrack.cxx 19641 2006-05-10 14:03:22Z dule $
 *
 * @file I3OpheliaConvertJulietParticle.cxx
 * @version $Revision: 1.7 $
 * @date $Date: 2006-05-10 23:03:22 +0900 (水, 10  5月 2006) $
 * @author mase
 */

#include "icetray/I3TrayHeaders.h"
#include "ophelia/util/I3OpheliaConvertJulietParticle.h"
#include "dataclasses/physics/I3MCTreeUtils.h"

#include <iostream>
//#include <boost/iterator/filter_iterator.hpp>

using namespace std;

I3_MODULE(I3OpheliaConvertJulietParticle);


/* ******************************************************************** */
/* Constructor                                                          */
/* ******************************************************************** */
I3OpheliaConvertJulietParticle::I3OpheliaConvertJulietParticle(const I3Context& ctx) : I3ConditionalModule(ctx)
{
  AddOutBox("OutBox");

  inputMCTree_ = "I3MCTree";
  AddParameter("InputMCTreeName", 
	       "Input name of I3MCTree", inputMCTree_);

  outputParticle_ = "ConvertedJulietParticle";
  AddParameter("OutputParticleName",
	       "Output name of I3Particle", outputParticle_);
}


/* ******************************************************************** */
/* Destructor                                                            */
/* ******************************************************************** */
I3OpheliaConvertJulietParticle::~I3OpheliaConvertJulietParticle(){
}



/* ******************************************************************** */
/* Configure                                                            */
/* ******************************************************************** */
void I3OpheliaConvertJulietParticle::Configure()
{
  GetParameter("InputMCTreeName",inputMCTree_);
  GetParameter("OutputParticleName",outputParticle_);

  log_info ("Input:  InputMCTreeName = %s",inputMCTree_.c_str());
  log_info ("Output: OutputParticleName = %s",outputParticle_.c_str());

}


/* ******************************************************************** */
/* Physics                                                              */
/* ******************************************************************** */
void I3OpheliaConvertJulietParticle::Physics(I3FramePtr frame)
{
  log_debug("Entering Physics...");

  I3MCTreeConstPtr mc_tree_ptr = frame->Get<I3MCTreeConstPtr>(inputMCTree_);

  if(!mc_tree_ptr)
    log_fatal("This event has NO I3MCTree!");
  
  const std::vector<I3Particle> primaries 
    = I3MCTreeUtils::GetPrimaries(*mc_tree_ptr);
  
  if(primaries.size()==0){
    log_error("There is no primary.");
  }else if(primaries.size()>1){
    log_error("Multiple primary found. Juliet primary should be one.");
  }

  // Create the result particle so we can fill it later and put it in frame.
  I3ParticlePtr track(
          new I3Particle( I3Particle::InfiniteTrack, I3Particle::MuMinus )
          );

  I3Position  pos = primaries[0].GetPos();
  I3Direction dir = primaries[0].GetDir();
  double energy = primaries[0].GetEnergy();

  // Set track members.
  track->SetPos(pos);
  track->SetDir(dir);
  track->SetTime(0.0);
  track->SetEnergy(energy);
  track->SetFitStatus(I3Particle::OK);

  // Put it to the frame
  frame->Put(outputParticle_, track);
  PushFrame(frame,"OutBox");

  log_debug("Exiting Physics.");
}
