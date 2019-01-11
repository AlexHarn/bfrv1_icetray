/**
 * copyright  (C) 2004
 * the icecube collaboration
 * $Id: I3CLast.cxx 42855 2008-02-29 22:52:50Z joanna $
 *
 * @file I3CLastModule.cxx
 * @version $Revision: 1.6 $
 * @date $Date: 2008-02-29 15:52:50 -0700 (Fri, 29 Feb 2008) $
 * @author toale (derived from TensorOfInertia code originally written by grullon)
 */

/*
 * The main module for generating a complete seed for cascade analysis
 * uses the tensor of inertia reconstruction to estimate a direction,
 * the COG to estimate a vertex position, and the cfirst-ish light-cone
 * algorithm to estimate the start time (though, unlike cfirst, this
 * code will always return a start time since, if the cfirst algorithm
 * fails, it will return the hit time of the first hit in the event.)
 *
 */

// headers for this reconstruction

#include "clast/I3CLastModule.h"
#include "clast/I3CLastCalculator.h"
#include "recclasses/I3CLastFitParams.h"
#include "icetray/I3TrayHeaders.h"
#include "icetray/I3Units.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/I3Position.h"
#include "icetray/OMKey.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include <iostream>
#include "phys-services/I3Cuts.h"
#include <boost/iterator/filter_iterator.hpp>
#include "dataclasses/I3Constants.h"

using namespace std;
using boost::filter_iterator;
using boost::make_filter_iterator;

I3_MODULE(I3CLastModule);


I3CLastModule::I3CLastModule(const I3Context& ctx) : I3ConditionalModule(ctx)
{

  AddOutBox("OutBox");

  moduleName_         = "clast";
  inputReadout_       = "RecoPulses";
  AmandaMode_         = false;
  minHits_            = 3;
  ampWeight_          = 1.0;
  ampOpt_             = 1;
  directHitRadius_    = 300.0*I3Units::m;
  directHitWindow_    = 200.0*I3Units::ns;
  directHitThreshold_ = 3;

  e0_ =  1.069;   // parameters for energy estimator of I3 hits
  e1_ =  1.842;
  e2_ = -0.492;
  e3_ =  0.075;
  e4_ =  0.0;
  e5_ =  0.0;

  a0_ =  1.448;   // parameters for energy estimator for AM hits (contained E-2 NuE - dataset 2107)
  a1_ = -1.312;
  a2_ =  0.9294;
  a3_ = -0.1696;
  a4_ =  0.001123;
  a5_ =  0.0;

  AddParameter("Name",
               "Name given to the fit the module adds to the event",
               moduleName_);

  AddParameter("InputReadout",
               "Data Readout to use for input",
               inputReadout_);

  AddParameter("AmandaMode",
               "Detector Type For Energy Estimate Flag - True: Amanda / False: IceCube [default=False]",
               AmandaMode_);

  // minimum hits needed for the TOI algorithm
  AddParameter("MinHits",
               "Minimum number of hits needed for reconstruction",
               minHits_);

  // the following parameter is needed for the Inertia Tensor Algorithm,
  // a default value is assigned to 1

  AddParameter("AmplitudeWeight","Weight applied to virtual masses in the\n"
               "reconstruction", ampWeight_);

  // Amplitude option: treatment of small pulses
  // a default value = 1 for IceCube and TWR-Amanda; 
  // value = 0 for pre-TWR Amanda (used before) 

  AddParameter("AmplitudeOption","Treatment of small pulses in amplitude calculation",
               ampOpt_);
 
  AddParameter("DirectHitRadius", "Maximum distance from COG to consider a hit as direct", directHitRadius_);
  AddParameter("DirectHitWindow", "Maximum time from vertex time to consider a hit as direct", directHitWindow_);
  AddParameter("DirectHitThreshold", "Number of direct hits required by primary algorithm", directHitThreshold_);

  //AddParameter("EnergyIntercept", "A in: log10(E) = A + B*log10(Q)", energyIntercept_);
  //AddParameter("EnergySlope", "B in: log10(E) = A + B*log10(Q)", energySlope_);

  AddParameter("EnergyParam0", "Zeroth Order Energy Coefficient", e0_);
  AddParameter("EnergyParam1", "First Order Energy Coefficient", e1_);
  AddParameter("EnergyParam2", "Second Order Energy Coefficient", e2_);
  AddParameter("EnergyParam3", "Third Order Energy Coefficient", e3_);
  AddParameter("EnergyParam4", "Fourth Order Energy Coefficient", e4_);
  AddParameter("EnergyParam5", "Fifth Order Energy Coefficient", e5_);

  AddParameter("AmEnergyParam0", "Zeroth Order Energy Coefficient", a0_);
  AddParameter("AmEnergyParam1", "First Order Energy Coefficient", a1_);
  AddParameter("AmEnergyParam2", "Second Order Energy Coefficient", a2_);
  AddParameter("AmEnergyParam3", "Third Order Energy Coefficient", a3_);
  AddParameter("AmEnergyParam4", "Fourth Order Energy Coefficient", a4_);
  AddParameter("AmEnergyParam5", "Fifth Order Energy Coefficient", a5_);

  return;

}


