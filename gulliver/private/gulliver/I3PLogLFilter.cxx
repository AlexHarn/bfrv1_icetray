/**
 * copyright (C) 2007
 * the IceCube collaboration
 * $Id:$
 *
 * @file I3PLogLFilter.cxx
 * @version $Revision$
 * @author Anna Franckowiak
 * @date Oct 22 2007
 */

#include <gulliver/I3PLogLFilter.h>

#include <icetray/I3Context.h>
#include <icetray/I3Frame.h>
#include <icetray/I3IcePickInstaller.h>
#include <interfaces/I3IcePickModule.h>
#include <interfaces/I3IceForkModule.h>
#include <dataclasses/physics/I3Particle.h>
#include <gulliver/I3LogLikelihoodFitParams.h>
#include <dataclasses/physics/I3RecoPulse.h>
#include <phys-services/I3CutValues.h>

using namespace std;


I3PLogLFilter::I3PLogLFilter(const I3Context& context) : 
    I3IcePick(context),
    pulseSeriesName_(""),
    particleKey_("ipdfGCpandel"),
    cutValuesKey_(""),
    subFromNCh_(2.0),
    maxPLogL_(7.5)
{
  AddParameter("ParticleKey",
           "Name of the likelihood reconstruction",
           particleKey_);
  AddParameter("CutValuesKey",
               "Name of cut values to read NCh from, if no pulse series is in the frame",
               cutValuesKey_);
  AddParameter("MaxPLogL",
           "The maximal likelihood/(NCh - SubstractFromNCh) above which events are cut",
           maxPLogL_);
  AddParameter("SubstractFromNCh",
               "Substract this value from NCh before the likelihood is divided",
               subFromNCh_);
  AddParameter("RecoPulseSeries",
            "The name of the reco pulse series to calculate NCh",
            pulseSeriesName_);
}


I3PLogLFilter::~I3PLogLFilter()
{
}


void I3PLogLFilter::Configure()
{
  GetParameter("ParticleKey",particleKey_);
  GetParameter("CutValuesKey",cutValuesKey_);
  GetParameter("MaxPLogL",maxPLogL_);
  GetParameter("SubstractFromNCh",subFromNCh_);
  GetParameter("RecoPulseSeries", pulseSeriesName_);
}


bool I3PLogLFilter::SelectFrame(I3Frame& frame)
{
  bool retVal = false; // skip event

  I3ParticleConstPtr particle = frame.Get<I3ParticleConstPtr>(particleKey_);
  string fitParamsName = particleKey_ + "FitParams";
  I3LogLikelihoodFitParamsConstPtr llhParams =
    frame.Get<I3LogLikelihoodFitParamsConstPtr>(fitParamsName);
  I3RecoPulseSeriesMapConstPtr pulseSeries = frame.Get<I3RecoPulseSeriesMapConstPtr>(pulseSeriesName_); 
  I3CutValuesConstPtr cutValues = frame.Get<I3CutValuesConstPtr>(cutValuesKey_);

  if(!particle){
    log_debug("Event has no particle \"%s\".  Skip Event!!!",
         particleKey_.c_str());
  }
  else if(!llhParams){
    log_debug("Event has no llh parameter \"%s\".  Skip Event!!!",
         (fitParamsName).c_str());
  }
  else if(!pulseSeries && !cutValues){
    log_debug("Event has no pulse series \"%s\" and no cut values \"%s\". Skip Event!!!",
         pulseSeriesName_.c_str(),cutValuesKey_.c_str());
  }
  else if(I3Particle::OK != particle->GetFitStatus()){
    log_debug("Fit \"%s\" was not successful.  Skip Event!!!",
         particleKey_.c_str());
  }
  else{
    // test reduced likelihood, keep events with LogL/(NCh - 2) < cut value
    int nch = 0;
    if(pulseSeries)
        nch = pulseSeries->size();
    else
        nch = cutValues->Nchan;
    if (nch > 0){
        retVal = (llhParams->logl_/(nch - subFromNCh_)) < maxPLogL_;
    }
  }
  return retVal;
}



I3_MODULE(I3IcePickModule<I3PLogLFilter>);
I3_MODULE(I3IceForkModule<I3PLogLFilter>);
I3_SERVICE_FACTORY(I3IcePickInstaller<I3PLogLFilter>);
