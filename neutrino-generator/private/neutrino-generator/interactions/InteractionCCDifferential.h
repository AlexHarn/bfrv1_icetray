#ifndef NuGINTERACTIONCCD_H
#define NuGINTERACTIONCCD_H

/**
 *   Copyright  (C) 2005
 *   The IceCube Collaboration
 *   $Id: $
 *
 *   @file InteractionCCDifferential.h
 *   @version $Revision: $
 *   @date    $Date:     $ 
 *   @author Aya Ishihara <aya.ishihara@icecube.wisc.edu>
 *
 *   @brief InteractionCCDifferential header file
 *   Standard Model cross-sections derived from sigma
 */

#include "phys-services/I3CrossSection.h"
#include "neutrino-generator/interactions/InteractionCC.h"

namespace nugen {

I3_FORWARD_DECLARATION(Steering);
I3_FORWARD_DECLARATION(Particle);

class InteractionCCDifferential : public InteractionCC {

 public:
  InteractionCCDifferential(I3RandomServicePtr random, 
                boost::shared_ptr<Steering> steer);
  virtual ~InteractionCCDifferential();

  void InitializeI3CrossSectionTables(const std::string &differential, const std::string &total);

  // overload get functions
  virtual double GetMinEnergy();
  virtual double GetMaxEnergy();

  //overload util functions
  /**
   @brief returns vector<double> = (Bjorken X,  Bjorken Y)
	private/neutrino-generator/legacy/I3NuGInteractionInfo.cxx
   */
  std::vector<double> SelectXY(double log_e, I3Particle::ParticleType ptype);

  /**
   @brief returns cross section of this interaction in [cm^2]
   */
  double GetXsecCGS (const double energy);

 private:

  //don't use these functions.
  void InitializeCrosssectionTable(const std::string &) {}
  void InitializeFinalStateTable(const std::string &) {}

  InteractionCCDifferential() : InteractionCC() {}
  I3CrossSection xsec_;
  SET_LOGGER("I3NuG");

};
I3_POINTER_TYPEDEFS(InteractionCCDifferential);

}
#endif //I3NuGINTERACTIONCC_H


