/**
 * @brief implementation of the I3GulliverFinitePhPnh class
 *
 * @file I3GulliverFinitePhPnh.cxx
 * @version $Revision$
 * @date $Date$
 * @author Sebastian Euler <sebastian.euler@icecube.wisc.edu>
 *
 * This class calculates a PDF based on the probability of a DOM to have a hit from a given track. By this the PDF is sensitive to finite tracks. The implementation is done as an I3PDFService for gulliver.
 * There are various options coming along with this PDF, which are set by I3GulliverFinitePhPnhFactory.
 */

#include "finiteReco/I3GulliverFinitePhPnh.h"
#include "finiteReco/StringLLH.h"
#include "finiteReco/probability/PhPnhProbBase.h"
#include "icetray/I3Units.h"
#include "icetray/I3Frame.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/physics/I3EventHeader.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3Particle.h"
#include "gulliver/I3EventHypothesis.h"
#include "gulliver/I3PDFBase.h"
#include "phys-services/I3Calculator.h"
#include <float.h>

using namespace std;

I3GulliverFinitePhPnh::I3GulliverFinitePhPnh( const string &name,
                                              const string &inputreadout,
                                              const double &noiserate,
                                              const double &defaultEventDuration,
                                              const double& rCylinder,
                                              const bool &FlagStringLLH,
                                              const vector<int> selectedStrings,
                                              const PhPnhProbBasePtr prob,
                                              const bool& useOnlyFirstHit,
                                              const double& probMultiDet):
  I3EventLogLikelihoodBase(),
  name_(name),
  inputReadout_(inputreadout),
  noiseRate_(noiserate),
  defaultEventDuration_(defaultEventDuration),
  rCylinder_(rCylinder),
  flagStringLLH_(FlagStringLLH),
  selectedStrings_(selectedStrings),
  prob_(prob),
  multi_(0),
  useOnlyFirstHit_(useOnlyFirstHit),
  probMultiDet_(probMultiDet)
{
  if(probMultiDet_ < 0 || probMultiDet_ > 1) log_fatal("Not able to determine the multiplicity!");
  llhName_ = "FinitePhPnh";
  log_info( "(%s) will try to get pulses from %s",
            name_.c_str(), inputReadout_.c_str() );
  if(noiserate <= 0) log_info("You set zero noise rate. Might be still OK");
}

// Functions to get charge from RecoPulses.
static double GetCharge(const I3RecoPulse& pulse)
{ return (pulse.GetCharge() >= 2.0) ? pulse.GetCharge() : 1; }
static double GetTime(const I3RecoPulse& pulse) { return pulse.GetTime(); }

I3GulliverFinitePhPnh::~I3GulliverFinitePhPnh(){
}

// Local function to extract pulses from frame.
// Copied from I3DipoleFitExtras.h, and made more efficient.
// (Do the omgeo.find *after* checking that there are hits.)
void FillHits( I3RecoPulseSeriesMapConstPtr pulsemap,
               const I3Geometry& geometry,
               const string &name,
               map<OMKey,I3GulliverFinitePhPnh::I3MHit>& hitvect,
               int& multi)
{
  multi = pulsemap->size();
  I3RecoPulseSeriesMap::const_iterator ipulsemap;
  for(ipulsemap = pulsemap->begin(); ipulsemap != pulsemap->end(); ++ipulsemap){
    const OMKey& om = ipulsemap -> first;
    if(hitvect.count(om) == 0) log_fatal("This OM (Key:%d-%d) does not exist",om.GetString(),om.GetOM());
    else if(hitvect.count(om) > 1)  log_fatal("This OM (Key:%d-%d) exists twice",om.GetString(),om.GetOM());
    const vector<I3RecoPulse>& hits = ipulsemap->second;
    if(! hits.empty()){
       vector<I3RecoPulse>::const_iterator ihits = hits.begin();
       I3GulliverFinitePhPnh::I3MHit& mhit = hitvect[om];
       mhit.t = GetTime(*ihits);
       mhit.a = GetCharge(*ihits);
       mhit.n = hits.size();
    }
  }
}

bool I3GulliverFinitePhPnh::IsStringSelected(const int& stringNr){
  vector<int>::const_iterator iString = selectedStrings_.begin();
  if(iString==selectedStrings_.end()) log_fatal("No Strings selected!!");
  for(;iString!=selectedStrings_.end();iString++){
    if(stringNr==*iString) return true;
  }
  return false;
}

