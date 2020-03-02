#ifndef MONOPOLE_GENERATOR_I3MONOPOLEGENERATOR_H_INCLUDED
#define MONOPOLE_GENERATOR_I3MONOPOLEGENERATOR_H_INCLUDED
/**
 * A Monopole Generator Module
 * (c) 2004 - 2014 IceCube Collaboration
 * Version $Id$
 *
 * @file I3MonopoleGenerator.h
 * @date $Date$
 * @author jacobi
 * @author bchristy
 * @author olivas
 * @author flauber
 * @brief A module to generate a magnetic monopole particle
 */

#include <string>
#include <cmath>
#include "icetray/I3Module.h"
#include "icetray/I3Frame.h"
#include "dataclasses/I3Double.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/physics/I3MCTree.h"
#include "dataclasses/physics/I3MCTreeUtils.h"
#include "monopole-generator/I3MonopoleGeneratorUtils.h"



class I3MonopoleGenerator : public I3Module
{
 public:

  I3MonopoleGenerator(const I3Context& ctx);
  ~I3MonopoleGenerator();

  void Configure();
  void DAQ(I3FramePtr frame);
  void Finish();

 private:
  I3MonopoleGenerator();
  I3MonopoleGenerator(const I3MonopoleGenerator&);
  I3MonopoleGenerator& operator=(const I3MonopoleGenerator&);

  std::string treeName_;
  std::string infoName_;
  int Nevents_;
  double mass_;
  double beta_;
  double gamma_;
  double speed_;
  double energy_;
  bool useBetaRange_;
  double diskDist_;
  double diskRad_;
  double radOnDisk_;
  double aziOnDisk_;
  double startTime_;
  double weight_;
  double powerLawIndex_;
  double length_;
  double totalweight_;
  I3MapStringDoublePtr mpinfo_dict_config_;

  /**
   * Zenith and Azimuth follow convention defined in I3Direction.h
   * Angle points to origin of particle, hence zenith=0 is down-going
   */
  std::vector<double> betaRange_;
  std::vector<double> zenithRange_;
  std::vector<double> azimuthRange_;
  std::vector<double> shiftCenter_;
  std::vector<double> precalculated_betas_;
  bool need_precalculate_;

  SET_LOGGER("I3MonopoleGenerator");

};
#endif //MONOPOLE_GENERATOR_I3MONOPOLEGENERATOR_H_INCLUDED
