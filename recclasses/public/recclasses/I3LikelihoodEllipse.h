/**
 * copyright  (C) 2004
 * the icecube collaboration
 * @version $Id: I3LikelihoodEllipse.h 158928 2017-10-19 23:26:27Z cweaver $
 * @file I3LikelihoodEllipse.h
 * @date $Date: 2017-10-19 18:26:27 -0500 (Thu, 19 Oct 2017) $
 */

#ifndef I3LIKELIHOODELLIPSE_H_INCLUDED
#define I3LIKELIHOODELLIPSE_H_INCLUDED

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <set>
#include <vector>
#include <icetray/I3FrameObject.h>
#include "dataclasses/I3Vector.h"
#include "dataclasses/I3Matrix.h"

/**
 * @brief Wrapper for the ellipsoids so that we can store the list of ellipses                                                                                                   
 *   generated for each fit. 
 * 
 * This class holds information about ellipsoids that are created by the various
 * likelihood fits. It is initially to be used for outputting the ellipses generated
 * during a MultiNest/nestle minimization in low energy events, but can be expanded
 * down the road to also be used for other purposes if needed.
 */
static const unsigned i3likelihoodellipse_version_ = 0;

class I3LikelihoodEllipse : public I3FrameObject
{

 public:

  I3LikelihoodEllipse() : 
    center_(NAN), 
    inverse_covariance_(NAN), 
    nllh_(NAN), 
    axis_names_(NAN) { }
    
  I3LikelihoodEllipse(std::vector<double> center, 
		      std::vector<std::vector<double> > inverse_cov, 
		      double nllh, 
		      std::vector<std::string> axis_names) : 
    center_(center), 
    inverse_covariance_(inverse_cov), 
    nllh_(nllh), 
    axis_names_(axis_names) { }
    
  I3LikelihoodEllipse(const I3LikelihoodEllipse& ellipse);
  
  I3LikelihoodEllipse(std::vector<double> center, 
		      I3Matrix inverse_cov, 
		      double nllh, 
		      std::vector<std::string> axis_names); 
    
  ~I3LikelihoodEllipse();
  
  std::ostream& Print(std::ostream&) const;
  std::string repr() const;

  std::vector<double> GetCenter() const { return center_;}
  void SetCenter(std::vector<double> center){center_ = center;}

  std::vector<std::vector<double> > GetInverseCovariance() const { return inverse_covariance_;}
  void SetInverseCovariance(std::vector<std::vector<double> > inverse_covariance){inverse_covariance_ = inverse_covariance;}

  double GetNLLH() const { return nllh_; }
  void SetNLLH(double nllh) { nllh_ = nllh; }

  std::vector<std::string> GetAxisNames() const { return axis_names_; }
  void SetAxisNames(std::vector<std::string> axis_names) { axis_names_ = axis_names; }

  bool Contains(const std::vector<double>& point);
  
  bool operator==(const I3LikelihoodEllipse& rhs) const {
    return (center_ == rhs.GetCenter()
	    && inverse_covariance_ == rhs.GetInverseCovariance()
	    && axis_names_ == rhs.GetAxisNames()
	    && nllh_ == rhs.GetNLLH());
  }

  bool operator!=(const I3LikelihoodEllipse& rhs) const {
    return !(*this == rhs);
  }

  static void ConvertToUnitVectors(const std::vector<double>& azimuth,
				   const std::vector<double>& zenith,
				   std::vector<double>& nx,
				   std::vector<double>& ny,
				   std::vector<double>& nz);

  static double Dot(std::vector<double>& vec1, std::vector<double>& vec2);
  static std::vector<double> Dot(std::vector<double>& vec, std::vector<std::vector<double> >& matrix);

  I3LikelihoodEllipse Profile(uint dimension);
  I3LikelihoodEllipse Profile(std::vector<uint> dimensions);

 private:
  std::vector<double> center_;
  std::vector<std::vector<double> > inverse_covariance_;
  double  nllh_;
  std::vector<std::string>  axis_names_;

  friend class icecube::serialization::access;

  template <class Archive> void serialize(Archive & ar, const unsigned version);

};


/**
 * @brief Vector container for the generated ellipses to simplify calculations
 * 
 * This class holds multiple ellipsoids and gives some convenience functions to
 * make life a bit simpler.
 */


class I3LikelihoodEllipseCollection : public I3FrameObject
{
 public:
 I3LikelihoodEllipseCollection() : ellipses_(NAN){ }

  I3LikelihoodEllipseCollection(std::vector<I3LikelihoodEllipse> ellipses);
  
  ~I3LikelihoodEllipseCollection();

  std::ostream& Print(std::ostream&) const;
  std::string repr() const;

  void Clear();
  
  std::vector<I3LikelihoodEllipse> GetEllipses() const;
  void SetEllipses(std::vector<I3LikelihoodEllipse> ellipses);

  uint GetSize();

  bool In(I3LikelihoodEllipse ellipse);
  
  std::vector<I3LikelihoodEllipse>::const_iterator begin() const;
  std::vector<I3LikelihoodEllipse>::const_iterator end() const;

  void AddEllipse(I3LikelihoodEllipse ellipse);
  void push_back(I3LikelihoodEllipse ellipse);

  I3LikelihoodEllipse GetEllipse(uint i) const;
  I3LikelihoodEllipse operator[](uint i) const;
  void SetEllipse(uint i, I3LikelihoodEllipse ellipse);

  std::vector<std::string> GetAxisNames() const { return axis_names_; }

  void RemoveEllipse(uint i);
  void erase(uint i);

  I3LikelihoodEllipseCollection Profile(uint dimension);
  I3LikelihoodEllipseCollection Profile(std::vector<uint> dimensions);
  bool Contains(const std::vector<double>& point);

  I3LikelihoodEllipseCollection Prune(double maxDeltaNLLH=15);

  void Extend(std::vector<I3LikelihoodEllipse> ellipses);
  
  bool operator!=(const I3LikelihoodEllipseCollection& rhs) const;
  bool operator==(const I3LikelihoodEllipseCollection& rhs) const;
  
  template<class Archive> void serialize(Archive& archive, unsigned version);

 private:
  std::vector<I3LikelihoodEllipse> ellipses_;
  friend class icecube::serialization::access;
  std::vector<std::string> axis_names_;

};



I3_CLASS_VERSION(I3LikelihoodEllipse,i3likelihoodellipse_version_);
I3_CLASS_VERSION(I3LikelihoodEllipseCollection,i3likelihoodellipse_version_);

std::ostream& operator<<(std::ostream& oss, const I3LikelihoodEllipse& h);
std::ostream& operator<<(std::ostream& oss, const I3LikelihoodEllipseCollection& h);


I3_POINTER_TYPEDEFS(I3LikelihoodEllipse);
I3_POINTER_TYPEDEFS(I3LikelihoodEllipseCollection);

#endif //I3LIKELIHOODELLIPSE_H_INCLUDED





