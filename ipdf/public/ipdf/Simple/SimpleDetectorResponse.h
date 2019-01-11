#ifndef IPDF_SimpleDetectorResponse_H
#define IPDF_SimpleDetectorResponse_H

/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @file SimpleDetectorResponse.h
    @version $Revision: 1.3 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>
*/

#include <vector>

namespace IPDF {
class SimpleHitOm;

/**
 * @brief Basic implementation of the detector response.
 *
 * Basically is a container for the hit oms.
 */
class SimpleDetectorResponse {
public:
  typedef std::vector<SimpleHitOm*>::const_iterator const_iterator;
  typedef SimpleHitOm HitOm;
  typedef SimpleHitOm value_type;  // conform to STL

  /// @brief Ctor from ipdf vector of native HitOms
  explicit SimpleDetectorResponse(const std::vector<SimpleHitOm*> hitoms)
      : hitoms_(hitoms) { }

  /// @brief Ctor from an ipdf native HitOm
  explicit SimpleDetectorResponse(SimpleHitOm* hitom)
      : hitoms_(1,hitom) { }

  /// @brief Dtor: destroys all the contained PEHits
  ~SimpleDetectorResponse() {
    for(SimpleDetectorResponse::iterator hiter = hitoms_.begin();
	hiter != hitoms_.end(); ++hiter) {
      delete *hiter;
    }
  }
  
  /// @brief Access an HitOm via a simple integer key
  const SimpleHitOm& operator[](int ihit) const {
    return *hitoms_[ihit];
  }

  /// @brief STL compliant iterator
  const_iterator begin() const { return hitoms_.begin(); }
  /// @brief STL compliant iterator
  const_iterator end() const { return hitoms_.end(); }
  /// @brief STL compliance
  size_t size() const { return hitoms_.size(); }
  /// @brief STL compliance
  bool empty() const { return hitoms_.empty(); }
  
private:
  typedef std::vector<SimpleHitOm*>::iterator iterator;
  std::vector<SimpleHitOm*> hitoms_;
};

} // namespace IPDF

#endif // IPDF_SimpleDetectorResponse_H
