#ifndef IPDF_I3DetectorResponse_H
#define IPDF_I3DetectorResponse_H

/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @file I3DetectorResponse.h
    @version $Revision: 1.2 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>
*/

#include <vector>
#include <iosfwd>

#include "ipdf/I3/I3DetectorConfiguration.h"
#include "icetray/I3Logging.h"
#include "dataclasses/I3Vector.h"
#include "dataclasses/physics/I3AMANDAAnalogReadout.h"
#include "ipdf/I3/I3HitOm.h"
#include "ipdf/I3/I3OmReceiver.h"

namespace IPDF {
class I3HitOm;

  /**
    @brief Implementation of the detector response in the Icecube framework.  

    Basically is a container for the hit oms.  This class owns 
    the oms it contains and as such deletes them when it is 
    destroyed.
   */
class I3DetectorResponse {
public:
  typedef std::vector<I3HitOm*>::const_iterator const_iterator;
  typedef I3HitOm HitOm;
  typedef I3HitOm value_type;  // conform to STL

  /// @brief Ctor from ipdf vector of native I3HitOms
  explicit I3DetectorResponse(const std::vector<I3HitOm*> hitoms)
      : hitoms_(hitoms) { }

  /// @brief Ctor from an ipdf native I3HitOm
  explicit I3DetectorResponse(I3HitOm* hitom)
      : hitoms_(1,hitom) { }

  /// @brief Ctor from icetray native hit types
  template<class Response>
  I3DetectorResponse(const Response&,
		     const IPDF::I3DetectorConfiguration&);

  ~I3DetectorResponse();

  /// @brief Access an I3HitOm via a simple integer key
  const I3HitOm& operator[](int ihit) const {
    return *hitoms_[ihit];
  }

  /// @brief STL compliant iterator
  const_iterator begin() const { return hitoms_.begin(); }
  /// @brief STL compliant iterator
  const_iterator end() const { return hitoms_.end(); }
  /// @brief STL compliant size
  unsigned int   size() const { return hitoms_.size(); }
  
  friend std::ostream& operator<<(std::ostream&, const I3DetectorResponse&);

private:
  typedef std::vector<I3HitOm*>::iterator iterator;
  std::vector<I3HitOm*> hitoms_;
  SET_LOGGER( "ipdf" );
};

std::ostream& operator<<(std::ostream& os, 
    const IPDF::I3DetectorResponse&);

template<class Response> inline
I3DetectorResponse::I3DetectorResponse(const Response& i3ResponseMap,
				       const I3DetectorConfiguration& geom)
{
  typedef typename Response::element_type::value_type::second_type HitVector;
  typedef typename Response::element_type::const_iterator selector;
  selector mapIter = i3ResponseMap->begin();
  const selector mapEnd = i3ResponseMap->end();
  unsigned int nch=0;

  for( ; mapIter != mapEnd; ++mapIter ) {
    const I3OmReceiver& omr( geom.find(mapIter->first) );
    const HitVector& pehits = mapIter->second;
    if(pehits.empty()) {
      log_debug("'HitSeries' was found but is empty.");
      continue;
    }
    if(!pehits.empty()) {
      hitoms_.push_back(new I3HitOm(omr,pehits));
      if(hitoms_.back()->empty()) {
        hitoms_.pop_back();
        // Don't allow empty OmHits
        // This condition can happen if the LE times are not finite
        // I3HitOm will issue a log_error in that case
      } else {
        ++nch;
      }
    }
  }
  log_debug( "nch=%d", nch );
  if(nch==0){ log_error( "Failed to find any hits in Response" ); }
}

template<> inline
I3DetectorResponse::I3DetectorResponse(const I3AMANDAAnalogReadoutMapConstPtr& i3ResponseMap,
				       const I3DetectorConfiguration& geom)
{
  typedef I3AMANDAAnalogReadoutMap::const_iterator selector;
  selector mapIter	= i3ResponseMap->begin();
  const selector mapEnd	= i3ResponseMap->end();
  unsigned int nch=0;

  for( ; mapIter != mapEnd; ++mapIter ) {
    const I3OmReceiver& omr( geom.find(mapIter->first) );
    const I3AMANDAAnalogReadout& readout = mapIter->second;
    hitoms_.push_back( new I3HitOm(omr,readout) );
    if(hitoms_.back()->empty()) {
      hitoms_.pop_back();  // Don't allow empty OmHits
    } else {
      ++nch;
    }
  }
  log_warn( "nch=%d", nch );
  if(nch==0){ log_error( "Failed to find any hits in Response" ); }
}

I3_POINTER_TYPEDEFS(I3DetectorResponse);

} // namespace IPDF

#endif // IPDF_I3DetectorResponse_H
