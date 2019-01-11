/**
    copyright  (C) 2004
    the icecube collaboration
    $Id: dst.cxx 16031 2006-02-20 12:38:45Z troy $

    @version $Revision: 1.2 $
    @date $Date: 2006-02-20 06:38:45 -0600 (lun, 20 feb 2006) $
    @author juancarlos@icecube.wisc.edu
*/

#include "recclasses/I3DST.h"
#include "dst/DSTCoordinates.h"

#include <iostream>
#include <cmath>

using std::cout;

float DSTCoordinate::compute_dphi(float theta)
{	
	int itheta = int( theta/dtheta0_ );
	if (abs(sin(itheta*dtheta0_*I3Units::deg)) < 1e-6 ) return 360.0;
	double th1 = itheta * dtheta0_*I3Units::deg;
	double th2 = th1 - dtheta0_*I3Units::deg;
	return min((domega0_ / abs(cos(th2) - cos(th1))) , 360.0);	// returns dphi in deg
}

DSTCoordinate::DSTCoordinate(float dtheta,float dphi,uint8_t digits)
{
	presc_ 		= int(pow(10.0,digits));
	dphi0_ 		= dphi/float(presc_);
	dtheta0_ 	= dtheta/float(presc_);
	domega0_ 	= dphi0_ * cos((90.-dtheta0_)*I3Units::deg);	// dphi0_ in deg
}


pair<uint16_t,uint16_t>  DSTCoordinate::mkpair(float theta,float phi)
{
  /*	takes the float numbers (theta, phi) and calculates the integer
	numbers (itheta, iphi) that contain decimal digit precision
	determined by presc_ and that are rounded to the center of the
	sky grid bins
  */	
	int itheta  = int( presc_ * theta);
	int idtheta = int( presc_ * dtheta0_);
	itheta = itheta - itheta % idtheta + idtheta/2;

	float dphi = compute_dphi(theta);
	int iphi    = int( presc_ * phi );
	int idphi   = int( presc_ * dphi );
	iphi = iphi - iphi % idphi + idphi/2;

	return  uintpair(itheta,min(iphi, presc_ * 360));
}

void DSTCoordinate::ComputeBins()
{
	int ntheta = int(round(90.0/dtheta0_));
	int index = 1;
	float theta = 90.0;

	log_info("dtheta %g ", dtheta0_ );
	log_info("ntheta %d ", ntheta );
	log_info("range %g ", ntheta*dtheta0_ );

	// loop over theta bands
	for (int i=0;i<ntheta+1;i++)
	{
		// compute dphi
		float dphi = compute_dphi(theta);
		float domega_band = 360.0;
		float domega_bin = dphi;
		float phi = 0.0;
		// compute number of bins in phi nphi = int (dOmega_band / dOmega_bin)
		int nphi = int(round(domega_band/domega_bin));
		// re-compute dphi from the number of bins in phi
		dphi = 360./nphi;
		log_debug("theta = %f | (dtheta,dphi) = (%f,[%f,%f]) | number of phi bins = %d\n", theta, dtheta0_, compute_dphi(theta), dphi, nphi);

		// for each theta band loop over phi angle
		for (int j=0;j< nphi+1; j++)
		{
			// make (itheta, iphi) = keys pairs ...
			uintpair keys = mkpair(theta,phi);
			// ... and make two maps:
			//     - map < keys , index >
			//     - map < index , keys >
			key2index_[keys] = index;
			index2key_[index] = keys;
			phi += dphi;
			index++;
		}
		theta -= dtheta0_;
	}
}

int16_t DSTCoordinate::GetIndex(float theta, float phi)
{
  // get index from (theta, phi)
		float zen = theta;
		
		if (std::isnan(zen) || std::isnan(phi)) {
			return 0;
		} else if (zen > 90.0) {
				zen = 180.0-theta;
				return -key2index_[mkpair(zen,phi)];
		}
		return key2index_[mkpair(zen,phi)];
}

 floatpair DSTCoordinate::GetCoords(int16_t index)
{
  // get (theta, phi) from index
		float first,second;
		if (index == 0) {
			first  = NAN;
			second = NAN;
		} else {
			uintpair p = index2key_[abs(index)];
			first  = p.first/float(presc_);
			second = p.second/float(presc_);
		}
		if (index < 0) {
			first = 180.0-first;
		}
		return floatpair(first,second);
}

