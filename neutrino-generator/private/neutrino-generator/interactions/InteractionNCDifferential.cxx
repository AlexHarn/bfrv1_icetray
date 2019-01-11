/**
 *   copyright  (C) 2005
 *   the IceCube Collaboration
 *   $Id:  $
 *
 *   @version $Revision: $
 *   @date $Date: $
 *   @author Kotoyo Hoshina<hoshina@icecube.wisc.edu>
 *
 *   @brief NC interaction class that uses differential cross section
 * 
 */
//////////////////////////////////////////////////////////////////////// 
#include "neutrino-generator/interactions/InteractionBase.h"
#include "neutrino-generator/interactions/InteractionNCDifferential.h"
#include "neutrino-generator/interactions/DifferentialUtils.h"
#include "neutrino-generator/Particle.h"
#include "neutrino-generator/Steering.h"
#include "neutrino-generator/utils/Constants.h"
#include "neutrino-generator/table-interface/CrosssectionTableReader.h"
#include "neutrino-generator/table-interface/FinalStateTableReader.h"
#include "dataclasses/I3Constants.h"

namespace nugen {

//____________________________________________________________________
InteractionNCDifferential::InteractionNCDifferential(
                             I3RandomServicePtr random,
                             SteeringPtr steer):
  InteractionNC(random, steer)
{
   log_debug("Constructing NC Differential interaction");
}

//____________________________________________________________________
InteractionNCDifferential::~InteractionNCDifferential()
{
}

//____________________________________________________________________
void InteractionNCDifferential::InitializeI3CrossSectionTables(
                 const std::string &differential, const std::string& total)
{
  finalname_ = differential;
  sigmaname_ = total;
  DifferentialUtils::InitializeI3CrossSectionTables(differential, total, xsec_);
}

//___________________________________________________________________
std::vector<double> InteractionNCDifferential::SelectXY(
                      double log_e, I3Particle::ParticleType ptype) 
{
  return DifferentialUtils::SelectXY(log_e, ptype, xsec_, random_);
}

//____________________________________________________________________
double InteractionNCDifferential::GetXsecCGS(const double energy) 
{
   return DifferentialUtils::GetXsecCGS(energy, xsec_);
}

//____________________________________________________________________
double InteractionNCDifferential::GetMinEnergy()
{
   // TODO : this code will be replaced to GetMinimumEnergy() 
   // at the next minor update
   const splinetable& spt = xsec_.getTotalCrossSection();
   return std::pow(10, spt.extents[0][0]);

}

//____________________________________________________________________
double InteractionNCDifferential::GetMaxEnergy()
{
   // TODO : this code will be replaced to GetMinimumEnergy() 
   // at the next minor update
   const splinetable& spt = xsec_.getTotalCrossSection();
   return std::pow(10, spt.extents[0][1]);
}


} // nugen