void I3CLastModule::Configure()
{

  GetParameter("Name",moduleName_);
  GetParameter("AmandaMode",AmandaMode_);
  GetParameter("InputReadout",inputReadout_);
  GetParameter("AmplitudeWeight",ampWeight_);
  GetParameter("MinHits",minHits_);
  GetParameter("AmplitudeOption",ampOpt_);

  GetParameter("DirectHitRadius", directHitRadius_);
  GetParameter("DirectHitWindow", directHitWindow_);
  GetParameter("DirectHitThreshold", directHitThreshold_);

  GetParameter("EnergyParam0" , e0_);
  GetParameter("EnergyParam1" , e1_);
  GetParameter("EnergyParam2" , e2_);
  GetParameter("EnergyParam3" , e3_);
  GetParameter("EnergyParam4" , e4_);
  GetParameter("EnergyParam5" , e5_);

  GetParameter("AmEnergyParam0" , a0_);
  GetParameter("AmEnergyParam1" , a1_);
  GetParameter("AmEnergyParam2" , a2_);
  GetParameter("AmEnergyParam3" , a3_);
  GetParameter("AmEnergyParam4" , a4_);
  GetParameter("AmEnergyParam5" , a5_);

  return;

}

// the only stream that is overloaded at this point is the Physics Stream

