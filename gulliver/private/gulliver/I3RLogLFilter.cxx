/**
 * copyright (C) 2007
 * the IceCube collaboration
 * $Id:$
 *
 * @file I3RLogLFilter.cxx
 * @version $Revision$
 * @author Anna Franckowiak
 * @date Oct 22 2007
 */

#include <gulliver/I3RLogLFilter.h>

#include <icetray/I3Context.h>
#include <icetray/I3Frame.h>
#include <icetray/I3IcePickInstaller.h>
#include <interfaces/I3IcePickModule.h>
#include <interfaces/I3IceForkModule.h>
#include <dataclasses/physics/I3Particle.h>
#include <gulliver/I3LogLikelihoodFitParams.h>


using namespace std;


I3RLogLFilter::I3RLogLFilter(const I3Context& context) : 
    I3IcePick(context),
    particleKey_("ipdfGCpandel"),
    maxRLogL_(9.0)
{
  AddParameter("ParticleKey",
           "Name of the likelihood reconstruction",
           particleKey_);
  AddParameter("MaxRLogL",
           "The maximal reduced likelihood above which events are cut",
           maxRLogL_);
}


I3RLogLFilter::~I3RLogLFilter()
{
}


void I3RLogLFilter::Configure()
{
  GetParameter("ParticleKey",particleKey_);
  GetParameter("MaxRLogL",maxRLogL_);
}


bool I3RLogLFilter::SelectFrame(I3Frame& frame)
{
  bool retVal = false; // skip event

  I3ParticleConstPtr particle = frame.Get<I3ParticleConstPtr>(particleKey_);
  string fitParamsName = particleKey_ + "FitParams";
  I3LogLikelihoodFitParamsConstPtr llhParams =
    frame.Get<I3LogLikelihoodFitParamsConstPtr>(fitParamsName);

  if(!particle){
    log_warn("Event has no particle \"%s\".  Skip Event!!!",
         particleKey_.c_str());
  }
  else if(!llhParams){
    log_warn("Event has no llh parameter \"%s\".  Skip Event!!!",
         (fitParamsName).c_str());
  }
  else if(I3Particle::OK != particle->GetFitStatus()){
    log_warn("Fit \"%s\" was not successful.  Skip Event!!!",
         particleKey_.c_str());
  }
  else{
    // test reduced likelihood, keep events with RLogL < cut value
    retVal = llhParams->rlogl_ < maxRLogL_;
  }

  return retVal;
}



I3_MODULE(I3IcePickModule<I3RLogLFilter>);
I3_MODULE(I3IceForkModule<I3RLogLFilter>);
I3_SERVICE_FACTORY(I3IcePickInstaller<I3RLogLFilter>);
