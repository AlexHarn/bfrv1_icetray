/**
 * Copyright (C) 2009
 * The IceCube collaboration
 * ID: $Id$
 *
 * @file I3G4TankResponse.h
 * @version $Revision$
 * @date $Date$
 * @author Tilo Waldenmaier <tilo.waldenmaier@desy.de>, Thomas Melzig <thomas.melzig@desy.de>
 *
 * $LastChangedBy$
 */


#ifndef _TOPSIM_I3G4TANKRESPONSE_H_
#define _TOPSIM_I3G4TANKRESPONSE_H_

#include <topsimulator/interface/I3TankResponse.h>

#include <icetray/I3Configuration.h>
#include <icetray/I3Logging.h>

class G4Interface;
class G4IceTopTank;
class I3Particle;

// This constant used to be in <pmt-simulator/I3PMTConstants.h>, which is now deprecated.
// Does it exist anywhere else now?
/**
 * This parameter needs to be updated whenever
 * the charge distribution constants change.
 * This should be caught in the tests.
 */
//   const double MEAN_NORMALIZED_PE(0.850529);
#define MEAN_NORMALIZED_PE 0.850529

I3_FORWARD_DECLARATION(I3RandomService);

/**
 * THE one and only service... implementation of topsimulator's I3TankResponse. The top level from a user's point of view. It delegates all Geant4 interaction to G4Interface.
 */
class I3G4TankResponse : public I3TankResponse
{
 public:
  I3G4TankResponse(I3Configuration& config, const I3Context& context, const TankKey& tankKey);
  ~I3G4TankResponse();

  void Initialize(const I3Geometry& geometry,
                  const I3Calibration& calib,
                  const I3DetectorStatus& status);

  void Configure();

  double GetX() const;
  double GetY() const;
  double GetZ() const;

  double GetTankRadius() const;
  double GetTankHeight() const;
  double GetSnowHeight() const;

  double GetVirtualTankRadius() const {return GetTankRadius() + safetyMargin_;}
  double GetVirtualTankHeight() const {return GetTankHeight() + 2.0*safetyMargin_;}

  bool TrackParticle(const ExtendedI3Particle& particle, HitHistoCollection& hitHC, HitHistoCollection& cherHitCollection);

  /// Called at the beginning of I3TopSimulator::DAQ
  void BeginEvent(const I3Particle& primary);

  /// Called at the end of I3TopSimulator::DAQ
  void EndEvent(HitHistoCollection &hitHC, HitHistoCollection& cherHitCollection);


 private:
  bool IntersectTank(const I3Particle& particle, double margin);

  void GenerateHits(int npe, const ExtendedI3Particle& particle, HitHisto& hitHisto);

  double tauZirco_;
  double tauTyvek_;
  double decayTime_;
  double safetyMargin_;
  double chargeScale_;

  bool cherenkovMethod_;
  bool cherenkovWeighting_;
  int particlesBeforeThreshold_;
  int particlesAfterThreshold_;
  double vemCounter_;

  std::string visMacroFile_;
  std::map<OMKey, double> pePerVEM_;
  std::string randomServiceName_;
  I3RandomServicePtr randomService_;

  G4Interface* g4Interface_;
  G4IceTopTank* g4Tank_;

  static const double PHOTONS_PER_VEM;
  static const double WEIGHTED_PHOTONS_PER_VEM;
  static const double MEV_PER_VEM;
  static const double VEM_THRESHOLD;

  SET_LOGGER("I3G4TankResponse");
};

I3_POINTER_TYPEDEFS(I3G4TankResponse);

#endif
