/**
 * @brief implementation of the PhPnhPhotorec class
 *
 * @file PhPnhPhotorec.cxx
 * @version $Revision: 52296 $
 * @date $Date: 2009-02-03 16:30:36 +0100 (Tue, 03 Feb 2009) $
 * @author Sebastian Euler <sebastian.euler@icecube.wisc.edu>
 *
 * This class derives from PhPnhProbBase. It calculates the hit probability at the given position by a call to the photonics tables. A corresponding pointer must be provided. Photorec requires an energy to determine the probability. If no energy is provided with the track, 1 TeV is assumed. (see <https://wiki.icecube.wisc.edu/index.php/FiniteReco.CalculationOfProbabilities>)
 */

#include "finiteReco/probability/PhPnhPhotorec.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/I3Constants.h"

#include <boost/math/distributions/poisson.hpp>

double PhPnhPhotorec::GetHitProb(const I3Particle& track, const I3Position& pos, const int& Nhit) const{ 
  double length = track.GetLength()/I3Units::m;
  double energy = track.GetEnergy()/I3Units::GeV;
  double x = track.GetPos().GetX()/I3Units::m;
  double y = track.GetPos().GetY()/I3Units::m;
  double z = track.GetPos().GetZ()/I3Units::m;
  
  // set energy if missing
  if(!std::isnormal(energy) || energy <= 0){
    log_debug("Photorec needs an energy for the calculation. Using minimum: 1 TeV");
    energy = 1.*I3Units::TeV;
  }
  // correct finiteTracks to photonics definition
  I3Particle::ParticleShape shape = track.GetShape(); 
  if(onlyInfiniteTables_){
    if(shape!=I3Particle::InfiniteTrack) log_fatal("You have selected only infinite tracks");
    else{
      length = -1;
    }
  }
  else{
    if (shape == I3Particle::StartingTrack){
      if (!(length > 0.)){
        length = finiteDefaultLength_/I3Units::m;
      }
    } else if (shape == I3Particle::StoppingTrack){
      if (!(length > 0.)){
        length = finiteDefaultLength_/I3Units::m;
      }
      x -= length * track.GetDir().GetX();
      y -= length * track.GetDir().GetY();
      z -= length * track.GetDir().GetZ();
    } else if (shape == I3Particle::ContainedTrack){
      if (!(length > 0.)){
        log_warn( "got a 'contained track' with an undefined length!" );
        length = finiteDefaultLength_/I3Units::m;
      }
    } else if (shape == I3Particle::InfiniteTrack){
      length = finiteDefaultLength_/I3Units::m;
      x -= length * track.GetDir().GetX();
      y -= length * track.GetDir().GetY();
      z -= length * track.GetDir().GetZ();
      
      length = 2*finiteDefaultLength_/I3Units::m;
    } else {
      log_fatal("You have to choose a different HypothesisType");
    }
  }

  double meanPEs = -1;
  double emissionPointDistance =-1;
  double geoTime = -1;

  PhotorecPtr_->SelectModuleCoordinates(pos.GetX()/I3Units::m,pos.GetY()/I3Units::m,pos.GetZ()/I3Units::m); 
  PhotorecPtr_->SelectSource(meanPEs,
                             emissionPointDistance,
                             geoTime,
                             PhotonicsSource(x, y, z,
                                             track.GetDir().GetZenith()/I3Units::degree,
                                             track.GetDir().GetAzimuth()/I3Units::degree,
                                             track.GetSpeed() / I3Constants::c,
                                             length, energy, 0));   // 0 means type is muon

  if(meanPEs < 0 || (!std::isnormal(meanPEs) && meanPEs!=0)){
    meanPEs = 0.;
    log_warn("Got no average PEs from table!");
  }
  
  double hitProb;
  if(Nhit < 0) hitProb = 1-exp(-meanPEs);
  else {
    if(meanPEs == 0.){
      return Nhit>0 ? 0.:1.;
    }
    boost::math::poisson_distribution<double> poissonDist(meanPEs);
    hitProb = boost::math::pdf(poissonDist, Nhit);
  }
  
  return hitProb;
} 
