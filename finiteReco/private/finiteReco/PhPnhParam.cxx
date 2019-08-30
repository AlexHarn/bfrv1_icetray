/**
 * @brief implementation of the PhPnhParam class
 *
 * @file PhPnhParam.cxx
 * @version $Revision: 59053 $
 * @date $Date: 2009-10-20 12:58:54 +0200 (Tue, 20 Oct 2009) $
 * @author Sebastian Euler <sebastian.euler@icecube.wisc.edu>
 *
 * This class derives from PhPnhProbBase. It calculates the hit probability by an analytic function. This parametrization has two free parameters. As default the AMANDA best fit values are inserted. (see <https://wiki.icecube.wisc.edu/index.php/FiniteReco.CalculationOfProbabilities>)
 */

#include "finiteReco/probability/PhPnhParam.h"
#include "finiteReco/probability/PhPnhPhotorec.h"

#include <boost/math/distributions/poisson.hpp>


// The values defined here are the best fit values copied from the AMANDA muon track reconstruction paper given in meters
PhPnhParam::PhPnhParam():
  tau_(557*I3Units::m),
  l_a_(98.0*I3Units::m){}

PhPnhParam::PhPnhParam(double tau, const double& absorptionLength):
  tau_(tau),
  l_a_(absorptionLength){}

PhPnhParam::~PhPnhParam(){}

//double PhPnhParam::GetHitProb(const I3Particle& track, const I3Position& pos, const int& Nhit) const{
double PhPnhParam::GetHitProb(const I3Particle& track, const I3OMGeo& omgeo, const int& Nhit) const{
  I3Position chPos;
  double chTime;
  double ori;
  double chDist;
  bool isOK = this->CherenkovCalc(track,
                                  omgeo.position,
                                  chPos,
                                  chTime,
                                  chDist,
                                  ori);
  if(!isOK) return 0.0;
  return GetHitProb(chDist,ori,track.GetEnergy(),Nhit);
}

double PhPnhParam::GetHitProb(const double& chdist,const double& ori, const double& energy, const int& Nhit) const{

  const double effDist1 = 3.1;
  const double effDist2 = 3.9;
  const double effDist3 = 4.6;
  const double effDist4 = 0.84;
  const double lambda   = 33.3;
  
  double cs_ori = cos(ori);
  double perpdist = chdist *sqrt(1-1/I3Constants::n_ice_phase/I3Constants::n_ice_phase);
  double effdist = effDist1 - effDist2 *cs_ori+ effDist3 *cs_ori*cs_ori+ effDist4 *perpdist*I3Units::m;
  double mue = exp(-effdist/ l_a_*(1+l_a_/ lambda *log(1+(tau_*(I3Constants::c_ice)/l_a_))));
  
  if(mue == 0.){
    return Nhit>0 ? 0.:1.;
  }
  
  double hitProb;
  if(Nhit < 0) hitProb=1.-exp(-mue);
  else{
    boost::math::poisson_distribution<double> poissonDist(mue);
    hitProb = boost::math::pdf(poissonDist, Nhit);
  }
  
  return hitProb;
}
