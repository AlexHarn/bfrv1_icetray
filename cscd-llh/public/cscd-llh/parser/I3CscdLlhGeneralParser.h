/**
 * copyright  (C) 2005
 * the icecube collaboration
 *  $Id$
 *
 * @file I3CscdLlhGeneralParser.h
 * @version $Revision: $
 * @date $Date$
 * @author mggreene
 */


#ifndef I3_CSCD_LLH_GENERAL_PARSER
#define I3_CSCD_LLH_GENERAL_PARSER

/**
 * @brief I3CscdLlhGeneralParser extracts parameters from cscd-llh 
 * steering files.
 */

#include "cscd-llh/parser/I3CscdLlhAbsParser.h"
//#include "icetray/services/I3Logging.h"

//class I3ModuleImpl;

class I3CscdLlhGeneralParser : public I3CscdLlhAbsParser 
{
public:

  // Constructor
  //I3CscdLlhGeneralParser(I3ModuleImplPtr impl) : I3CscdLlhAbsParser(impl) {}
  I3CscdLlhGeneralParser(I3Configuration& config) :
    I3CscdLlhAbsParser(config) {}
    
  // Destructor
  virtual ~I3CscdLlhGeneralParser() {}

public:

  /**
   * Add parameters.
   * 
   */
  virtual void AddParameters();

  /**
   * Get parameters from the steering file.
   * Parse parameter strings if necessary.
   * Tell the fitter what to do with the parameters.
   * 
   * @return true iff successful.
   */
  virtual bool Configure();

  bool UseParamT() {return useParamT_;}
  bool UseParamX() {return useParamX_;}
  bool UseParamY() {return useParamY_;}
  bool UseParamZ() {return useParamZ_;}
  bool UseParamZenith() {return useParamZenith_;}
  bool UseParamAzimuth() {return useParamAzimuth_;}
  bool UseParamEnergy() {return useParamEnergy_;}

private:

  /**
   * Parse the steering file strings that specify
   * the step size (SS), lower limit (LL), upper limit (UL),
   * and "fix" for the PDF parameters.
   * Fix is "true" or "false", to fix the parameter or to let it be free.
   * The string format is "SS, LL, UL, fix".
   * Setting LL = UL = 0 sets no limits on the parameter.
   * Setting limits is not recommended, unless they are absolutely necessary.
   *
   * @return true if able to parse the parameter strings.
   */
  bool ParseParamStrings();

  /**
   * Parse a single steering file string that specifies
   * the step size, lower limit, upper limit, and "fix"
   * for the minimization parameters.
   *
   * @param paramName The parameter name.
   * @param steeringFile The steering file string.
   *
   * @return true if able to parse the parameter string.
   */
  bool ParseParamString(std::string paramName, std::string steeringFile);

private:

  // Minimization parameters
  int optMaxCalls_;
  double optTolerance_;

  std::string optParamT_;
  std::string optParamX_;
  std::string optParamY_;
  std::string optParamZ_;
  std::string optParamZenith_;
  std::string optParamAzimuth_;
  std::string optParamEnergy_;
  
  bool useParamT_;
  bool useParamX_;
  bool useParamY_;
  bool useParamZ_;
  bool useParamZenith_;
  bool useParamAzimuth_;
  bool useParamEnergy_;

  bool optMinimizeInLogE_;

  SET_LOGGER("I3CscdLlhGeneralParser");
};

typedef boost::shared_ptr<I3CscdLlhGeneralParser> I3CscdLlhGeneralParserPtr;
#endif
