/**
 * @brief declaration of the PhPnhPhotorecCone class
 *
 * @file PhPnhPhotorecCone.h
 * @version $Revision: 52296 $
 * @date $Date: 2009-02-03 16:30:36 +0100 (Tue, 03 Feb 2009) $
 * @author Sebastian Euler <sebastian.euler@icecube.wisc.edu>
 *
 * This class derives from PhPnhProbBase. It calculates the hit probability at the given position by a call to the photonics tables. A corresponding pointer must be provided. Photorec requires an energy to determine the probability. If no energy is provided with the track, 1 TeV is assumed.
 * The probability is multiplied with a step function, to account for physical/non-physical hits. (see <https://wiki.icecube.wisc.edu/index.php/FiniteReco.CalculationOfProbabilities>)
 */

#ifndef PHPNHPHOTORECCONE_H_INCLUDED
#define PHPNHPHOTORECCONE_H_INCLUDED

#include <string>

#include "finiteReco/probability/PhPnhProbBase.h"
#include "finiteReco/probability/PhPnhPhotorec.h"
#include "icetray/IcetrayFwd.h"
#include "dataclasses/physics/I3Particle.h"
#include "gulliver/I3PDFBase.h"
#include "photonics-service/I3PhotonicsServiceFactory.h"


class PhPnhPhotorecCone : public PhPnhProbBase {
 public:
  PhPnhPhotorecCone(I3PhotonicsServicePtr photorecPtr, const double& finiteDefaultLength = 2*I3Units::km, const bool& onlyInfiniteTables = false);
  virtual ~PhPnhPhotorecCone();
  double GetHitProb(const I3Particle& track, const I3OMGeo& omgeo, const int& Nhit =1) const;
  
 private:
  int GetCone (const I3Particle& track, const I3Position& pos ) const;
  I3PhotonicsServicePtr PhotorecPtr_;
  double finiteDefaultLength_;
  bool onlyInfiniteTables_;
  PhPnhPhotorecCone();
  
};

I3_POINTER_TYPEDEFS( PhPnhPhotorecCone );
#endif /* PHPNHPHOTOREC_H_INCLUDED */
