/**
 *   copyright  (C) 2005
 *   the IceCube Collaboration
 *   $Id:  $
 *
 *   @version $Revision: $
 *   @date $Date: $
 *   @author Aya Ishihara <aya.ishihara@icecube.wisc.edu>
 *
 *   @brief Crosssection implimentation file for I3NeutrinoGeneratorCC
 *   here you find all the final state classes for processes in the
 *   Standard Model. The default data is supplied through tables.
 * 
 */

#include "neutrino-generator/Steering.h"
#include "neutrino-generator/Particle.h"
#include "neutrino-generator/interactions/InteractionBase.h"
#include "neutrino-generator/interactions/InteractionCCDifferential.h"
#include "neutrino-generator/interactions/DifferentialUtils.h"
#include "neutrino-generator/utils/Constants.h"
#include "neutrino-generator/table-interface/CrosssectionTableReader.h"
#include "neutrino-generator/table-interface/FinalStateTableReader.h"
#include "dataclasses/I3Constants.h"

namespace nugen {

using namespace Constants;

//___________________________________________________________________
InteractionCCDifferential::InteractionCCDifferential(I3RandomServicePtr random,
                             SteeringPtr steer):
  InteractionCC(random, steer)
{
  log_debug("Constructing CCDifferential interaction");
}

//___________________________________________________________________
InteractionCCDifferential::~InteractionCCDifferential()
{
  log_trace("deconstruct InteractionCCDifferential");
}

//____________________________________________________________________
void InteractionCCDifferential::InitializeI3CrossSectionTables(const std::string &differential, const std::string& total)
{
  finalname_ = differential;
  sigmaname_ = total;
  DifferentialUtils::InitializeI3CrossSectionTables(differential, total, xsec_);
}

//___________________________________________________________________
std::vector<double> InteractionCCDifferential::SelectXY(double log_e, I3Particle::ParticleType ptype) 
{
  return DifferentialUtils::SelectXY(log_e, ptype, xsec_, random_);
}

//____________________________________________________________________
double InteractionCCDifferential::GetXsecCGS(const double energy) 
{
   return DifferentialUtils::GetXsecCGS(energy, xsec_);

}

//____________________________________________________________________
double InteractionCCDifferential::GetMinEnergy()
{
   // TODO : this code will be replaced to GetMinimumEnergy() 
   // at the next minor update
   const splinetable& spt = xsec_.getTotalCrossSection();
   return std::pow(10, spt.extents[0][0]);

}

//____________________________________________________________________
double InteractionCCDifferential::GetMaxEnergy()
{
   // TODO : this code will be replaced to GetMinimumEnergy() 
   // at the next minor update
   const splinetable& spt = xsec_.getTotalCrossSection();
   return std::pow(10, spt.extents[0][1]);
}


} // nugen

