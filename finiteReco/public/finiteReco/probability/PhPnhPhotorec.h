/**
 * @brief declaration of the PhPnhPhotorec class
 *
 * @file PhPnhPhotorec.h
 * @version $Revision$
 * @date $Date$
 * @author Sebastian Euler <sebastian.euler@icecube.wisc.edu>
 *
 * This class derives from PhPnhProbBase. It calculates the hit probability at the given position by a call to the photonics tables. A corresponding pointer must be provided. Photorec requires an energy to determine the probability. If no energy is provided with the track, 1 TeV is assumed. (see <https://wiki.icecube.wisc.edu/index.php/FiniteReco.CalculationOfProbabilities>)
 */

#ifndef PHPNHPHOTOREC_H_INCLUDED
#define PHPNHPHOTOREC_H_INCLUDED

#include <string>

#include "finiteReco/probability/PhPnhProbBase.h"
#include "icetray/IcetrayFwd.h"
#include "gulliver/I3PDFBase.h"
#include "photonics-service/I3PhotonicsServiceFactory.h"


class PhPnhPhotorec : public PhPnhProbBase {
 public:
  PhPnhPhotorec(I3PhotonicsServicePtr photorecPtr, const double& finiteDefaultLength = 2*I3Units::km, const bool& onlyInfiniteTables = false):
    PhotorecPtr_(photorecPtr),
    finiteDefaultLength_(finiteDefaultLength),
    onlyInfiniteTables_(onlyInfiniteTables){}
  virtual ~PhPnhPhotorec(){}
  
  double GetHitProb(const I3Particle& track, const I3OMGeo& omgeo, const int& Nhit =1) const;
 private:
  I3PhotonicsServicePtr PhotorecPtr_;
  double finiteDefaultLength_;
  bool onlyInfiniteTables_;
  PhPnhPhotorec(){}
};

I3_POINTER_TYPEDEFS( PhPnhPhotorec );
#endif /* PHPNHPHOTOREC_H_INCLUDED */
