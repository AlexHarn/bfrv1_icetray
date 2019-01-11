/*
 * copyright  (C) 2011
 * The Icecube Collaboration
 *
 * $Id$
 *
 * @version $Revision$
 * @date $LastChangedDate$
 * @author Fabian Kislat <fabian.kislat@desy.de> Last changed by: $LastChangedBy$
 */

#ifndef TPX_UTILITY_H_INCLUDED
#define TPX_UTILITY_H_INCLUDED

#include <icetray/name_of.h>
#include <icetray/OMKey.h>
#include <icetray/I3Logging.h>
#include <dataclasses/I3Vector.h>
#include <boost/shared_ptr.hpp>
#include <set>


class I3DOMCalibration;
class I3DOMStatus;
class I3Waveform;


namespace tpx {

#ifdef __GNUC__
  __attribute__((unused))
#endif
  SET_LOGGER("tpx");


  namespace detail_ {
    
    template <typename T>
    void warn_missing(const OMKey &omKey)
    {
      static std::set<OMKey> keys;
      if (keys.find(omKey) == keys.end()) {
	log_warn("Missing %s for DOM %s", icetray::name_of<T>().c_str(), omKey.str().c_str());
	keys.insert(omKey);
      }
    }
    
  }


  template <typename T>
  const T* get_dom_info(OMKey omKey, const std::map<OMKey, T> &m)
  {
    typedef typename std::map<OMKey, T>::const_iterator m_iterator;

    m_iterator iter = m.find(omKey);
    if (iter == m.end()) {
      detail_::warn_missing<T>(omKey);
      return NULL;
    }

    return &(iter->second);
  }

  
  double GetSPEPeakCharge(const I3DOMStatus *domStatus, const I3DOMCalibration *domCalib);

  int8_t GetChannel(const I3Waveform &wf);

} // namespace tpx

#endif // TPX_UTILITY_H_INCLUDED
