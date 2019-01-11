/**
    copyright  (C) 2007
    the icecube collaboration

    @author juancarlos@icecube.wisc.edu

    @brief a class to compute and store a binned sky map. Each bin is assigned
    an integer index which can be stored in an event dst.
*/

#ifndef DST_DSTCOORDINATES08_H_INCLUDED
#define DST_DSTCOORDINATES08_H_INCLUDED

#include <icetray/I3Units.h>
#include <recclasses/I3DST.h>
#include <phys-services/I3RandomService.h>
#include <map>

typedef  pair<uint16_t,uint16_t>  uintpair;
typedef  pair<float,float>  floatpair;

using namespace std;

class DSTCoordinate {

	  float dtheta0_;
	  float dphi0_;
	  float domega0_;

	public:

	  uint16_t presc_;

	  map< uintpair , int16_t> key2index_;
	  map<int16_t, uintpair > index2key_;

	  /**
       * Constructor
       * @param dtheta size (in degrees) in theta of each band in the skymap
       * @param dphi initial size (in degrees) in phi of bins in skymap at the horizon
       * @param digits number of digits to save in order to represent
       * coordinates the center of the bin
       */
	  DSTCoordinate(float dtheta,float dphi, uint8_t digits);
	  DSTCoordinate() {};
	  ~DSTCoordinate() {};

	  /**
       * Generate map of sky 
       */
	  void ComputeBins();

	  /**
       * Compute size of phi at given theta angle
       * @param theta zenith angle for which we are computing dphi
       * @return the size in phi of a bin at a particular zenith angle
       */
	  float compute_dphi(float theta);

	  /**
       * Create an integer pair corresponding to the center of the bin that
       * encloses the coordinates theta,phi
       *
       * @param theta 
       * @param phi
       * @return the integer pair corresponding to the center of the bin
       */
	  uintpair mkpair(float theta, float phi);

	  /**
       * Given an integer index, return the values of theta and phi at the
       * center of the bin 
       *
       * @param index
       * @return float pair corresponding to the center of the bin
       */
	  floatpair GetCoords(int16_t index);

	  /**
       * Given a coordinate (theta,phi), return the index of the bin that
       * contains it
       *
       * @param theta
       * @param phi
       * @return integer index corresponding to bin
       */
	  int16_t GetIndex(float theta, float phi);

	  /**
       * @return the size of the skymap
       */
	  uint8_t NumberOfBins() { return index2key_.size(); }
};


#endif
