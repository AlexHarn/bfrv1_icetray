#ifndef IPDF_I3DetectorConfiguration_H
#define IPDF_I3DetectorConfiguration_H

/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @file I3DetectorConfiguration.h
    @version $Revision: 1.1 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>
*/

#include <vector>
#include <map>
#include <iosfwd>

#include "dataclasses/geometry/I3OMGeo.h"
#include "ipdf/Utilities/IPdfException.h"

class I3Geometry;

namespace IPDF {
class I3OmReceiver;

  /**
   * @brief Implementation of the detector configuration in the Icecube framework.
   *
   * Basically is a container for the OM Receivers, thus, acting as 
   * the central point of access for the detector geometry and hardware.

    The idea of this class is to act as a central cache for the 
    detector hardware and geometry infomation, in order that it 
    can be accessed quickly inside the ipdf framework.
   */
class I3DetectorConfiguration {
public:
  typedef std::vector<I3OmReceiver*>::const_iterator const_iterator;
  typedef I3OmReceiver OmReceiver;
  typedef I3OmReceiver value_type;  // conform to STL

  /// @brief Ctor from a vector of ipdf native (compatible) OmReceivers
  explicit I3DetectorConfiguration(const std::vector<OmReceiver*> omReceivers)
      : omReceivers_(omReceivers) { }

  /// @brief Ctor from a single ipdf native (compatible) OmReceivers
  explicit I3DetectorConfiguration(OmReceiver* omReceiver)
      : omReceivers_(1,omReceiver) { }

  /// @brief Ctor from icetray native class
  explicit I3DetectorConfiguration(const I3Geometry&);

  ~I3DetectorConfiguration();

  /// @brief Access an ipdf native OmReceiver via a simple integer key
  const OmReceiver& operator[](int iomr) const {
    return *omReceivers_[iomr];
  }

  /// @brief Access an ipdf native OmReceiver via an icetray OMKey key (without bound checking)
  OmReceiver& operator[](const OMKey& i3om) {
    return *i3omHash_[i3om];
  }
  /// @brief Safely access an ipdf native OmReceiver via an icetray OMKey key, throws IPDF::IllegalInputParameter
  const OmReceiver& find(const OMKey& i3om) const {
    std::map<OMKey,OmReceiver*>::const_iterator elem = i3omHash_.find(i3om);
    AssertX(elem!=i3omHash_.end(),IPDF::IllegalInputParameter);
    return *(*elem).second;
  }

  /// @brief STL compliant iterator
  const_iterator begin() const { return omReceivers_.begin(); }
  /// @brief STL compliant iterator
  const_iterator end() const { return omReceivers_.end(); }
  /// @brief STL compliant size
  unsigned int size() const { return omReceivers_.size(); }
  
  friend std::ostream& operator<<(std::ostream&, const I3DetectorConfiguration&);

private:
  typedef std::vector<OmReceiver*>::iterator iterator;
  std::vector<OmReceiver*> omReceivers_;
  /// Hash for quickly mapping I3 geometry to ipdf geometry
  std::map<OMKey,OmReceiver*> i3omHash_;
};

I3_POINTER_TYPEDEFS(I3DetectorConfiguration);

std::ostream& operator<<(std::ostream& os, 
    const IPDF::I3DetectorConfiguration&);

} // namespace IPDF

#endif // IPDF_I3DetectorConfiguration_H
