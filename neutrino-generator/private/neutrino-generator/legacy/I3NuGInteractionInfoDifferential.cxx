/**
 *   Copyright  (C) 2005
 *   The IceCube Collaboration
 *   $Id: $
 *
 *   @file I3NuGInteractionInfoDifferential.cxx
 *   @version $Revision: $
 *   @date    $Date:     $ 
 *   @author Aya Ishihara <aya.ishihara@icecube.wisc.edu>
 *
 *   @brief I3NuGInteractionInfoDifferential implementaion file
 *   This class accesss to the classes containing 
 *   all active interaction channels 
 */
#include "neutrino-generator/Steering.h"
#include "neutrino-generator/Particle.h"
#include "neutrino-generator/legacy/I3NuGInteractionInfoDifferential.h"
#include "neutrino-generator/interactions/InteractionBase.h"
#include "neutrino-generator/interactions/InteractionCCDifferential.h"
#include "neutrino-generator/interactions/InteractionNCDifferential.h"
#include "neutrino-generator/interactions/InteractionGR.h"
#include "neutrino-generator/interactions/TauDecay.h"
#include "neutrino-generator/utils/Constants.h"
#include "neutrino-generator/utils/Calculator.h"
#include "neutrino-generator/utils/EnumTypeDefs.h"
#include "neutrino-generator/table-interface/TableUtils.h"
#include "phys-services/I3RandomService.h"
#include "icetray/I3SingleServiceFactory.h"
#include "dataclasses/I3Constants.h"

#include <boost/foreach.hpp>

using namespace std;
using namespace nugen;
using namespace earthmodel;

//_____________________________________________________________
I3NuGInteractionInfoDifferential::I3NuGInteractionInfoDifferential(const I3Context &c):
  I3NuGInteractionInfo(c)
{
  log_trace("construct I3NuGInteractionInfoDifferential");
}

//_____________________________________________________________
I3NuGInteractionInfoDifferential::I3NuGInteractionInfoDifferential(
                       I3RandomServicePtr random, 
                       SteeringPtr steer,
                       const string &xsec_model):
  I3NuGInteractionInfo(random, steer, xsec_model)
{
  log_trace("construct I3NuGInteractionInfoDifferential");
}

//_____________________________________________________________
I3NuGInteractionInfoDifferential::~I3NuGInteractionInfoDifferential()
{
  //destructor is responsible for deleting the partial cross sections
}


//_____________________________________________________________
void I3NuGInteractionInfoDifferential::ReadInteractionFiles()
{
  
  interactionVect_.clear();

  log_debug("Read neutrino interaction files");
  //CrossSection(std::string dd_crossSectionFile, std::string total_crossSectionFile){
  //interactionVect_.push_back(InteractionCCNu_ptr);
  //interactionVect_.push_back(InteractionNCNu_ptr);
  //interactionMap_["CC_Nu_iso"] = InteractionCCNu_ptr;
  //interactionMap_["NC_Nu_iso"] = InteractionNCNu_ptr;

  log_debug("Read neutrino interaction files");
  InteractionCCDifferentialPtr InteractionCCNu_ptr(new InteractionCCDifferential(random_, steer_));
  InteractionNCDifferentialPtr InteractionNCNu_ptr(new InteractionNCDifferential(random_, steer_));
  InteractionCCNu_ptr->InitializeI3CrossSectionTables(xsecs_["CC_Nu_iso_final"], xsecs_["CC_Nu_iso_xsec"]);
  InteractionNCNu_ptr->InitializeI3CrossSectionTables(xsecs_["NC_Nu_iso_final"], xsecs_["NC_Nu_iso_xsec"]);

  InteractionCCNu_ptr->SetActiveFlavorMask(flavorMaskCCNu_);
  InteractionNCNu_ptr->SetActiveFlavorMask(flavorMaskNCNu_);

  InteractionCCNu_ptr->SetActiveMaterialMask(materialMaskCCNu_);
  InteractionNCNu_ptr->SetActiveMaterialMask(materialMaskNCNu_);

  interactionVect_.push_back(InteractionCCNu_ptr);
  interactionVect_.push_back(InteractionNCNu_ptr);
  interactionMap_["CC_Nu_iso"] = InteractionCCNu_ptr;
  interactionMap_["NC_Nu_iso"] = InteractionNCNu_ptr;

  log_debug("Read anti-neutrino interaction files");
  InteractionCCDifferentialPtr InteractionCCNuBar_ptr(new InteractionCCDifferential(random_, steer_));
  InteractionNCDifferentialPtr InteractionNCNuBar_ptr(new InteractionNCDifferential(random_, steer_));
  // I don't have grashow resonance differential table yet, so use old class.
  InteractionGRPtr InteractionGRNuBar_ptr(new InteractionGR(random_, steer_));
    
  InteractionCCNuBar_ptr->InitializeI3CrossSectionTables(xsecs_["CC_NuBar_iso_final"], xsecs_["CC_NuBar_iso_xsec"]);
  InteractionNCNuBar_ptr->InitializeI3CrossSectionTables(xsecs_["NC_NuBar_iso_final"],xsecs_["NC_NuBar_iso_xsec"]);

  InteractionCCNuBar_ptr->SetActiveFlavorMask(flavorMaskCCNuBar_);
  InteractionNCNuBar_ptr->SetActiveFlavorMask(flavorMaskNCNuBar_);
  InteractionGRNuBar_ptr->SetActiveFlavorMask(flavorMaskGRNuBar_);

  InteractionCCNuBar_ptr->SetActiveMaterialMask(materialMaskCCNuBar_);
  InteractionNCNuBar_ptr->SetActiveMaterialMask(materialMaskNCNuBar_);
  InteractionGRNuBar_ptr->SetActiveMaterialMask(materialMaskGRNuBar_);

  interactionVect_.push_back(InteractionGRNuBar_ptr);
  interactionVect_.push_back(InteractionCCNuBar_ptr);
  interactionVect_.push_back(InteractionNCNuBar_ptr);
  interactionMap_["CC_NuBar_iso"] = InteractionCCNuBar_ptr;
  interactionMap_["NC_NuBar_iso"] = InteractionNCNuBar_ptr;
  interactionMap_["GR_NuBar"] = InteractionGRNuBar_ptr;

  vector<InteractionBasePtr>::const_iterator iter = interactionVect_.begin();
  for(; iter != interactionVect_.end(); ++iter){
     InteractionBasePtr int_ptr = *iter;    

     // set supported min energy and max energy
     // find energy range covered by all cross sections
     double enemin = int_ptr->GetMinEnergy();
     double enemax = int_ptr->GetMaxEnergy();
     log_debug("enemin = %g, enemax = %g", enemin, enemax);
     if (energyMin_ < enemin) {
        energyMin_ = enemin;
     }
     if (energyMax_ > enemax) {
        energyMax_ = enemax;
     }

     if(int_ptr)log_trace("Cross-section file: %s", int_ptr->GetCrosssectionFileName().c_str());
  }

  //----------------------------------------------------
  // initialize flag for GetTotalCrossSection()
  prev_state_crosssec_.use_prev_interaction_vect_ = false;
  //----------------------------------------------------

  log_trace("Interaction files are read");
}

typedef I3SingleServiceFactory<I3NuGInteractionInfoDifferential>
I3NuGInteractionInfoDifferentialFactory;
I3_SERVICE_FACTORY( I3NuGInteractionInfoDifferentialFactory );

