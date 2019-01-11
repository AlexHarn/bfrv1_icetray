/**
 * @brief declaration of the PhPnhProbBase class
 *
 * @file PhPnhProbBase.h
 * @version $Revision$
 * @date $Date$
 * @author Sebastian Euler <sebastian.euler@icecube.wisc.edu>
 *
 * This class provides a general interface to derive a hit probability for a DOM at a certain position. The assumed track (I3Particle) and the position of the DOM are the only required information.
 */

#ifndef PHPNHPROBBASE_H_INCLUDED
#define PHPNHPROBBASE_H_INCLUDED

#include "icetray/IcetrayFwd.h"
#include "icetray/I3DefaultName.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/I3Position.h"
#include "phys-services/I3Calculator.h"


class PhPnhProbBase {
 public:
  PhPnhProbBase();
  virtual ~PhPnhProbBase();
  virtual double GetHitProb(const I3Particle& track, const I3Position& pos, const int& Nhit = -1) const = 0;
  double GetNoHitProb(const I3Particle& track, const I3Position& pos, const int& Nhit = -1) const;
  bool CherenkovCalc(const I3Particle& track,
                     const I3Position& pos,
                     I3Position& chpos,
                     double& chtime,
                     double& chdist,
                     double& ori) const;
 private:
};
I3_POINTER_TYPEDEFS( PhPnhProbBase );
#endif /* PHPNHPROBBASE_H_INCLUDED */
