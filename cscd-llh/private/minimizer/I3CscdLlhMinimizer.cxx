/**
 * copyright  (C) 2005
 * the icecube collaboration
 *  $Id$
 *
 * @file I3CscdLlhMinimizer.cxx
 * @version $Revision: 1.1 $
 * @date $Date$
 * @author mggreene
 */

#include "cscd-llh/minimizer/I3CscdLlhHit.h"
#include "cscd-llh/minimizer/I3CscdLlhMinimizer.h"

#include <iostream>

using namespace std;


const int I3CscdLlhMinimizer::MAX_MINIMIZATION_PARAMS = 15;
const int I3CscdLlhMinimizer::DEFAULT_MAX_CALLS = 50000;
const double I3CscdLlhMinimizer::DEFAULT_TOLERANCE = 0.01;

const int I3CscdLlhMinimizer::STATUS_NULL = -1;
const int I3CscdLlhMinimizer::STATUS_SUCCESS = 0;
const int I3CscdLlhMinimizer::STATUS_MAX_FUNCTION_CALLS = 1;

list<I3CscdLlhHitPtr>* I3CscdLlhMinimizer::hits_ = 0;
I3CscdLlhAbsPdfPtr I3CscdLlhMinimizer::pdf_;

/* ****************************************************** */
/* constructors                                           */
/* ****************************************************** */
I3CscdLlhMinimizer::I3CscdLlhMinimizer() 
{ 
  log_debug("Enter I3CscdLlhMinimizer::I3CscdLlhMinimizer()");
  Init(MAX_MINIMIZATION_PARAMS);
  log_debug("Exit I3CscdLlhMinimizer::I3CscdLlhMinimizer()");
}

I3CscdLlhMinimizer::I3CscdLlhMinimizer(int maxParams) 
{
  log_debug("Enter I3CscdLlhMinimizer::I3CscdLlhMinimizer(int)");
  Init(maxParams);
  log_debug("Exit I3CscdLlhMinimizer::I3CscdLlhMinimizer(int)");
}

/* ****************************************************** */
/* Init                                                   */
/* ****************************************************** */
void I3CscdLlhMinimizer::Init(int maxParams) 
{
  log_debug("Entering I3CscdLlhMinimizer::Init()");
 
  maxParams_ = maxParams;
  numParams_ = 0;
  numFreeParams_ = 0;
  maxCalls_ = DEFAULT_MAX_CALLS;
  tolerance_ = DEFAULT_TOLERANCE;

  stepSize_ = new double[maxParams];
  lowerLimit_ = new double[maxParams];
  upperLimit_ = new double[maxParams];
  fixParam_ = new bool[maxParams];
  seed_ = new double[maxParams];

  for (int i=0; i<maxParams; i++) 
  {
    stepSize_[i] = NAN;
    lowerLimit_[i] = 0.0;
    upperLimit_[i] = 0.0;
    fixParam_[i] = false;
    seed_[i] = NAN;
  }

  result_ = I3CscdLlhResultPtr(new I3CscdLlhResult());
 
  log_debug("Exiting I3CscdLlhMinimizer::Init()");
  return;
} // end Init

/* ****************************************************** */
/* destructor                                             */
/* ****************************************************** */ 
I3CscdLlhMinimizer::~I3CscdLlhMinimizer() 
{
  delete[] stepSize_;
  delete[] lowerLimit_;
  delete[] upperLimit_;
  delete[] fixParam_;
  delete[] seed_;
}

/* ****************************************************** */
/* InitParam: Initialize a parameter.                     */
/* ****************************************************** */ 
bool I3CscdLlhMinimizer::InitParam(int idx, double stepSize,
  double lowerLimit, double upperLimit, bool fix) 
{
  log_trace("Enter I3CscdLlhMinimizer::InitParam()");

  if (idx >= maxParams_) 
  {
    log_error("Parameter index is to large [%d >= %d]", idx, maxParams_);
  }
  
  stepSize_[idx] = stepSize;
  lowerLimit_[idx] = lowerLimit;
  upperLimit_[idx] = upperLimit;

  fixParam_[idx] = fix;

  if (!fix)
    numFreeParams_++;

  numParams_++;

  log_trace("Exit I3CscdLlhMinimizer::InitParam()");
  return true;
} // end InitParam

/* ****************************************************** */
/* SetSeed                                                */
/* ****************************************************** */
bool I3CscdLlhMinimizer::SetSeed(int idx, string name, double seed) 
{
  log_debug("Entering I3CscdLlhMiminizer::SetSeed()");

  if (idx >= maxParams_) 
  {
    log_error("Parameter index is to large [%d >= %d]", idx, maxParams_);
  }
  
  seed_[idx] = seed;
  log_debug("Seed parameter %d set: %f",idx,seed);

  log_debug("Exiting I3CscdLlhMinimizer::SetSeed()");
  return true; 
} // end SetSeed

/* ****************************************************** */
/* Minimize: Perform the actual minimization.             */
/* ****************************************************** */ 
bool I3CscdLlhMinimizer::Minimize(list<I3CscdLlhHitPtr>* hits,
  I3CscdLlhAbsPdfPtr pdf) 
{
  log_debug("Entering I3CscdLlhMinimizer::Minimize()");

  functionCalls_ = 0;
  status_ = STATUS_NULL;

  if (numFreeParams_ < 1)
    log_fatal("There are no free parameters!");

  // Need to Set pdf_ every time Minimize is called.
  // The reason is, there could be more than one Minimizer
  // (e.g., if there is more than one module) but there is
  // only one pdf_ (it's static!!!)

  hits_ = hits;
  pdf_ = pdf;

  result_->Clear();
  if (!pdf) return false;

  log_debug("Exiting I3CscdLlhMinimizer::Minimize()");
  return true;
} // end Minimize