void I3CLastModule::Physics(I3FramePtr frame)
{
  
  log_debug("Entering I3CLastModule::Physics().");
  
  const I3Geometry &geometry = frame->Get<I3Geometry>();
  const I3OMGeoMap &om_geo   = geometry.omgeo;
  
  // The quantities I need to calculate are the Center of Gravity 
  // of the Hits, the Inertia Tensor, and the Eigenvalues & eigenvectors of 
  // the Inertia Tensor. For the matrix related manipulations, I will use
  // ROOT's Matrix classes.

  // Get the recopulse series from the frame
  I3RecoPulseSeriesMapConstPtr pulse_series=frame->Get<I3RecoPulseSeriesMapConstPtr>(inputReadout_);

  //Check that the pulses exist, if not, return failed fit status particle.
  if ( pulse_series==0 ) {
    
    I3ParticlePtr particle(new I3Particle);
    particle->SetFitStatus(I3Particle::GeneralFailure);
    frame->Put(moduleName_,particle);
    PushFrame(frame,"OutBox");
    log_info("Could not find pulses in the frame.  Leaving I3CLastModule::Physics().");
    return;

  //Check that pulses are above min hits, if not, return insufficient hit status particle
  } else if ((int) pulse_series->size() < minHits_) {

    I3ParticlePtr particle(new I3Particle);
    particle->SetFitStatus(I3Particle::InsufficientHits);
    frame->Put(moduleName_,particle);
    PushFrame(frame,"OutBox");
    log_info("Less hits than minhits parameter.  Leaving I3CLastModule::Physics().");
    return;

  }

  //Set up the calc object with options, this is called later
  I3CLastCalculator calc(ampWeight_       , ampOpt_ , 
                         directHitRadius_ , directHitWindow_, directHitThreshold_ , 
                         e0_, e1_, e2_, e3_, e4_, e5_ ,
                         a0_, a1_, a2_, a3_, a4_, a5_ );

  // first, the center of gravity (indep of calc)
  I3Position cogPosition=I3Cuts::COG(geometry, *pulse_series);

  // check for NaN, paranoias sake
  if (std::isnan(cogPosition.GetX())) {
    log_warn("Got a nan for COG x.  Setting it to 0 instead.  Something could be screwy with a DOM calibration!!");
    cogPosition.SetX(0.0);
  }

  if (std::isnan(cogPosition.GetY())) {
    log_warn("Got a nan for COG y.  Setting it to 0 instead.  Something could be screwy with a DOM calibration!!");
    cogPosition.SetY(0.0);
  }

  if (std::isnan(cogPosition.GetZ())) {
    log_warn("Got a nan for COG z.  Setting it to 0 instead.  Something could be screwy with a DOM calibration!!");
    cogPosition.SetZ(0.0);
  }

  //now, we can get the tensor of inertia.  
  I3Matrix toi=calc.CalculateTOI(pulse_series,om_geo,cogPosition);
  
  double evalratio=0.0;
  double eval2=0.0;
  double eval3=0.0;
  double mineval=0.0;
 
  // Here, we pass by reference the eigenvalue variables, and we calculate 
  // the eigenvalues of the tensor of inertia and the eigenvector.  
  std::vector<double> minevect=calc.diagonalizeTOI(toi,mineval,eval2,eval3,evalratio);

  // now there is a corresponding ambiguity in the direction of the track given
  // by the eigenvector. This needs to be corrected 
  int corr = calc.CorrectDirection(pulse_series, om_geo, cogPosition, minevect); 

  // Get the time
  double time = calc.CalculateTime(pulse_series, om_geo, cogPosition);

  // Get the energy
  double energy = -1.0;
  if ( ! AmandaMode_ ) {
    energy = calc.CalculateEnergy_From_I3Hits(pulse_series);
  } else {
    energy = calc.CalculateEnergy_From_AMHits(pulse_series);
  }

  // now with everything in place, time to use I3position, I3direction
  // and I3Particle and create a reconstructed single track.
  I3ParticlePtr particle(new I3Particle);
  
  if ( !particle ) {
    log_error("there is a problem with the \"particle\" pointer.");
  }
    
  I3Direction dparticle;
  
  // the direction of the cascade is given by the eigenvector corresponding
  // to the smallest eigenvalue. The correction is taken into account  
  dparticle = I3Direction(corr*minevect[0],corr*minevect[1],corr*minevect[2]);
  particle->SetPos(cogPosition);
  particle->SetTime(time);
  particle->SetDir(dparticle);
  particle->SetEnergy(energy);
  particle->SetType(I3Particle::unknown);
  particle->SetShape(I3Particle::Cascade);
  particle->SetFitStatus(I3Particle::OK);

  I3CLastFitParamsPtr params(new I3CLastFitParams);
  // set all the TOI particle parameters
  params->mineval=mineval;
  params->evalratio=evalratio;
  params->eval2=eval2;
  params->eval3=eval3;

  double phi=particle->GetAzimuth();
  
  if ( phi>2.0*I3Constants::pi ) {
     phi -= 2.0*I3Constants::pi;
  }
  particle->SetDir(particle->GetZenith(),phi);
  
  log_debug("CLast Fit Complete: COG vertex = (%.3f,%.3f,%.3f) - TOI dir = (%.3f,%.3f) [mineval=%.3f]- StartTime = %.2f",
            (particle->GetPos()).GetX(),(particle->GetPos()).GetY(),(particle->GetPos()).GetZ(),
            particle->GetZenith(),particle->GetAzimuth(),
            params->mineval,
            particle->GetTime());

  frame->Put(moduleName_,particle);
  frame->Put(moduleName_+"Params",params);
    
  PushFrame(frame,"OutBox");
  log_debug("Leaving I3CLastModule::Physics().");

  return;

}
