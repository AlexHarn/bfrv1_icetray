#ifndef I3NuGINTERACTIONINFODiff_H
#define I3NuGINTERACTIONINFODiff_H
/**
 *   Copyright  (C) 2005
 *   The IceCube Collaboration
 *   $Id: $
 *
 *   @file I3NuGInteractionInfoDifferential.h
 *   @version $Revision: $
 *   @date    $Date:     $ 
 *   @author Aya Ishihara <aya.ishihara@icecube.wisc.edu>
 *
 *   @brief I3NuGInteractionInfoDifferential header file
 *   Total Crosssection header file for I3NeutrinoGenerator
 *   interface to interaction and decay classes 
 *   sigmatotal is the sum of all crossections     
 */

#include "dataclasses/physics/I3Particle.h"
#include "earthmodel-service/EarthModelService.h"
#include "phys-services/I3RandomService.h"
#include "icetray/I3ServiceBase.h"
#include "neutrino-generator/utils/EnumTypeDefs.h"
#include <vector>
#include <map>

////////////////////////////////////////////////////////////////


#include "neutrino-generator/legacy/I3NuGInteractionInfo.h"

namespace nugen {
   I3_FORWARD_DECLARATION(Steering); 
   I3_FORWARD_DECLARATION(Particle); 
   I3_FORWARD_DECLARATION(InteractionBase); 
   I3_FORWARD_DECLARATION(TauDecay); 
}

class I3NuGInteractionInfoDifferential : public I3NuGInteractionInfo{

 public:

  /**
   * constructor & destructor
   */
  I3NuGInteractionInfoDifferential(const I3Context &c);
  I3NuGInteractionInfoDifferential(I3RandomServicePtr random,
                       boost::shared_ptr<nugen::Steering> steer,
                       const std::string &xsec_name);
  //I3NuGInteractionInfoDifferential(const I3NuGInteractionInfoDifferential &c);

  virtual ~I3NuGInteractionInfoDifferential();

 protected:

  void   ReadInteractionFiles();
  I3NuGInteractionInfoDifferential() : I3NuGInteractionInfo() {}

  SET_LOGGER("I3NuG"); 
};

////////////////////////////////////////////////////////////////
typedef boost::shared_ptr<I3NuGInteractionInfoDifferential> I3NuGInteractionInfoDifferentialPtr;

#endif //I3NuGINTERACTIONINFO_h









