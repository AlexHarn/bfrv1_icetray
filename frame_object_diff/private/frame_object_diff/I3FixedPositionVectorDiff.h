/**
 *
 * copyright  (C) 2015
 * the icecube collaboration
 * @version n$Id: I3Geometry.h 88543 2012-05-22 04:54:48Z claudio.kopper $
 * @file I3FixedPositionVectorDiff.h
 * @date $Date: 2012-05-21 23:54:48 -0500 (Mon, 21 May 2012) $
 */

#ifndef I3FIXEDPOSITIONVECTORDIFF_H_INCLUDED
#define I3FIXEDPOSITIONVECTORDIFF_H_INCLUDED

#include <cstddef>
#include <utility>

#include "icetray/I3FrameObject.h"
#include "dataclasses/I3Vector.h"

#include "frame_object_diff/pack_helper.hpp"

#include <boost/foreach.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/integral_constant.hpp>
#include <boost/type_traits/is_base_of.hpp>

static const unsigned fixedpositionvectordiff_version_ = 0;

/**
 * Store the difference between two vectors where each object
 * has a fixed position in the vector. This allows us to
 * directly Diff two objects at the same position, like
 * in a map with integer keys.
 */
template <typename T, typename Orig=T, typename VectorType = std::vector<Orig> >
class FixedPositionVectorDiff
{
private:
  typedef boost::shared_ptr<VectorType > VectorPtr;
  typedef boost::shared_ptr<const VectorType > VectorConstPtr;

public:
  FixedPositionVectorDiff<T,Orig,VectorType>() { }
  
  /**
   * Create a Diff against a base, for the cur object.
   */
  FixedPositionVectorDiff<T,Orig,VectorType>(const VectorType& base,
      const VectorType& cur)
  {
    Pack_(base,cur);
  }
  FixedPositionVectorDiff<T,Orig,VectorType>(VectorConstPtr base, VectorConstPtr cur)
  { 
    Pack_(*base,*cur);
  }
  
  virtual ~FixedPositionVectorDiff<T,Orig,VectorType>() { }
  
  /**
   * Return the size of the Diff
   */
  std::size_t size() const
  {
    return diff_.size();
  }
  
  /**
   * Unpack the Diff, returning a shared pointer to
   * the original object.
   *
   * Takes the base that was originally provided to the
   * constructor as input.
   */
  VectorPtr Unpack(const VectorType& base) const
  {
    if (unpacked_)
      return unpacked_;
    
    i3_assert(diff_.size() == base.size());
    unpacked_ = VectorPtr(new VectorType());
    const std::size_t b_size = base.size();
    for(std::size_t i=0;i<b_size;i++)
      unpacked_->push_back(pack_helper::Unpack(diff_[i],base[i]));
    return unpacked_;
  }
  
  VectorPtr Unpack(VectorConstPtr base) const
  {
    return Unpack(*base);
  }

private:  
  /**
   * Store the actual diff content
   */
  std::vector<T> diff_;
  
  /**
   * A shared pointer to the unpacked data, so we don't
   * have to regenerate this for subsequent calls.
   */
  mutable VectorPtr unpacked_;

  /**
   * Pack up the data. This is a shared function for
   * the constructors.
   */
  void Pack_(const VectorType& base, const VectorType& cur)
  {
    // first, check that the vectors are the same size
    i3_assert(base.size() == cur.size());
    const T tmp;
    const std::size_t b_size = base.size();
    for(std::size_t i=0;i<b_size;i++)
      diff_.push_back(pack_helper::Pack(cur[i],base[i],tmp));
  }
  
  
protected:
  friend class icecube::serialization::access;
  template <class Archive> void serialize(Archive & ar, unsigned version)
  {
    ar & make_nvp("diff",diff_);
  }
};

/**
 * Store the difference between two fixed position I3Vectors.
 *
 * This inherits from FixedPositionVectorDiff.
 */
template <typename T, typename Orig=T>
class I3FixedPositionVectorDiff : public I3FrameObject,
    public FixedPositionVectorDiff<T,Orig,I3Vector<Orig> >
{
private:
  typedef boost::shared_ptr<I3Vector<Orig> > I3VectorPtr;
  typedef boost::shared_ptr<const I3Vector<Orig> > I3VectorConstPtr;
public:
  I3FixedPositionVectorDiff<T,Orig>()
      : FixedPositionVectorDiff<T,Orig,I3Vector<Orig> >()
  { }
  
  /**
   * Create a Diff against a base, for the cur object.
   * Store the filename that the base came from.
   */
  I3FixedPositionVectorDiff<T,Orig>(const std::string filename,
      const I3Vector<Orig>& base, const I3Vector<Orig>& cur)
      : FixedPositionVectorDiff<T,Orig,I3Vector<Orig> >(base,cur),
        base_filename_(filename)
  { }
  I3FixedPositionVectorDiff<T,Orig>(const std::string filename,
      I3VectorConstPtr base, I3VectorConstPtr cur)
      : FixedPositionVectorDiff<T,Orig,I3Vector<Orig> >(base,cur),
        base_filename_(filename)
  { }

  /**
   * Unpack the Diff, returning a shared pointer to
   * the original object.
   *
   * Takes the base that was originally provided to the
   * constructor as input.
   *
   * Note that this is required for SFINAE detection of
   * Diff objects, and cannot just be inherited.
   */
  I3VectorPtr Unpack(const I3Vector<Orig>& base) const
  {
    return FixedPositionVectorDiff<T,Orig,I3Vector<Orig> >::Unpack(base);
  }
  I3VectorPtr Unpack(I3VectorConstPtr base) const
  {
    return FixedPositionVectorDiff<T,Orig,I3Vector<Orig> >::Unpack(base);
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
    FixedPositionVectorDiff<T,Orig,I3Vector<Orig> >::serialize(ar,version);
  }
};

#endif // I3FIXEDPOSITIONVECTORDIFF_H_INCLUDED
