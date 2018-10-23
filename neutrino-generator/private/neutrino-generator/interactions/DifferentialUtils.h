#ifndef NuGDifferentialUtils_H
#define NuGDifferentialUtils_H
/**
 *   copyright  (C) 2005
 *   the IceCube Collaboration
 *   $Id:  $
 *
 *   @version $Revision: $
 *   @date $Date: $
 *   @author Kotoyo Hoshina <hoshina@icecube.wisc.edu>
 *
 *   @brief utility class for intarction classes using 
 *    differential cross sections
 * 
 */

#include "phys-services/I3CrossSection.h"
#include "dataclasses/physics/I3Particle.h"
#include <vector>

namespace nugen {
namespace DifferentialUtils {

void InitializeI3CrossSectionTables(const std::string &differential, 
                       const std::string& total, I3CrossSection &xsec);

std::vector<double> SelectXY(double log_e, I3Particle::ParticleType ptype, 
                              I3CrossSection &xsec,
                              I3RandomServicePtr random) ;

double GetXsecCGS(const double energy, I3CrossSection &xsec) ;

} // DifferentialUtils
} // nugen

#endif //NuGDifferentialUtils_H

