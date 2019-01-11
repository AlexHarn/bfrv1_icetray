/**
 * copyright  (C) 2005
 * the icecube collaboration
 *  $Id$
 *
 * @file I3CscdLlhGeneralParser.cxx
 * @version $Revision: $
 * @date $Date$
 * @author mggreene
 */

#include "cscd-llh/I3CscdLlhFitter.h"
#include "cscd-llh/parser/I3CscdLlhGeneralParser.h"

#include "dataclasses/I3Constants.h"

using namespace std;

/* ****************************************************** */
/* AddParameters                                          */
/* ****************************************************** */
void I3CscdLlhGeneralParser::AddParameters() {
  log_debug("Enter AddParameters().");

  useParamT_ = false;
  useParamX_ = false;
  useParamY_ = false;
  useParamZ_ = false;
  useParamZenith_ = false;
  useParamAzimuth_ = false;
  useParamEnergy_ = false;

  optMaxCalls_ = I3CscdLlhMinimizer::DEFAULT_MAX_CALLS;
  AddParameter("MaxCalls", 
    "Maximum number of function calls after which minimizer will stop,\n"
    "even if it has not yet converged.",
    optMaxCalls_);
    
  optTolerance_ = I3CscdLlhMinimizer::DEFAULT_TOLERANCE;
  AddParameter("Tolerance", 
    "Required tolerance on the function value at the minimum.",
    optTolerance_);

  optParamT_.clear();
  AddParameter("ParamT", 
    "Step size (SS), lower limit (LL), upper limit (UL), fix "
    "(true or false) for the vertex time.\n"
    "String format is \"SS, LL, UL, fix\".", optParamT_);

  optParamX_.clear();
  AddParameter("ParamX", "Step size (SS), lower limit (LL),"
    " upper limit (UL), fix (true or false) for the vertex x-coordinate.\n"
    "String format is \"SS, LL, UL, fix\".", optParamX_);

  optParamY_.clear();
  AddParameter("ParamY", "Step size (SS), lower limit (LL),"
    " upper limit (UL), fix (true or false) for the vertex y-coordinate.\n"
    "String format is \"SS, LL, UL, fix\".", optParamY_);

  optParamZ_.clear();
  AddParameter("ParamZ", "Step size (SS), lower limit (LL),"
    " upper limit (UL), fix (true or false) for the vertex z-coordinate.\n"
    "String format is \"SS, LL, UL, fix\".", optParamZ_);

  optParamZenith_.clear();
  AddParameter("ParamZenith", "Step size (SS), lower limit (LL),"
    " upper limit (UL), fix (true or false) for the cascade polar angle.\n"
    "String format is \"SS, LL, UL, fix\".", optParamZenith_);

  optParamAzimuth_.clear();
  AddParameter("ParamAzimuth", "Step size (SS), lower limit (LL), "
    "upper limit (UL), fix (true or false) for the cascade azimuthal angle.\n"
    "String format is \"SS, LL, UL, fix\".", optParamAzimuth_);

  optParamEnergy_.clear();
  AddParameter("ParamEnergy", "Step size (SS), lower limit (LL), "
    "upper limit (UL), fix (true or false) for the cascade energy.\n"
    "String format is \"SS, LL, UL, fix\".", optParamEnergy_);

  optMinimizeInLogE_ = false;
  AddParameter("MinimizeInLog(E)",
    "Minimizing in log(E) can improve the results when E varies slowly " 
    "across the range.",optMinimizeInLogE_);

  log_debug("Exit AddParameters().");
  return;
} // end AddParameters

/* ****************************************************** */
/* Configure                                              */
/* ****************************************************** */
bool I3CscdLlhGeneralParser::Configure()
{
  log_debug("Enter Configure().");

  GetParameter("MaxCalls", optMaxCalls_);
  GetParameter("Tolerance", optTolerance_);

  GetParameter("ParamT", optParamT_);
  GetParameter("ParamX", optParamX_);
  GetParameter("ParamY", optParamY_);
  GetParameter("ParamZ", optParamZ_);
  GetParameter("ParamZenith", optParamZenith_);
  GetParameter("ParamAzimuth", optParamAzimuth_);
  GetParameter("ParamEnergy", optParamEnergy_);
  GetParameter("MinimizeInLog(E)", optMinimizeInLogE_);

  useParamT_ = optParamT_.size() != 0;
  useParamX_ = optParamX_.size() != 0;
  useParamY_ = optParamY_.size() != 0;
  useParamZ_ = optParamZ_.size() != 0;
  useParamZenith_ = optParamZenith_.size() != 0;
  useParamAzimuth_ = optParamAzimuth_.size() != 0;
  useParamEnergy_ = optParamEnergy_.size() != 0;

  log_info("MaxCalls = %d", optMaxCalls_);
  log_info("Tolerance = %f", optTolerance_);

  log_info("ParamT = %s", optParamT_.c_str());
  log_info("ParamX = %s", optParamX_.c_str());
  log_info("ParamY = %s", optParamY_.c_str());
  log_info("ParamZ = %s", optParamZ_.c_str());
  log_info("ParamZenith = %s", optParamZenith_.c_str());
  log_info("ParamAzimuth = %s", optParamAzimuth_.c_str());
  log_info("ParamEnergy = %s", optParamEnergy_.c_str());

  fitter_->SetMaxCalls(optMaxCalls_);
  fitter_->SetTolerance(optTolerance_);

  if(optMinimizeInLogE_)
  {
    fitter_->MinimizeInLogE();
  }

  if (!ParseParamStrings()) 
  {
    return false;
  }

  log_debug("Exit Configure().");
  return true;
} // end Configure

