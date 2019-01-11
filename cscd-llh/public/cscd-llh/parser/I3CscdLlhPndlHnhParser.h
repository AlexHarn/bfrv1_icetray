/**
 * copyright  (C) 2005
 * the icecube collaboration
 *  $Id$
 *
 * @file I3CscdLlhPndlHnhParser.h
 * @version $Revision: $
 * @date $Date$
 * @author mggreene
 */


#ifndef I3_CSCD_LLH_PNDL_HNH_PARSER
#define I3_CSCD_LLH_PNDL_HNH_PARSER

/**
 * @brief I3CscdLlhPndlHnhParser extracts parameters from cscd-llh steering files.
 */

#include "cscd-llh/parser/I3CscdLlhAbsParser.h"
//#include "icetray/services/I3Logging.h"

//class I3ModuleImpl;

class I3CscdLlhPndlHnhParser : public I3CscdLlhAbsParser 
{
  public:

    // Constructor
 //   I3CscdLlhPndlHnhParser(I3ModuleImplPtr impl) : I3CscdLlhAbsParser(impl) 
    I3CscdLlhPndlHnhParser(I3Configuration& config) :
      I3CscdLlhAbsParser(config) 
    {
    }
    
    // Destructor
    virtual ~I3CscdLlhPndlHnhParser() 
    {
    }

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

  private:

    // PndlHnh options
    double optPndlHnhWeight_;

    // UPandel options
    double optPandelSmallProb_;
    double optPandelTau_;
    double optPandelLambda_;
    double optPandelLambdaA_;
    double optPandelSigma_;
    double optPandelLightSpeed_;
    double optPandelMaxDist_;

    // Hit/No-hit options
    double optHitNoHitNorm_;
    double optHitNoHitLambdaAttn_;
    double optHitNoHitNoise_;
    double optHitNoHitDistCutoff_;
    double optHitNoHitDead_;
    double optHitNoHitSmallProb_;

  SET_LOGGER("I3CscdLlhPndlHnhParser");
};

typedef boost::shared_ptr<I3CscdLlhPndlHnhParser> I3CscdLlhPndlHnhParserPtr;
#endif