void I3GulliverFinitePhPnh::SetEvent( const I3Frame &f ){
  I3RecoPulseSeriesMapConstPtr pulsemap =
    f.Get<I3RecoPulseSeriesMapConstPtr>(inputReadout_);
  
  const I3Geometry &geometry = f.Get<I3Geometry>();
  const I3EventHeader& header= f.Get<I3EventHeader>("I3EventHeader");

  double time = header.GetEndTime() - header.GetStartTime(); // I3Time::- returns time difference in ns!!
  if(time > 1.*I3Units::s || time < 0. || !isnormal(time)){
    log_info("event duration is unknown. %f ns used", defaultEventDuration_);
    time = defaultEventDuration_;
  }
  minProb_ = 1. - exp(-noiseRate_*time);
  if(minProb_ < 1.e-6){
    log_warn("minProb_ value too small, check noiseRate_ or EventHeader; set to 1e-6");
    minProb_ = 1.e-6;
  }
  
  if(hits_.empty()){// not perfect
    I3OMGeoMap::const_iterator igeo;
    for(igeo = geometry.omgeo.begin(); igeo!=geometry.omgeo.end(); ++igeo){
      if(IsStringSelected(igeo->first.GetString())){
        if(igeo->second.omtype != I3OMGeo::IceTop){
          hits_[igeo->first].t = NAN;
          hits_[igeo->first].a = NAN;
          hits_[igeo->first].n = 0;
          hits_[igeo->first].omgeo = igeo->second;
        }
      }
    }
  }
  else{
    map<OMKey,I3GulliverFinitePhPnh::I3MHit>::iterator ihits;
    for(ihits=hits_.begin(); ihits!=hits_.end(); ++ihits){
      ihits->second.t = NAN;
      ihits->second.a = NAN;
      ihits->second.n = 0;
    }
  }
  if(pulsemap) FillHits(pulsemap, geometry, name_, hits_, multi_);
  else log_fatal("(%s) can't find the pulses with label %s",
                 name_.c_str(), inputReadout_.c_str() );
  
  log_debug("(%s) got %zu pulses", name_.c_str(), hits_.size() );
}

unsigned int I3GulliverFinitePhPnh::GetMultiplicity(){
  if(multi_ == 0){
    log_info("multiplicity is not calculated OR no DOMs within a prob. > %e",probMultiDet_);
  }
  return multi_;
}

//double I3GulliverFinitePhPnh::GetProbCylinder( const I3Particle& track, const I3Position& pos, const int& Nhit){
double I3GulliverFinitePhPnh::GetProbCylinder( const I3Particle& track, const I3OMGeo& omgeo, const int& Nhit){
  double distAlong = I3Calculator::DistanceAlongTrack(track,omgeo.position);
  double dist      = (omgeo.position-track.GetPos()).Magnitude();
  double distPerp  = dist*dist - distAlong*distAlong;
  if (distPerp > rCylinder_*rCylinder_){
    return minProb_;
  }
  
  double physProb = prob_->GetHitProb(track,omgeo,Nhit);
  double totProb  = minProb_ + physProb - minProb_*physProb;
  
  if (totProb<=0. || !isnormal(totProb)) {
    log_warn("no correct value for the probability: %e",totProb);
    return 0.;
  }
  else if (totProb>=1.) {
    log_info("probability >= 1: %e, using largest representable value < 1",totProb);
    return 1-numeric_limits<double>::epsilon();
  }
  
  return totProb;
}

double I3GulliverFinitePhPnh::GetLogLikelihoodString( const I3Particle& track ){
  double llh = 0.0;
  StringLLH stringProb;

  map<OMKey,I3GulliverFinitePhPnh::I3MHit>::iterator ihit;
  multi_ = 0;
  for ( ihit = hits_.begin(); ihit != hits_.end(); ++ihit ){
    int stringNr = ihit->first.GetString();
    if (stringNr != stringProb.GetStringNr()){
      llh += stringProb.GetProb();
      stringProb.NewString(stringNr);
    } 
    
    double tot_prob;
    if(useOnlyFirstHit_){ 
      tot_prob = GetProbCylinder(track,ihit->second.omgeo,-1);
      stringProb.AddOM(tot_prob,!std::isnan(ihit->second.t), false);
    }
    else{
      tot_prob = GetProbCylinder(track,ihit->second.omgeo,ihit->second.n);
      if(ihit->second.n == 0) stringProb.AddOM(1-tot_prob,!std::isnan(ihit->second.t), false);
      else stringProb.AddOM(tot_prob,!std::isnan(ihit->second.t), false);
    }
    
    if(tot_prob > probMultiDet_) multi_++;
  }
  llh += stringProb.GetProb();
  return llh;
}

double I3GulliverFinitePhPnh::GetLogLikelihoodOM( const I3Particle& track ){
  double llh = 0.0;
  map<OMKey,I3GulliverFinitePhPnh::I3MHit>::iterator ihit;
  multi_ = 0;
  for ( ihit = hits_.begin(); ihit != hits_.end(); ++ihit ){
    double tot_prob;
    double log_pdf;
    if(useOnlyFirstHit_){
      tot_prob = GetProbCylinder(track,ihit->second.omgeo,-1);
      log_pdf = ( std::isnan(ihit->second.t) ) ? log(1-tot_prob) : log(tot_prob) ;
    }
    else{
      tot_prob = GetProbCylinder(track,ihit->second.omgeo,ihit->second.n);
      log_pdf  = log(tot_prob);
    }
    
    if(tot_prob > probMultiDet_) multi_++; 
    if(std::isinf(log_pdf) || std::isnan(log_pdf)) log_warn("Log of probability has no normal value (%e,%e)",log_pdf,tot_prob);
    llh += log_pdf;
  }
  return llh;
}

double I3GulliverFinitePhPnh::GetLogLikelihood( const I3EventHypothesis &q ){
  const I3Particle &track = *(q.particle);
  double llh = (flagStringLLH_)? GetLogLikelihoodString(track) : GetLogLikelihoodOM(track);
  return llh;
}
