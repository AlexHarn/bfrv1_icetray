/**
 $ $Id$
 * @version $Revision$
 * @date $Date$
 * @author Sebastian Euler <sebastian.euler@icecube.wisc.edu>
 */

#ifndef RECCLASSES_I3FINITECUTS_H_INCLUDED
#define RECCLASSES_I3FINITECUTS_H_INCLUDED

#include <icetray/I3FrameObject.h>
#include <icetray/I3Logging.h>

#include <serialization/access.hpp>

#include <cmath>
#include <ostream>

/**
 *  @brief Stores results of function I3FiniteCalc (see project finiteReco).
 */
class I3FiniteCuts : public I3FrameObject {
  
 public:
  /**
     The estimated length of the track
   */
  double Length;
  /**
     Distance between the stop point of the track and the border of the detector. As border of the detector the last Cherenkov emission point on the track is used. Light emitted up to this point can reach a DOM without scattering.  
   */
  double Lend;
  /**
     Similar to Lend. Distance between the interaction vertex and the border of the detector. From the border on Cherenkov light can reach a DOM without scattering.
   */
  double Lstart;
  /**
     This parameter is similar to a smoothness. It depends on the distribution of the Cherenkov light emission points for the given hits along the track.
   */
  double Sdet;
  /**
     Sum of the signed distances between the middle of an assumed infinite track and the Cherenkov emission points on the track corresponding to the given hits. The sum is normalized by the number of hits.
   */
  double finiteCut;  
  /**
     The estimated length of the detector the event has passed
   */
  double DetectorLength;
  
  I3FiniteCuts(double l  = NAN,
               double le = NAN,
               double ls = NAN,
               double S  = NAN,
               double f  = NAN,
               double dl = NAN) :
    I3FrameObject(),
    Length(l),
    Lend(le),
    Lstart(ls),
    Sdet(S),
    finiteCut(f),
    DetectorLength(dl)
  {
    log_debug( "hello" );
  }

  virtual ~I3FiniteCuts();
  virtual void Reset();
  
  bool operator==(const I3FiniteCuts&) const;
  
  std::ostream& Print(std::ostream&) const override;

 private:
  friend class icecube::serialization::access;

  template <class Archive>
  void serialize(Archive& ar, unsigned version);

  SET_LOGGER( "I3FiniteCuts" );
};

std::ostream& operator<<(std::ostream&, const I3FiniteCuts&);

I3_POINTER_TYPEDEFS(I3FiniteCuts);

#endif 
