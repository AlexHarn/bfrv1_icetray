/**
 *  Copyright  (C) 2005
 *  The IceCube collaboration
 *  $Id: $
 *  @file SourceUtils.cxx
 *  @version $Revision:  $
 *  @date    $Date:      $
 *  @author Kotoyo Hoshina
 *
 *  @brief 
 *  provides utility functions to set primary neutrino
 */

#include "earthmodel-service/EarthModelCalculator.h"
#include "neutrino-generator/sources/SourceUtils.h"
#include "neutrino-generator/Steering.h"
#include "neutrino-generator/utils/TreeUtils.h"
#include "neutrino-generator/Particle.h"
#include "neutrino-generator/utils/Calculator.h"
#include "dataclasses/I3Constants.h"
#include <string>
#include <iostream>

using namespace std;
using namespace earthmodel;

namespace nugen {

//_________________________________________________________________
ParticlePtr
SourceUtils::GeneratePrimaryNeutrino(const map<I3Particle::ParticleType, double> &pmap,
                                     I3RandomServicePtr rand,
                                     SteeringPtr steer)
{

  if (pmap.size()<1) {
     log_error("Your flavor map is empty!");
  }

  double sum = 0;
  map<double, pair<I3Particle::ParticleType, double> > copied;
  pair<I3Particle::ParticleType, double> buffer;

  map<I3Particle::ParticleType, double>::const_iterator iter;
  for (iter = pmap.begin(); iter != pmap.end(); ++iter) {
     I3Particle::ParticleType ptype = iter->first;
     double ratio = iter->second;
     buffer.first = ptype;
     buffer.second = ratio;

     if (ratio > 0) {
        sum += ratio;
        copied[sum] = buffer;
     }
  }

  if (sum <= 0) {
     log_error("negative ratio for flavor map is not allowed!");
  }

  // get one of candidates
  double random = rand->Uniform(0, sum);
  map<double, pair<I3Particle::ParticleType, double> >::iterator ii;
  ii = copied.lower_bound(random);
  I3Particle::ParticleType ptype = ii->second.first;
  double ratio = ii->second.second;

  // normalize ratio.
  double type_ratio = ratio/sum;

  // generate new particle
  ParticlePtr nu = 
         ParticlePtr(new Particle(I3Particle::Null, ptype, steer));

  // set flavor weight
  nu->GetInteractionInfoRef().SetTypeWeight(type_ratio);

  // set type etc.
  nu->SetTime(0.0*I3Units::ns);
  nu->SetSpeed(I3Constants::c);
  nu->SetShape(I3Particle::Primary);
  nu->SetFitStatus(I3Particle::NotSet);
  nu->SetLocationType(I3Particle::Anywhere);

  log_debug("particle type %d is selected", ptype);

  return nu;
}

//_________________________________________________________________
bool SourceUtils::SetPowerLowEnergy(ParticlePtr nu,
                                    double gamma, 
                                    double eminLog, double emaxLog,
                                    I3RandomServicePtr random) 
{

  double EnergyLog = 0.0;

  log_trace("injected with power law index %f, between log10(minE)=%f and log10(maxE)=%f", gamma, eminLog, emaxLog);

  if (eminLog == emaxLog) { 
     EnergyLog = eminLog;

  } else {
    
     if (gamma == 1.0) {
        EnergyLog = random->Uniform( eminLog, emaxLog );

     } else if (gamma > 0) {
        double rand = random->Uniform(0., 1.);
        double energyP = (1-rand)*pow(pow(10,eminLog),1-gamma)
                          +rand*pow(pow(10,emaxLog),1-gamma);
        double energy  =  pow(energyP,1/(1-gamma));
        EnergyLog = log10( energy );

     } else {
        double energy = random->Uniform(pow(10,eminLog), pow(10,emaxLog));
        EnergyLog = log10( energy );
        log_error("Negative power is not supported. " 
                  "set constant energy %f instead", energy);
     }
    
  }
  log_trace("injected with log E=%.10f", EnergyLog);

  double energy = pow(10, EnergyLog) * I3Units::GeV;
  if (energy < 0) {
     log_error("Energy is negative! e=%f", energy);
     return false;
  }

  nu->SetEnergy(energy);

  return true;

}

//_________________________________________________________________
bool SourceUtils::SetNuPositions( ParticlePtr nu_ptr,
                                  EarthModelServicePtr earth,
                                  I3RandomServicePtr random,
                                  SteeringPtr steer)
{
  // the input nu_ptr must have direction information in advance! 

  I3Direction dirDC(nu_ptr->GetDir());

  // select a random position within injection area.
  I3Position ipDC = SelectRandomPos(nu_ptr, random, steer);

  // convert them to EarthCenter coordinate
  I3Direction dirCE(earth->GetEarthCoordDirFromDetCoordDir(dirDC)); 
  I3Position ipCE(earth->GetEarthCoordPosFromDetCoordPos(ipDC));  

  // check if the obtained track pass through the earth_radius.
  I3Position entCE, exitCE;
  int success = EarthModelCalculator::GetIntersectionsWithSphere(
                 ipCE, dirCE,
                 steer->GetWorldRadius(), entCE, exitCE);
  if (!success) {
     log_error("Your track doesn't path through the defined simulation "
               " radius (fWorldRadius). ");
     return false;
  }

  // Now we get entrance and exit position of neutrino to the Earth 

  I3Position entEarthDC(earth->GetDetCoordPosFromEarthCoordPos(entCE));
  I3Position exitEarthDC(earth->GetDetCoordPosFromEarthCoordPos(exitCE));
  nu_ptr->SetPos(entEarthDC);

  // to the exit from earth
  double dist = (exitEarthDC - entEarthDC).Magnitude();
  nu_ptr->SetLength(dist);

  return true;

}

//_________________________________________________________________
I3Position SourceUtils::SelectRandomPos(ParticlePtr nu_ptr,
                                 I3RandomServicePtr random, 
                                 SteeringPtr steer)
{
  I3Surfaces::SamplingSurfacePtr surface = steer->GetDetectionSurface();

  I3Position modip = surface->SampleImpactPosition(nu_ptr->GetDir(), *random);

  return modip;
  
}
//_________________________________________________________________
void SourceUtils::FillInjectionGeomInfo(SteeringPtr steer, 
                                        ParticlePtr nu_ptr,
                                        I3MapStringDoublePtr wmap)
{
   //
   // following information will be needed for
   // weighter module. 
   //

   // get injection geom info
   double cylrad = steer->GetCylinderRadius();
   if (cylrad > 0) {
      // this is surface mode. For now, 
      // available option is cylinder only.
      (*wmap)["InjectionCylinderRadius"] = cylrad/I3Units::m;
   }

   double cyllen= steer->GetCylinderHeight();
   if (cyllen> 0) {
      // this is surface mode. For now, 
      // available option is cylinder only.
      (*wmap)["InjectionCylinderHeight"] = cylrad/I3Units::m;
   }

   double injectionRad = steer->GetInjectionRadius();
   if (injectionRad > 0) {
      // this is circle mode. 
      (*wmap)["InjectionCircleRadius"] = injectionRad/I3Units::m;
   }

   I3Position orig = steer->GetCylinderOrigin();
   (*wmap)["InjectionOrigin_x"] = orig.GetX()/I3Units::m;
   (*wmap)["InjectionOrigin_y"] = orig.GetY()/I3Units::m;
   (*wmap)["InjectionOrigin_z"] = orig.GetZ()/I3Units::m;

   // fill injection area in cm2
   I3Surfaces::SamplingSurfacePtr surface = steer->GetDetectionSurface();
   double injectionAreaSI = surface->GetArea(nu_ptr->GetDir());
   (*wmap)["InjectionAreaCGS"] = injectionAreaSI / I3Units::cm2;

}

//_________________________________________________________________
void SourceUtils::FillPointPrimaryGenInfo(ParticlePtr nu_ptr,
                                     SteeringPtr steer,
                                     double gammaIndex,
                                     double energyMinLog,
                                     double energyMaxLog,
                                     double zenith,
                                     double zenithSigma,
                                     double azimuth,
                                     double azimuthSigma,
                                     I3MapStringDoublePtr wmap)
{
   // set simmode
   (*wmap)["SimMode"] = steer->GetSimMode();

   // set injection geometory info
   FillInjectionGeomInfo(steer, nu_ptr, wmap);

   // set generation info
   (*wmap)["PowerLawIndex"] = gammaIndex;
   (*wmap)["MinEnergyLog"] = energyMinLog;
   (*wmap)["MaxEnergyLog"] = energyMaxLog;
   (*wmap)["PointSourceZenith"] = zenith;
   (*wmap)["PointSourceZenithSigma"] = zenithSigma;
   (*wmap)["PointSourceAzimuth"] = zenith;
   (*wmap)["PointSourceAzimuthSigma"] = zenithSigma;

   // use solidAngle 1 for point source. It looks weird,
   // but when we calculate OneWeight we have multiplied 1.0
   // for point source sim. 
   double solidAngle = 1.0; 
   (*wmap)["SolidAngle"] = solidAngle;

}

//_________________________________________________________________
void SourceUtils::FillDiffusePrimaryGenInfo(ParticlePtr nu_ptr,
                                     SteeringPtr steer,
                                     double gammaIndex,
                                     double energyMinLog,
                                     double energyMaxLog,
                                     double zenithMin,
                                     double zenithMax,
                                     double azimuthMin,
                                     double azimuthMax,
                                     I3MapStringDoublePtr wmap)
{
   // set simmode
   (*wmap)["SimMode"] = steer->GetSimMode();

   // set injection geometory info
   FillInjectionGeomInfo(steer, nu_ptr, wmap);

   // set generation info
   (*wmap)["PowerLawIndex"] = gammaIndex;
   (*wmap)["MinEnergyLog"] = energyMinLog;
   (*wmap)["MaxEnergyLog"] = energyMaxLog;
   (*wmap)["MinZenith"] = zenithMin;
   (*wmap)["MaxZenith"] = zenithMax;
   (*wmap)["MinAzimuth"] = azimuthMin;
   (*wmap)["MaxAzimuth"] = azimuthMax;

   double solidAngle = Calculator::CalcSolidAngleFactor(zenithMin, zenithMax, azimuthMin, azimuthMax);
   (*wmap)["SolidAngle"] = solidAngle;

}

//_________________________________________________________________
void SourceUtils::FillPrimaryInfo(ParticlePtr nu,
                                  I3MapStringDoublePtr wmap)
{
   // fill primary info
   (*wmap)["PrimaryNeutrinoEnergy"] = nu->GetEnergy();
   (*wmap)["PrimaryNeutrinoZenith"] = nu->GetDir().GetZenith();
   (*wmap)["PrimaryNeutrinoAzimuth"] = nu->GetDir().GetAzimuth();
   (*wmap)["PrimaryNeutrinoType"] = nu->GetType();

   // fill number of primary nu
   double nNu = (*wmap)["NInIceNus"];
   if (nNu == 0) {
      // no primary exists other than this nu. set 1.0.
      nu->GetInteractionInfoRef().SetNInIceNus(1.0);
      (*wmap)["NInIceNus"] = 1.0;
   } else {
      nu->GetInteractionInfoRef().SetNInIceNus(nNu);
      (*wmap)["NInIceNus"] = nNu;
   }
}

}