/* ******************************************************************** */
/* ParseParamStrings                                                    */
/* ******************************************************************** */
bool I3CscdLlhGeneralParser::ParseParamStrings() 
{
  log_debug("Enter I3CscdLlhGeneralParser::ParseParamStrings()."); 

  if (!ParseParamString(string("t"), optParamT_)) 
  {
    return false;
  }

  if (!ParseParamString(string("x"), optParamX_)) 
  {
    return false;
  }

  if (!ParseParamString(string("y"), optParamY_)) 
  {
    return false;
  }

  if (!ParseParamString(string("z"), optParamZ_)) 
  {
    return false;
  }

  if (!ParseParamString(string("zenith"), optParamZenith_)) 
  {
    return false;
  }

  if (!ParseParamString(string("azimuth"), optParamAzimuth_)) 
  {
    return false;
  }

  if (!ParseParamString(string("energy"), optParamEnergy_)) 
  {
    return false;
  }

  log_debug("Exit I3CscdLlhGeneralParser::ParseParamStrings().");
  return true;
} // end ParseParamStrings

/* ******************************************************************** */
/* ParseParamString                                                     */
/*                                                                      */
/* Parse a steering file string that specifies                          */
/* the step size (SS), lower limit (LL), and upper limit (UL),          */
/* fix (true or false)                                                  */
/* for the minimization parameters.                                     */
/* The string format is "SS, LL, UL, fix".                              */
/* ******************************************************************** */
bool I3CscdLlhGeneralParser::ParseParamString(string paramName, 
  string steeringFile) 
{
  log_debug("Enter I3CscdLlhGeneralParser::ParseParamString()."); 

  if (steeringFile.size() == 0) return true;

  log_debug("Parse steering file parameter string [%s]", 
    steeringFile.c_str());

  string::size_type comma1 = 
    steeringFile.find(',', 0);
  if (comma1 == string::npos) 
  {
    log_fatal("Unable to Parse steering file parameter string [%s]", 
      steeringFile.c_str());
    return false;
  }

  string::size_type comma2 = steeringFile.find(',', comma1+1);
  if (comma2 == string::npos) 
  {
    log_fatal("Unable to Parse steering file parameter string [%s]", 
      steeringFile.c_str());
    return false;
  }

  string::size_type comma3 = steeringFile.find(',', comma2+1);
  if (comma3 == string::npos)
  {
    log_fatal("Unable to Parse steering file parameter string [%s]", 
      steeringFile.c_str());
    return false;
  }
  
  double stepSize = atof(steeringFile.substr(0, comma1).c_str());
  double lowerLimit = atof(steeringFile.substr(comma1 + 1, 
    (comma2 - comma1) - 1).c_str());
  double upperLimit = atof(steeringFile.substr(comma2 + 1, 
    (comma3 - comma2) - 1).c_str());
  string fix = steeringFile.substr(comma3+1).c_str();

  // Trim left
  string::size_type firstNonSpace = fix.find_first_not_of(' ');
  if (firstNonSpace != string::npos) 
  {
    fix = fix.erase(0, firstNonSpace);
  }
  // Trim right
  string::size_type lastNonSpace = fix.find_last_not_of(' ');
  if (lastNonSpace != string::npos) 
  {
    fix = fix.erase(lastNonSpace+1, fix.size()-(lastNonSpace+1));
  }

  bool fixParam = false;
  if (fix == "false") 
  {
    fixParam = false;
  }
  else if (fix == "true") 
  {
    fixParam = true;
  }
  else 
  {
    log_fatal("Unable to Parse steering file \"fix\" string [%s]", 
      fix.c_str());
    return false;
  } 

  if (!fitter_->InitParam(paramName, stepSize, 
    lowerLimit, upperLimit, fixParam) ) 
  {
    log_fatal("Unable to Initialize parameter %s",paramName.c_str());  
    return false;
  }

  log_debug(
    "Initialize parameter %s: SS = %f, LL = %f, UL = %f, fix = %s",
    paramName.c_str(), stepSize, lowerLimit, upperLimit, fix.c_str());

  log_debug("Exit I3CscdLlhGeneralParser::ParseParamString().");
  return true;
} // end ParseParamString
