/**
 *
 * copyright  (C) 2015
 * the icecube collaboration
 * @version n$Id: pack_helper.hpp 88543 2012-05-22 04:54:48Z claudio.kopper $
 * @file pack_helper.hpp
 * @date $Date: 2012-05-21 23:54:48 -0500 (Mon, 21 May 2012) $
 */

#ifndef PACK_HELPER_HPP_INCLUDED
#define PACK_HELPER_HPP_INCLUDED

#include <boost/foreach.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/type_traits/is_same.hpp>

#include "icetray/I3FrameObject.h"

//! Helper functions for packing and unpacking compressed objects
namespace pack_helper {
  
  /**
   * Check if a class is a *Diff class
   */ 
  template <typename T, typename Orig>
  struct has_diff
  {
    typedef char yes[1];
    typedef char no[2];
    typedef boost::shared_ptr<Orig> OrigPtr;
    
    template <typename U, U> struct type_check;
    template <typename _1> static yes &chk(type_check<OrigPtr(T::*)(const Orig&) const, &_1::Unpack> *);
    template <typename   > static no  &chk(...);
    
    static bool const value = (sizeof(chk<T>(0)) == sizeof(yes));
  };

  /**
   * Check if a class is a *Diff class and not an I3FrameObject
   */ 
  template <typename T, typename Orig>
  struct has_diff_no_frameobj
  {
    typedef char yes[1];
    typedef char no[2];
    typedef boost::shared_ptr<Orig> OrigPtr;
    
    template <typename U, U> struct type_check;
    template <typename _1> static yes &chk(type_check<OrigPtr(T::*)(const Orig&) const, &_1::Unpack> *);
    template <typename   > static no  &chk(...);
    
    static bool const value = (sizeof(chk<T>(0)) == sizeof(yes) &&
        ! boost::is_base_of<I3FrameObject,T>::value);
  };

  /**
   * Check if a class is a *Diff class and an I3FrameObject
   */ 
  template <typename T, typename Orig>
  struct has_diff_and_frameobj
  {
    typedef char yes[1];
    typedef char no[2];
    typedef boost::shared_ptr<Orig> OrigPtr;
    
    template <typename U, U> struct type_check;
    template <typename _1> static yes &chk(type_check<OrigPtr(T::*)(const Orig&) const, &_1::Unpack> *);
    template <typename   > static no  &chk(...);
    
    static bool const value = (sizeof(chk<T>(0)) == sizeof(yes) &&
        boost::is_base_of<I3FrameObject,T>::value);
  };
  
  /**
   * Unpack the data from compressed format, if necessary.
   */ 
  template <typename T, typename B>
  typename boost::enable_if_c<has_diff<T,B>::value, const B>::type
  Unpack(const T& data, const B& base)
  {
    return *(data.Unpack(base));
  }
  template <typename T, typename B>
  typename boost::disable_if_c<has_diff<T,B>::value, const B>::type
  Unpack(const T& data, const B& base)
  {
    return data;
  }

  /**
   * Pack the data into compressed format, if necessary.
   */ 
  template <typename T,typename B>
  typename boost::enable_if_c<has_diff_and_frameobj<T,B>::value, T>::type
  Pack(const B& data, const B& base, const T& dummy)
  {
    return T("", base, data);
  }
  template <typename T,typename B>
  typename boost::enable_if_c<has_diff_no_frameobj<T,B>::value, T>::type
  Pack(const B& data, const B& base, const T& dummy)
  {
    return T(base, data);
  }
  template <typename T,typename B>
  typename boost::disable_if_c<has_diff<T,B>::value, T>::type
  Pack(const B& data, const B& base, const T& dummy)
  {
    return T(data);
  }

}

#endif // PACK_HELPER_HPP_INCLUDED 
