#ifndef IPDF_SimpleHitOm_H
#define IPDF_SimpleHitOm_H

/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @file SimpleHitOm.h
    @version $Revision: 1.4 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>
*/

#include <vector>
#include <algorithm>

#include "ipdf/Simple/SimplePEHit.h"

namespace IPDF {

class SimpleOmReceiver;
  
/**
 * @brief Basic implementation of an hit OM
 */
class SimpleHitOm {
public:
  typedef double npe_result;
  typedef SimplePEHit value_type;
  typedef std::vector<SimplePEHit*>::const_iterator const_iterator;
  typedef SimpleOmReceiver OmReceiver;

  /// @brief Ctor from ipdf native OmReceiver and vector of PEHits
  explicit SimpleHitOm(const SimpleOmReceiver& omr,
		       std::vector<SimplePEHit*> hits)
      : omr_(omr), photon_hits_(hits),
	num_photo_electrons_(-1.)  { }

  /// @brief Ctor from ipdf native OmReceiver and a single PEHit
  explicit SimpleHitOm(const SimpleOmReceiver& omr,
		       SimplePEHit* hit)
      : omr_(omr), photon_hits_(1,hit),
	num_photo_electrons_(-1.)  { }

  ~SimpleHitOm() {
    for(SimpleHitOm::iterator hiter = photon_hits_.begin();
	hiter != photon_hits_.end(); ++hiter) {
      delete *hiter;
    }
  }

  /// @brief Access the OmReceiver on which this hit occurred
  const SimpleOmReceiver& getOmReceiver() const { return omr_; }

  /// @brief Access total charge measured in photo-electrons
  npe_result getTotalPe() const {
    if( num_photo_electrons_ < 0. ) {
      num_photo_electrons_=0.;
      SimpleHitOm::const_iterator iter;
      for(iter=this->begin();iter!=this->end();++iter)
	num_photo_electrons_+=(*iter)->getPeAmplitude();
    }
    return num_photo_electrons_;
  }

  /// @brief Access to all the photo-electron hits on this OM in this event
  const std::vector<SimplePEHit*> getPEHits() const {
    return photon_hits_;
  }

  /// @brief Access an PEHit via a simple integer key
  const SimplePEHit& operator[](int ihit) const {
    return *photon_hits_[ihit];
  }

  /// @brief STL compliant iterator
  const_iterator begin() const { return photon_hits_.begin(); }
  /// @brief STL compliant iterator
  const_iterator end() const { return photon_hits_.end(); }
  /// @brief STL compliatce
  bool empty() const { return photon_hits_.empty(); }
  /// @brief STL compliatce
  size_t size() const { return photon_hits_.size(); }
  
private:
  typedef std::vector<SimplePEHit*>::iterator iterator;
  const SimpleOmReceiver& omr_;
  std::vector<SimplePEHit*> photon_hits_;
  mutable double num_photo_electrons_;
};

} // namespace IPDF

#endif // IPDF_SimpleHitOm_H
