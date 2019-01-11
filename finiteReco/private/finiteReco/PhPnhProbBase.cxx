/**
 * @brief implementation of the PhPnhProbBase class
 *
 * @file PhPnhProbBase.cxx
 * @version $Revision: 59053 $
 * @date $Date: 2009-10-20 12:58:54 +0200 (Tue, 20 Oct 2009) $
 * @author Sebastian Euler <sebastian.euler@icecube.wisc.edu>
 *
 * This class provides a general interface to derive a hit probability for a DOM at a certain position. The assumed track (I3Particle) and the position of the DOM are the only required information.
 */

#include "finiteReco/probability/PhPnhProbBase.h"

PhPnhProbBase::PhPnhProbBase(){}
PhPnhProbBase::~PhPnhProbBase(){}

double PhPnhProbBase::GetNoHitProb(const I3Particle& track, const I3Position& pos, const int& Nhit) const{
  return (1 - this->GetHitProb(track,pos,Nhit));
}

bool PhPnhProbBase::CherenkovCalc(const I3Particle& track,
                                  const I3Position& pos,
                                  I3Position& chpos,
                                  double& chtime,
                                  double& chdist,
                                  double& ori)const{
  I3Calculator::CherenkovCalc( track,
                               pos,
                               chpos,
                               chtime,
                               chdist,
                               ori );
  if(std::isnan(chtime)||std::isnan(ori)) return false;
  double dist = (chpos - track.GetPos()).Magnitude();
  if(track.GetShape()==I3Particle::ContainedTrack && dist > track.GetLength()) return false;
  
  double dir  = ( chpos.GetX() - track.GetPos().GetX() ) / track.GetDir().GetX();
  if(track.GetShape()==I3Particle::StoppingTrack && dir > 0) return false;
  if(track.GetShape()==I3Particle::StartingTrack && dir < 0) return false;
  if(track.GetShape()==I3Particle::ContainedTrack && dir < 0) return false;

  return true;
}
