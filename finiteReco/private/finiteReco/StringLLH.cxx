/**
 * @brief implementation of the StringLLH class
 *
 * @file StringLLH.cxx
 * @version $Revision$
 * @date $Date$
 * @author Sebastian Euler <sebastian.euler@icecube.wisc.edu>
 *
 * This class accounts for hard local coincidences in the hit probabilities. For more details about the implementation see <https://wiki.icecube.wisc.edu/index.php/FiniteReco.StringLLH>
 */

#include <iostream>
#include <cmath>

#include "icetray/I3Frame.h"
#include "finiteReco/StringLLH.h"

StringLLH::StringLLH(const int& stringNr):
  numOM_(0),
  typeOM_(0),
  brokenOM_(false),
  logProb_(1),
  probLast_(1),
  probChainNO_(1),
  probChainYES_(0),
  stringNr_(stringNr)
{}

StringLLH::StringLLH():
  numOM_(0),
  typeOM_(0),
  brokenOM_(false),
  logProb_(1),
  probLast_(1),
  probChainNO_(1),
  probChainYES_(0),
  stringNr_(-100)
{}

StringLLH::~StringLLH(){}

void StringLLH::AddOM(double probHit,bool hit, const bool& broken){
  if(broken){
    probHit = 0; 
    hit = false;
  }
  if(hit){
    logProb_ *= probLast_;
    logProb_ *= (probChainNO_ + probChainYES_);
    typeOM_ = 1;
    probChainNO_  = 1;
    probChainYES_ = 0;
  }
  else if (typeOM_ == 1){
    logProb_ *= probLast_;
    --typeOM_;
    probChainNO_  = 1;
    probChainYES_ = 0;
  }
  else if (typeOM_ == 0){
    logProb_ *= probLast_;
    --typeOM_;
    probChainNO_  = 1;
    probChainYES_ = 0;
  }
  else{
    double holdProb = probChainNO_;
    probChainNO_  = (probChainNO_ + probChainYES_) * probLast_;
    probChainYES_ =  holdProb * (1 - probLast_);
  }
  ++numOM_;
  brokenOM_ = broken;
  probLast_ = hit ? probHit : 1-probHit;
}

double StringLLH::GetProb(){
  double logAll;
  if(typeOM_ != -1) logAll = logProb_ * probLast_;
  else{
    double probNO_  = (probChainNO_ + probChainYES_) * probLast_;
    double probYES_ =  probChainNO_ * (1 - probLast_);
    logAll = logProb_ * (probNO_ + probYES_);
  }
  return log(logAll);
}

void StringLLH::NewString(const int& stringNr){
  numOM_ = 0;
  typeOM_ = 0;
  brokenOM_ = false;
  logProb_ = 1;
  probLast_ = 1;
  probChainNO_ = 1;
  probChainYES_ = 0;

  stringNr_ = stringNr;
}

double StringLLH::GetStringNr(){
  return stringNr_;
}


