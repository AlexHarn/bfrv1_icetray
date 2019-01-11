#ifndef NuGDefaults_HH
#define NuGDefaults_HH
//--------------------------------------
// NuGen default names

#include "icetray/I3Units.h"
#include "neutrino-generator/utils/EnumTypeDefs.h"
#include "phys-services/surfaces/Cylinder.h"
#include <string>

namespace nugen {
namespace Defaults {

const int         nEvts = -1;

const std::string simModeString = "Full";
const SimMode     simMode = FULL;

const std::string vtxGenModeString = "NuGen"; 
const VTXGenMode  vtxGenMode = NUGEN; 
const double      vtxGenEThreshold = 190 * I3Units::GeV;

const std::string benchmarkModeString = "Off";
const BenchMode   benchmarkMode = OFF;

const std::string mcTreeName = "I3MCTree";
const std::string mcWeightDictName = "I3MCWeightDict";
const std::string primaryNuName = "NuGPrimary";

const std::string steerName = "NuGSteer";
const std::string earthModelName = "EarthModelService";

const std::string injectionModeString = "Surface";
const InjectionMode injectionMode = SURFACE;

const std::string angleSamplingModeString = "COS";
const AngleSamplingMode angleSamplingMode = COS;

const std::string propagationModeString = "AutoDetect";
const PropagationMode propagationMode = AUTODETECT;

const double      injectionRadius = 1200*I3Units::m;
const double      activeHeightBefore = 1200.0*I3Units::m;
const double      activeHeightAfter = 1200.0*I3Units::m;
const double      cylinderHeight = 1900.0*I3Units::m;
const double      cylinderRadius = 950.0*I3Units::m;
const I3Surfaces::SamplingSurfacePtr detectionSurface = I3Surfaces::SamplingSurfacePtr(new I3Surfaces::Cylinder(cylinderHeight, cylinderRadius));

const double      energyMin = 10 * I3Units::GeV;
const double      energyMax = 1e12 * I3Units::GeV;

const bool        doMuRangeExtension = true;
const double      muRangeOpt = 11;

const double      densityTolerance = 0.05;
const double      psurvApproxLimit = 1e-5;
const bool        useSimpleScatterForm = false;
const bool        ignoreOutgoingAngleForNC = true;

const double      maxDetVolLen = 300.*I3Units::kilometer;// max detection vol_len 


// parameters that has been stored in StaticParam
const int         nsteps = 10000;
const double      stepSize   = 10 * I3Units::m;
const int         intpos_sample_opt = 1;
const int         interact_weight_opt = 1;
const int         crosssectionxcolumndepth_opt = 0;
const double      crosssectionxcolumndepth_stepsize = 10.0 * I3Units::m;
const int         impactparam_opt = 0;

}
}

#endif




