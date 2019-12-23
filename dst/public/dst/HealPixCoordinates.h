/**
    copyright  (C) 2007
    the icecube collaboration

    @author juancarlos@icecube.wisc.edu

    @brief a class to compute and store a binned sky map. Each bin is assigned
    an integer index which can be stored in an event dst.
*/

#ifndef DST_HEALPIXCOORDINATES_09_H_INCLUDED
#define DST_HEALPIXCOORDINATES_09_H_INCLUDED

#include <icetray/I3Units.h>
#include <recclasses/I3DST.h>
#include <phys-services/I3RandomService.h>
#include <map>

typedef  pair<uint16_t,uint16_t>  uintpair;
typedef  pair<float,float>  floatpair;

using namespace std;

class HealPixCoordinate {

	  void*  skyMap_;
	  double pix_size_;
	  uint16_t nside_;
	  I3RandomServicePtr rand_;

	public:

	  /**
       * Constructor
       * @param dphi initial size (in degrees) in phi of bins in skymap at the horizon
       * @param digits number of digits to save in order to represent
       * coordinates the center of the bin
       */

	  HealPixCoordinate();
	  ~HealPixCoordinate() {};

	  /**
       * Generate map of sky 
       */
	  void ComputeBins(int);

	  //inline void ComputeBins() { ComputeBins(128); }

	  /**
       * @return float size of bin
       */
	  inline double GetPixSize() { return pix_size_; }

	  /**
       * @return nside parameter
       */
	  inline uint16_t GetNSide() { return nside_; }

	  /**
       * Given an integer index, return the values of theta and phi at the
       * center of the bin 
       *
       * @param index
       * @return float pair corresponding to the center of the bin
       */
	  floatpair GetCoords(uint32_t index);

	  /**
       * Given a coordinate (theta,phi), return the index of the bin that
       * contains it
       *
       * @param theta
       * @param phi
       * @return integer index corresponding to bin
       */
	  uint32_t GetIndex(double theta, double phi);

	  /**
       * @return the size of the skymap
       */
	  uint32_t NumberOfBins();

	  inline void SetRNG(I3RandomServicePtr rand) { rand_ = rand; }

};

I3_POINTER_TYPEDEFS(HealPixCoordinate);

#endif
