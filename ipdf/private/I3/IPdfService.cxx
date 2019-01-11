

#include "ipdf/I3/IPdfService.h"

#include "icetray/I3Frame.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/physics/I3RecoHit.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/physics/I3AMANDAAnalogReadout.h"

I3_SERVICE_FACTORY(IPdfServiceFactory)

IPdfService::IPdfService()
 : inputDataReadout_("CleanHits"),
   inputSeedName_("NotSet")
{
}

void IPdfService::Geometry(const I3FramePtr frame) {
  log_debug("Entering IPdfService::Geometry()");

  try {
    const I3Geometry& i3geom = frame->Get<I3Geometry>();
    dgeom_ = boost::shared_ptr<IPDF::I3DetectorConfiguration>(
	new IPDF::I3DetectorConfiguration(i3geom)
	);
  } catch(std::exception& e) {
    log_error("failed to get detector geometry. %s",e.what());
    throw;
  }
}

void IPdfService::Physics(const I3FramePtr frame) {
  log_debug("Entering IPdfService::Physics()");

  /// @todo Do we want to handle the seed internally???
#if 0
  I3ParticleConstPtr hypothesis = this->retrieveSeed(frame);

  if(!this->checkTrack(hypothesis)) {
    log_warn("Skipping an event because the input track was unusable");
    return;
  }
#endif

  // Try to find some kind of detector response:
  // (can do I3RecoHitSeries, I3RecoPulseSeries or I3AMANDAAnalogReadoutMap)
  I3RecoHitSeriesMapConstPtr recohits =
      frame->Get<I3RecoHitSeriesMapConstPtr>(inputDataReadout_);

  if(recohits) {
    this->makeResponse(recohits);
  } else {

    I3RecoPulseSeriesMapConstPtr recopuls =
	frame->Get<I3RecoPulseSeriesMapConstPtr>(inputDataReadout_);

    if(recopuls) {
      this->makeResponse(recopuls);

    } else {
      I3AMANDAAnalogReadoutMapConstPtr amareads =
	  frame->Get<I3AMANDAAnalogReadoutMapConstPtr>(inputDataReadout_);

      if(amareads) {
	this->makeResponse(amareads);
      } else {
	log_fatal("Failed to get any detector response named \"%s\".",
	    inputDataReadout_.c_str());
      }
    }
  }
}

template<class Readout>
void IPdfService::makeResponse(const Readout& response) {
  if(!response) {
    log_debug("info: failed to get detector response named \"%s\" (may not be serious).",
	inputDataReadout_.c_str());
    return;
  }
  log_debug("found detector response \"%s\".",inputDataReadout_.c_str());

  IPDF::I3DetectorResponse dresponse(response,*dgeom_);
  dresponse_ = boost::shared_ptr<DResponse>(new DResponse(response,*dgeom_));
}

I3ParticleConstPtr IPdfService::retrieveSeed(const I3FramePtr& frame) const {
  I3ParticleConstPtr the_track = frame->Get<I3ParticleConstPtr>(inputSeedName_);
  if(the_track) return the_track;

  I3Frame::const_iterator iter;
  // for all tracks in the multimap with the right name, get the first
  // one that's a muon.
  for(iter = frame->begin() ;
      iter != frame->end();
      iter++)
  {
    I3ParticleConstPtr track 
	= boost::dynamic_pointer_cast<I3ParticleConstPtr::element_type>(iter->second);
    if(track)
      if(track->IsTrack() &&
         ((track->GetType() == I3Particle::MuMinus) ||
	  (track->GetType() == I3Particle::MuPlus)))
      {
	the_track = track;
	break;
      }
  }
  if(!the_track)
  {
    log_fatal("FATAL: couldn't find a seed (tried \"%s\").",inputSeedName_.c_str());
  }

  return the_track;
}

