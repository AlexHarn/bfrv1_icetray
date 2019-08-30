/**
 * @brief implementation of the PhPnhPhotorecCone class
 *
 * @file PhPnhPhotorecCone.cxx
 * @version $Revision: 52296 $
 * @date $Date: 2009-02-03 16:30:36 +0100 (Tue, 03 Feb 2009) $
 * @author Sebastian Euler <sebastian.euler@icecube.wisc.edu>
 *
 * This class derives from PhPnhProbBase. It calculates the hit probability at the given position by a call to the photonics tables. A corresponding pointer must be provided. Photorec requires an energy to determine the probability. If no energy is provided with the track, 1 TeV is assumed.
 * The probability is multiplied by a step function, to account for physical/non-physical hits. (see <https://wiki.icecube.wisc.edu/index.php/FiniteReco.CalculationOfProbabilities>)
 */

#include "finiteReco/probability/PhPnhPhotorecCone.h"

PhPnhPhotorecCone::PhPnhPhotorecCone(I3PhotonicsServicePtr photorecPtr, const double& finiteDefaultLength, const bool& onlyInfiniteTables):
  PhotorecPtr_(photorecPtr),
  finiteDefaultLength_(finiteDefaultLength),
  onlyInfiniteTables_(onlyInfiniteTables)
{}

PhPnhPhotorecCone::~PhPnhPhotorecCone(){}
  
//double PhPnhPhotorecCone::GetHitProb(const I3Particle& track, const I3Position& pos, const int& Nhit) const{ 
double PhPnhPhotorecCone::GetHitProb(const I3Particle& track, const I3OMGeo& omgeo, const int& Nhit) const{ 
  // look if the position is within a possible light cone from the track
  int factor = GetCone ( track, omgeo.position );
  if ( factor == 0 ) return 0.;
  PhPnhPhotorecPtr probPtr ( new PhPnhPhotorec ( PhotorecPtr_, finiteDefaultLength_, onlyInfiniteTables_ )  );
  double hitProb = probPtr->GetHitProb ( track, omgeo, Nhit);
  return hitProb * factor;
}

int PhPnhPhotorecCone::GetCone (const I3Particle& track, const I3Position& pos ) const{
  I3Position chPos;
  double chTime;
  double ori;
  double chDist;
  bool isOK = this->CherenkovCalc( track, pos, chPos, chTime, chDist, ori );
  if(!isOK) return 0;
  return 1;
}

PhPnhPhotorecCone::PhPnhPhotorecCone(){}

