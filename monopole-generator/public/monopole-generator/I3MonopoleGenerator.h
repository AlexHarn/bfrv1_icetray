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
 * @brief A module to generate a magnetic monopole particle
 */
#include "icetray/I3Module.h"
#include <string>

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

  /**
   * User input parameters
   */
  std::string treeName_;
  std::string infoName_;
  double mass_;
  double gamma_;
  double beta_;
  double betaMin_;
  double betaMax_;
  std::vector<double> betaRange_;
  bool useBetaRange_;
  double diskDist_;
  double diskRad_;
  bool randPos_;
  double radOnDisk_;
  double aziOnDisk_;
  double startTime_;

  /**
   * Zenith and Azimuth follow convention defined in I3Direction.h
   * Angle points to origin of particle, hence zenith=0 is down-going
   */
  double zenithMin_;
  double zenithMax_;
  std::vector<double> zenithRange_;
  double azimuthMin_;
  double azimuthMax_;
  std::vector<double> azimuthRange_;


  std::vector<double> shiftCenter_;
  double powerLawIndex_;
  double length_;

  double timescale_;

  SET_LOGGER("I3MonopoleGenerator");

};
#endif //MONOPOLE_GENERATOR_I3MONOPOLEGENERATOR_H_INCLUDED
