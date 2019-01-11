/**
 *
 * copyright  (C) 2015
 * the icecube collaboration
 * @version n$Id: I3Geometry.h 88543 2012-05-22 04:54:48Z claudio.kopper $
 * @file I3VectorDiff.h
 * @date $Date: 2012-05-21 23:54:48 -0500 (Mon, 21 May 2012) $
 */

#ifndef I3VECTORDIFF_H_INCLUDED
#define I3VECTORDIFF_H_INCLUDED

#include <cstddef>
#include <utility>

#include "icetray/I3FrameObject.h"
#include "dataclasses/I3Vector.h"
#include <serialization/dynamic_bitset.hpp>

#include "frame_object_diff/pack_helper.hpp"
#include "frame_object_diff/external/lcs.hpp"

#include <boost/foreach.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/integral_constant.hpp>
#include <boost/type_traits/is_base_of.hpp>

static const unsigned vectordiff_version_ = 0;

/**
 * Store the difference between two vectors.
 *
 * Clever users will note that VectorDiff<std::string>
 * can be used to diff two files.
 */
template <typename T, typename VectorType = std::vector<T> >
class VectorDiff
{
private:
  typedef boost::shared_ptr<VectorType > VectorPtr;
  typedef boost::shared_ptr<const VectorType > VectorConstPtr;

public:
  VectorDiff<T,VectorType>() { }
  
  /**
   * Create a Diff against a base, for the cur object.
   */
  VectorDiff<T,VectorType>(const VectorType& base,
      const VectorType& cur)
      : raw_(false)
  {
    Pack_(base,cur);
  }
  VectorDiff<T,VectorType>(VectorConstPtr base, VectorConstPtr cur)
      : raw_(false)
  { 
    Pack_(*base,*cur);
  }
  
  virtual ~VectorDiff<T,VectorType>() { }
  
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
    
    if (raw_)
      unpacked_ = VectorPtr(new VectorType(diff_));
    else
    {
      unpacked_ = VectorPtr(new VectorType());
      std::size_t base_pos(0), diff_pos(0);
      std::size_t base_len(base.size()), diff_len(diff_.size());
      while (base_pos < base_len)
      {
        while (diff_pos < diff_len && pos_[diff_pos] == base_pos)
        {
          if (mask_[diff_pos])
            unpacked_->push_back(diff_[diff_pos]);
          else
            base_pos++;
          diff_pos++;
        }
        if (base_pos < base_len)
        {
          unpacked_->push_back(base[base_pos]);
          base_pos++;
        }
      }
    }
    return unpacked_;
  }
  
  VectorPtr Unpack(VectorConstPtr base) const
  {
    return Unpack(*base);
  }

private:  
  /**
   * Indicate whether this is a diff (false), or a complete copy of the
   * current object (true). This is done in case the diff is inefficient.
   */
  bool raw_;
  
  /**
   * Store whether the diff is +/- at each index.
   */
  boost::dynamic_bitset<> mask_;
  
  /**
   * Store the insertion/removal point relative to the base.
   */
  std::vector<std::size_t> pos_;
  
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
    VectorType sub;
    lcs::lcs(base,cur,sub);
    std::size_t base_pos(0), cur_pos(0), sub_pos(0);
    std::size_t base_len(base.size()), cur_len(cur.size());
    std::size_t sub_len(sub.size());
    
    while (sub_pos < sub_len)
    {
      while (cur_pos < cur_len && sub[sub_pos] != cur[cur_pos])
      {
        mask_.push_back(true);
        pos_.push_back(base_pos);
        diff_.push_back(cur[cur_pos]);
        cur_pos++;
      }
      cur_pos++;
      while (base_pos < base_len && sub[sub_pos] != base[base_pos])
      {
        mask_.push_back(false);
        pos_.push_back(base_pos);
        diff_.push_back(base[base_pos]);
        base_pos++;
      }
      base_pos++;
      sub_pos++;
    }
    while (cur_pos < cur_len)
    {
      mask_.push_back(true);
      pos_.push_back(base_pos);
      diff_.push_back(cur[cur_pos]);
      cur_pos++;
    }
    while (base_pos < base_len)
    {
      mask_.push_back(false);
      pos_.push_back(base_pos);
      diff_.push_back(base[base_pos]);
      base_pos++;
    }
    
    // if the diff is actually longer than a full copy,
    // just take the current version whole
    if (diff_.size()*(sizeof(size_t)+sizeof(T)) >= cur_len*sizeof(T)) {
      log_trace("raw data is better!");
      raw_ = true;
      mask_.clear();
      pos_.clear();
      diff_.assign(cur.begin(),cur.end());
    }
    
    log_trace_stream("\nbase(" << base.size() << ")\ncur(" << cur.size() << ")\ndiff(" << diff_.size() << ")");
  }
  
  
protected:
  friend class icecube::serialization::access;
  template <class Archive> void serialize(Archive & ar, unsigned version)
  {
    ar & make_nvp("raw",raw_);
    ar & make_nvp("diff",diff_);
    if (!raw_)
    {
      ar & make_nvp("mask",mask_);
      ar & make_nvp("pos",pos_);
    }
  }
};

/**
 * Store the difference between two I3Vectors.
 *
 * This inherits from VectorDiff.
 */
template <typename T>
class I3VectorDiff : public I3FrameObject,
    public VectorDiff<T,I3Vector<T> >
{
private:
  typedef boost::shared_ptr<I3Vector<T> > I3VectorPtr;
  typedef boost::shared_ptr<const I3Vector<T> > I3VectorConstPtr;
public:
  I3VectorDiff<T>()
      : VectorDiff<T,I3Vector<T> >()
  { }
  
  /**
   * Create a Diff against a base, for the cur object.
   * Store the filename that the base came from.
   */
  I3VectorDiff<T>(const std::string filename,
      const I3Vector<T>& base, const I3Vector<T>& cur)
      : VectorDiff<T,I3Vector<T> >(base,cur),
        base_filename_(filename)
  { }
  I3VectorDiff<T>(const std::string filename,
      I3VectorConstPtr base, I3VectorConstPtr cur)
      : VectorDiff<T,I3Vector<T> >(base,cur),
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
  I3VectorPtr Unpack(const I3Vector<T>& base) const
  {
    return VectorDiff<T,I3Vector<T> >::Unpack(base);
  }
  I3VectorPtr Unpack(I3VectorConstPtr base) const
  {
    return VectorDiff<T,I3Vector<T> >::Unpack(base);
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
    VectorDiff<T,I3Vector<T> >::serialize(ar,version);
  }
};

typedef I3VectorDiff<OMKey> I3VectorOMKeyDiff;
typedef I3VectorDiff<TankKey> I3VectorTankKeyDiff;
typedef I3VectorDiff<int> I3VectorIntDiff;

I3_POINTER_TYPEDEFS(I3VectorOMKeyDiff);
I3_POINTER_TYPEDEFS(I3VectorTankKeyDiff);
I3_POINTER_TYPEDEFS(I3VectorIntDiff);

#endif // I3VECTORDIFF_H_INCLUDED
