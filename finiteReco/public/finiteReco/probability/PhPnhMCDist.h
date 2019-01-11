/**
 * @brief declaration of the PhPnhMCDist class
 *
 * @file PhPnhMCDist.h
 * @version $Revision$
 * @date $Date$
 * @author Sebastian Euler <sebastian.euler@icecube.wisc.edu>
 *
 * This class derives from PhPnhProbBase. It obtains the hit probability at a certain distance from a given table. This tables is averaged over energy and direction. The table has to be ASCII with the step width in meters in the first line and the maximum distance in meters in the second line. All lines afterwards contain the probabilities for each bin with float precision. (see <https://wiki.icecube.wisc.edu/index.php/FiniteReco.CalculationOfProbabilities>)
 */

#ifndef PHPNHMCDIST_H_INCLUDED
#define PHPNHMCDIST_H_INCLUDED

#include <string>

#include "finiteReco/probability/PhPnhProbBase.h"
#include "icetray/IcetrayFwd.h"


class PhPnhMCDist : public PhPnhProbBase {
 public:
  PhPnhMCDist(const std::string& inputProbFile_);
  virtual ~PhPnhMCDist();
  double GetHitProb(const I3Particle& track, const I3Position& pos, const int& Nhit) const;
 private:
  double GetHitProb(const double& chdist,const double& ori, const double& energy, const int& Nhit = 1) const;
  PhPnhMCDist();
  float stepSize_;
  float maxDist_;
  std::vector<float> prob_;
};

I3_POINTER_TYPEDEFS( PhPnhMCDist );
#endif /* PHPNHMCDIST_H_INCLUDED */
