/**
 * @brief implementation of the PhPnhMCDist class
 *
 * @file PhPnhMCDist.cxx
 * @version $Revision: 59053 $
 * @date $Date: 2009-10-20 12:58:54 +0200 (Tue, 20 Oct 2009) $
 * @author Sebastian Euler <sebastian.euler@icecube.wisc.edu>
 *
 * This class derives from PhPnhProbBase. It derives the hit probability at a
 * certain distance from a given table. This table is averaged over energy and 
 * direction. The table has to be ASCII with the step width in meters in the first
 * line and the maximum distance in meters in the second line. All lines afterwards 
 * contain the probabilities for each bin with float precision.
 * (see <https://wiki.icecube.wisc.edu/index.php/FiniteReco.CalculationOfProbabilities>)
 */

#include "finiteReco/probability/PhPnhMCDist.h"

#include <cstdio>

PhPnhMCDist::PhPnhMCDist(const std::string& inputProbFile_){
  FILE *probFile;
  probFile = fopen(inputProbFile_.c_str(),"r");
  if(! probFile){
    log_fatal("Could not open probability table.");
  }
  if(fscanf(probFile,"%f\n",&stepSize_)!=1){
    log_error("Error at reading the probability table.");
  }
  if(fscanf(probFile,"%f\n",&maxDist_)!=1){
    log_error("Error at reading the probability table.");
  }
  float fprob;
  for(;fscanf(probFile,"%f\n",&fprob)!=EOF;){
    prob_.push_back(fprob);
  }
  fclose(probFile);
}

PhPnhMCDist::~PhPnhMCDist(){}

//double PhPnhMCDist::GetHitProb(const I3Particle& track, const I3Position& pos, const int& Nhit) const{
double PhPnhMCDist::GetHitProb(const I3Particle& track, const I3OMGeo& omgeo, const int& Nhit) const{
  
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
  if(!isOK) return 0.;
  return GetHitProb(chDist,ori,track.GetEnergy());
}

double PhPnhMCDist::GetHitProb(const double& chdist,const double& ori, const double& energy, const int& Nhit) const{
  int pos = (int)(chdist / stepSize_);
  double phys_prob = 0.;
  if (chdist < maxDist_) phys_prob = prob_.at(pos);
  return phys_prob;
}

PhPnhMCDist::PhPnhMCDist(){}
