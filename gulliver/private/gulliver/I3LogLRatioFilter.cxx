/**
 * copyright (C) 2007
 * the IceCube collaboration
 * $Id:$
 *
 * @file I3LogLRatioFilter.cxx
 * @version $Revision$
 * @author Anna Franckowiak
 * @date Oct 22 2007
 */

#include <gulliver/I3LogLRatioFilter.h>

#include <icetray/I3Context.h>
#include <icetray/I3Frame.h>
#include <icetray/I3IcePickInstaller.h>
#include <interfaces/I3IcePickModule.h>
#include <interfaces/I3IceForkModule.h>
#include <dataclasses/physics/I3Particle.h>
#include <gulliver/I3LogLikelihoodFitParams.h>


using namespace std;


I3LogLRatioFilter::I3LogLRatioFilter(const I3Context& context) : 
  I3IcePick(context),
  particleKey1_("ipdfGCpandel"),
  particleKey2_("bayesianP_1it"),
  maxLogLRatio_(3.0)
{
  AddParameter("ParticleKey1",
           "Name of the likelihood reconstruction",
           particleKey1_);
  AddParameter("ParticleKey2",
           "Name of another likelihood reconstruction that will be subtracted",
           particleKey2_);
  AddParameter("MaxLogLRatio",
           "The maximal difference of the log likelihoods "
               "given by the two reconstructions above",
           maxLogLRatio_);
}


I3LogLRatioFilter::~I3LogLRatioFilter()
{
}


void I3LogLRatioFilter::Configure()
{
  GetParameter("ParticleKey1",particleKey1_);
  GetParameter("ParticleKey2",particleKey2_);
  GetParameter("MaxLogLRatio",maxLogLRatio_);
}


bool I3LogLRatioFilter::SelectFrame(I3Frame& frame)
{
  bool retVal = false; // skip event

  I3ParticleConstPtr particle1 = frame.Get<I3ParticleConstPtr>(particleKey1_);
  string fitParamsName1 = particleKey1_ + "FitParams";
  I3LogLikelihoodFitParamsConstPtr llhParams1 =
    frame.Get<I3LogLikelihoodFitParamsConstPtr>(fitParamsName1);

  I3ParticleConstPtr particle2 = frame.Get<I3ParticleConstPtr>(particleKey2_);
  string fitParamsName2 = particleKey2_ + "FitParams";
  I3LogLikelihoodFitParamsConstPtr llhParams2 =
    frame.Get<I3LogLikelihoodFitParamsConstPtr>(fitParamsName2);

  if(!particle1){
    log_debug("Event has no particle \"%s\".  Skip Event!!!",
         particleKey1_.c_str());
  }
  else if(!llhParams1){
    log_debug("Event has no llh parameter \"%s\".  Skip Event!!!",
         fitParamsName1.c_str());
  }
  else if(I3Particle::OK != particle1->GetFitStatus()){
    log_debug("Fit \"%s\" was not successful.  Skip Event!!!",
         particleKey1_.c_str());
  }
  else if(!particle2){
    log_debug("Event has no particle \"%s\".  Skip Event!!!",
         particleKey2_.c_str());
  }
  else if(!llhParams2){
    log_debug("Event has no llh parameter \"%s\".  Skip Event!!!",
         fitParamsName2.c_str());
  }
  else if(I3Particle::OK != particle2->GetFitStatus()){
    log_debug("Fit \"%s\" was not successful.  Skip Event!!!",
         particleKey2_.c_str());
  }
  else{
    // test likelihoods, keep events with LLHRatio < cut value
    retVal = (llhParams1->logl_ - llhParams2->logl_) < maxLogLRatio_;
  }

  return retVal;
}



I3_MODULE(I3IcePickModule<I3LogLRatioFilter>);
I3_MODULE(I3IceForkModule<I3LogLRatioFilter>);
I3_SERVICE_FACTORY(I3IcePickInstaller<I3LogLRatioFilter>);
