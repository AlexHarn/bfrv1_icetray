/**
 *
 * copyright  (C) 2015
 * the icecube collaboration
 * @version n$Id: I3Geometry.h 88543 2012-05-22 04:54:48Z claudio.kopper $
 * @file I3MapDiff.h
 * @date $Date: 2012-05-21 23:54:48 -0500 (Mon, 21 May 2012) $
 */

#ifndef I3MAPDIFF_H_INCLUDED
#define I3MAPDIFF_H_INCLUDED

#include <cstddef>

#include "icetray/I3FrameObject.h"
#include "dataclasses/I3String.h"
#include "dataclasses/I3Map.h"
#include "dataclasses/I3Vector.h"
#include "frame_object_diff/pack_helper.hpp"

#include <boost/foreach.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_base_of.hpp>

static const unsigned mapdiff_version_ = 0;

/**
 * Store the difference between two maps
 */
template <typename K, typename V, typename Orig = V,
          typename MapType = std::map<K,Orig> >
class MapDiff
{
private:
  typedef boost::shared_ptr<Orig> OrigPtr;
  typedef boost::shared_ptr<MapType > MapPtr;
  typedef boost::shared_ptr<const MapType > MapConstPtr;
  typedef typename MapType::value_type orig_value_type;
  typedef typename std::map<K,V>::value_type new_value_type;
  typedef typename MapType::iterator map_iterator;
  typedef typename MapType::const_iterator map_const_iterator;
  
public:
  typedef typename std::map<K,V>::iterator plus_iterator;
  typedef typename std::map<K,V>::value_type plus_value;

  MapDiff<K,V,Orig,MapType>() { }
  
  /**
   * Create a Diff against a base, for the cur object.
   */
  MapDiff<K,V,Orig,MapType>(const MapType& base, const MapType& cur)
  {
    Pack_(base,cur);
  }
  MapDiff<K,V,Orig,MapType>(MapConstPtr base, MapConstPtr cur)
  { 
    Pack_(*base,*cur);
  }
  
  virtual ~MapDiff<K,V,Orig,MapType>() { }
  
  /**
   * Return the sum of the additions and subtractions as the size.
   */
  std::size_t size() const
  {
    return plus_.size()+minus_.size();
  }
  
  /**
   * Unpack the Diff, returning a shared pointer to
   * the original object.
   *
   * Takes the base that was originially provided to the
   * constructor as input.
   */
  MapPtr Unpack(const MapType& base) const
  {
    if (unpacked_)
      return unpacked_;
    
    unpacked_ = MapPtr(new MapType(base));
    BOOST_FOREACH(const K& k, minus_)
      unpacked_->erase(k);
    BOOST_FOREACH(const new_value_type& v, plus_)
    {
      Orig& unpacked_val = (*unpacked_)[v.first];
      unpacked_val = pack_helper::Unpack(v.second,unpacked_val);
    }
    return unpacked_;
  }
  
  MapPtr Unpack(MapConstPtr base) const
  {
    return Unpack(*base);
  }
  
  /**
   * Return an iterator over the addition map.
   */
  plus_iterator begin_plus()
  { return plus_.begin(); }

  /**
   * Get the end of the addition map.
   */
  plus_iterator end_plus()
  { return plus_.end(); }

private:  
  /**
   * A map of all additions to the base.
   */
  std::map<K,V> plus_;
  
  /**
   * A vector of all keys to be removed from the base.
   */
  std::vector<K> minus_;
  
  /**
   * A shared pointer to the unpacked data, so we don't 
   * have to regenerate this for subsequent calls. 
   */
  mutable MapPtr unpacked_;
  
  /**
   * Pack up the data. This is a shared function for
   * the constructors.
   */
  void Pack_(const MapType& base, const MapType& cur)
  {
    BOOST_FOREACH(const orig_value_type& v, base)
    {
      map_const_iterator i = cur.find(v.first);
      if (i == cur.end())
        minus_.push_back(v.first);
      else if (v.second != i->second) {
        V& plus_val = plus_[v.first];
        plus_val = pack_helper::Pack(i->second,v.second,plus_val);
      }
    }
    const Orig empty;
    BOOST_FOREACH(const orig_value_type& v, cur)
    {
      if (base.find(v.first) == base.end()) {
        V& plus_val = plus_[v.first];
        plus_val = pack_helper::Pack(v.second,empty,plus_val);
      }
    }
  }
  
protected:
  friend class icecube::serialization::access;
  template <class Archive> void serialize(Archive & ar, unsigned version)
  {
    ar & make_nvp("plus",plus_);
    ar & make_nvp("minus",minus_);
  }
};

/**
 * Store the difference between two I3Maps
 */
template <typename K, typename V, typename Orig = V>
class I3MapDiff : public I3FrameObject,
    public MapDiff<K,V,Orig,I3Map<K,Orig> >
{
private:
  typedef boost::shared_ptr<I3Map<K,Orig> > I3MapPtr;
  typedef boost::shared_ptr<const I3Map<K,Orig> > I3MapConstPtr;
public:
  I3MapDiff<K,V,Orig>()
      : MapDiff<K,V,Orig,I3Map<K,Orig> >()
  { }
  
  /**
   * Create a diff against a base, for the cur object.
   * Store the filename that the base came from.
   */
  I3MapDiff<K,V,Orig>(const std::string filename,
      const I3Map<K,Orig>& base, const I3Map<K,Orig>& cur)
      : MapDiff<K,V,Orig,I3Map<K,Orig> >(base, cur),
        base_filename_(filename)
  { }
  I3MapDiff<K,V,Orig>(const std::string filename,
      I3MapConstPtr base, I3MapConstPtr cur)
      : MapDiff<K,V,Orig,I3Map<K,Orig> >(base, cur),
        base_filename_(filename)
  { }
  
  /**
   * Unpack the Diff, returning a shared pointer to
   * the original opbject.
   *
   * Takes the base that was originially provided to the
   * constructor as input.
   *
   * Note that this is required for SFINAE detection of
   * Diff objects, and cannot just be inherited.
   */
  I3MapPtr Unpack(const I3Map<K,Orig>& base) const
  {
    return MapDiff<K,V,Orig,I3Map<K,Orig> >::Unpack(base);
  }
  I3MapPtr Unpack(I3MapConstPtr base) const
  {
    return MapDiff<K,V,Orig,I3Map<K,Orig> >::Unpack(base);
  }

  /**
   * Get the filename for the base.
   */
  std::string GetFilename() const
  { return base_filename_; }
  
private:
  /**
   * Filename where the base object came from.
   */
  std::string base_filename_;

  friend class icecube::serialization::access;
  template <class Archive> void serialize(Archive & ar, unsigned version)
  {
    ar & make_nvp("I3FrameObject", base_object<I3FrameObject>(*this));
    ar & make_nvp("filename", base_filename_);
    MapDiff<K,V,Orig,I3Map<K,Orig> >::serialize(ar,version);
  }
};

#endif // I3MAPDIFF_H_INCLUDED
