/**
    copyright  (C) 2004
    the icecube collaboration
    $Id: dst.cxx 16031 2006-02-20 12:38:45Z troy $

    @version $Revision: 1.2 $
    @date $Date: 2006-02-20 06:38:45 -0600 (lun, 20 feb 2006) $
    @author juancarlos@icecube.wisc.edu
*/

#include "recclasses/I3DST.h"
#include "dst/HealPixCoordinates.h"

#include <iostream>
#include <cmath>

#ifndef HEALPIX_CXX_ON_FEDORA
#include <healpix_cxx/pointing.h>
#include <healpix_cxx/healpix_map.h>
#include <healpix_cxx/lsconstants.h>
#else
#include <healpix/pointing.h>
#include <healpix/healpix_map.h>
#include <healpix/lsconstants.h>
#endif

#include <dataclasses/I3Constants.h>
#include <icetray/I3Units.h>

using namespace std;

typedef  Healpix_Map<double> HealpixMapDbl;
typedef  Healpix_Map<float> HealpixMapFlt;


double angdist(const vec3 &v1, const vec3 &v2);

HealPixCoordinate::HealPixCoordinate()
{
	// Create a map with 3 deg angular resolution (see K. Gorski et al., Ap J 2005).
	skyMap_ = new HealpixMapDbl();

	rand_ = I3RandomServicePtr();
}



void HealPixCoordinate::ComputeBins(int nside)
{
	nside_ = int16_t(nside);
	HealpixMapDbl& skyMap = *( (HealpixMapDbl*) skyMap_);
	skyMap.SetNside(nside, RING);
	skyMap.fill(0.);

    fix_arr<int, 8u> neighbors;
	pointing dir;
    unsigned pix_id;

    dir.theta = 45.0*I3Units::degree;
    dir.phi   = 45.0*I3Units::degree;
    pix_id    = skyMap.ang2pix(dir);
    skyMap.neighbors(pix_id,neighbors);


	vec3 v1   = dir.to_vec3();
	vec3 v2   = skyMap.pix2ang(neighbors[0]).to_vec3();
	pix_size_ = angdist(v1, v2);
	log_info("dst pixel size: %g deg", pix_size_*rad2degr);
}

uint32_t HealPixCoordinate::GetIndex(double theta, double phi)
{
	pointing dir;
    dir.theta = theta;
    dir.phi = phi;

    // Find the pixel index of this vector.
	HealpixMapDbl& skyMap = *( (HealpixMapDbl*) skyMap_);
    return skyMap.ang2pix(dir);
}

 floatpair HealPixCoordinate::GetCoords(uint32_t index)
{
    // Read in RA, DECL in degrees.
    // Convert to a HEALPix pointing object, in radians.
	HealpixMapDbl& skyMap = *( (HealpixMapDbl*) skyMap_);
	pointing dir = skyMap.pix2ang(index);

	double phi,theta;
	if (!rand_) {
        log_warn("Random number generator not set. I won't smooth");
	    phi   = dir.phi;
	    theta = dir.theta;
    } else {
	    phi   = rand_->Uniform(
                    max(dir.phi-pix_size_*.5,dir.phi),
                    min(dir.phi+pix_size_*.5,dir.phi));
	    theta = rand_->Uniform(
                    max(dir.theta-pix_size_*.5,dir.theta),
                    min(dir.theta+pix_size_*.5,dir.theta));
    }
	return floatpair(theta,phi);
}

uint8_t HealPixCoordinate::NumberOfBins() 
{ 
	HealpixMapDbl& skyMap = *( (HealpixMapDbl*) skyMap_);
    return skyMap.Npix(); 
}

double angdist(const vec3 &v1, const vec3 &v2)
{
   vec3 r1 = v1;   r1.Normalize();
   vec3 r2 = v2;   r2.Normalize();
   double dist = 0.0;
   double sprod = dotprod(r1, r2);

   if (sprod > 0.999) {          // Vectors almost parallel.
      vec3 vdiff  = r1 - r2;
      double diff = std::sqrt(dotprod(vdiff, vdiff));
      dist = 2.0 * std::asin(0.5 * diff);
   }
   else if (sprod < -0.999) {     // Vectors almost antiparallel.
      vec3 vsum  = r1 + r2;
      double sum = std::sqrt(dotprod(vsum, vsum));
      dist = I3Constants::pi - 2.0 * std::asin(0.5 * sum);
   }
   else {
      dist = std::acos(sprod);
   }
   return dist;
}

