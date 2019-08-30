/**
 * @brief declaration of the PhPnhParam class
 *
 * @file PhPnhParam.h
 * @version $Revision$
 * @date $Date$
 * @author Sebastian Euler <sebastian.euler@icecube.wisc.edu>
 *
 * This class derives from PhPnhProbBase. It calculates the hit probability by an analytic function. This parametrization has two free parameters. As default the AMANDA best fit values are inserted. (see <https://wiki.icecube.wisc.edu/index.php/FiniteReco.CalculationOfProbabilities>)
 */

#ifndef PHPNHPARAM_H_INCLUDED
#define PHPNHPARAM_H_INCLUDED

#include <cmath>

#include "finiteReco/probability/PhPnhProbBase.h"
#include "icetray/IcetrayFwd.h"
#include "dataclasses/I3Constants.h"


class PhPnhParam : public PhPnhProbBase {
 public:
  PhPnhParam();
  PhPnhParam(double tau, const double& absorptionLength);
  virtual ~PhPnhParam();
  
  double GetHitProb(const I3Particle& track, const I3OMGeo& omgeo, const int& Nhit) const;
 private:
  double GetHitProb(const double& chdist,const double& ori, const double& energy, const int& Nhit = 1) const;
  double tau_;
  double l_a_;
};

I3_POINTER_TYPEDEFS( PhPnhParam );
#endif /* PHPNHPARAM_H_INCLUDED */
