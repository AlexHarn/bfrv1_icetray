/**
 * copyright  (C) 2005
 * the icecube collaboration
 *  $Id$
 *
 * @file I3CscdLlhUPandelParser.h
 * @version $Revision: $
 * @date $Date$
 * @author mggreene
 */


#ifndef I3_CSCD_LLH_U_PANDEL_PARSER
#define I3_CSCD_LLH_U_PANDEL_PARSER

/**
 * @brief I3CscdLlhUPandelParser extracts parameters from cscd-llh steering files.
 */

#include "cscd-llh/parser/I3CscdLlhAbsParser.h"
//#include "icetray/services/I3Logging.h"

//class I3ModuleImpl;

class I3CscdLlhUPandelParser : public I3CscdLlhAbsParser 
{
  public:

    // Constructor
    //I3CscdLlhUPandelParser(I3ModuleImplPtr impl) : I3CscdLlhAbsParser(impl) {}
    I3CscdLlhUPandelParser(I3Configuration& config) :
      I3CscdLlhAbsParser(config) {}
    
    // Destructor
    virtual ~I3CscdLlhUPandelParser() {}

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

  protected:

    // UPandel options
    double optPandelSmallProb_;
    double optPandelTau_;
    double optPandelLambda_;
    double optPandelLambdaA_;
    double optPandelSigma_;
    double optPandelLightSpeed_;
    double optPandelMaxDist_;

  SET_LOGGER("I3CscdLlhUPandelParser");
};

typedef boost::shared_ptr<I3CscdLlhUPandelParser> I3CscdLlhUPandelParserPtr;
#endif
