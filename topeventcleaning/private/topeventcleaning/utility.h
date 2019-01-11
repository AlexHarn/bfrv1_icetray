/*
 * copyright  (C) 2011
 * The Icecube Collaboration
 *
 * $Id: utility.h 78669 2011-08-08 17:10:31Z kislat $
 *
 * @version $Revision: 78669 $
 * @date $LastChangedDate: 2011-08-08 19:10:31 +0200 (Mo, 08 Aug 2011) $
 * @author Fabian Kislat <fabian.kislat@desy.de> Last changed by: $LastChangedBy: kislat $
 */

#ifndef TOPEVENTCLEANING_UTILITY_H_INCLUDED
#define TOPEVENTCLEANING_UTILITY_H_INCLUDED

#include <icetray/name_of.h>
#include <icetray/OMKey.h>
#include <dataclasses/TankKey.h>
#include <dataclasses/StationKey.h>
#include <icetray/I3Logging.h>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <map>
#include <set>


class I3DOMCalibration;
class I3DOMStatus;


namespace topeventcleaning {

#ifdef __GNUC__
  __attribute__((unused))
#endif
  SET_LOGGER("topeventcleaning");


  namespace detail_ {

    template <typename K> inline std::string make_label(const K &key);

    template <>
    inline std::string make_label<StationKey>(const StationKey &key) {
      return "station " + boost::lexical_cast<std::string>(key);
    }

    template <>
    inline std::string make_label<TankKey>(const TankKey &key) {
      return "tank " + boost::lexical_cast<std::string>(key);
    }

    template <>
    inline std::string make_label<OMKey>(const OMKey &key) {
      return "DOM " + boost::lexical_cast<std::string>(key);
    }
    
    template <typename T, typename K>
    void warn_missing(const K &key)
    {
      static std::set<K> keys;
      if (keys.find(key) == keys.end()) {
	log_warn("Missing %s for %s", icetray::name_of<T>().c_str(), make_label(key).c_str());
	keys.insert(key);
      }
    }
    
  }


  template <typename T, typename K>
  const T* get_dom_info(K key, const std::map<K, T> &m)
  {
    typedef typename std::map<K, T>::const_iterator m_iterator;

    m_iterator iter = m.find(key);
    if (iter == m.end()) {
      detail_::warn_missing<T, K>(key);
      return NULL;
    }

    return &(iter->second);
  }

} // namespace topeventcleaning

#endif // TOPEVENTCLEANING_UTILITY_H_INCLUDED
