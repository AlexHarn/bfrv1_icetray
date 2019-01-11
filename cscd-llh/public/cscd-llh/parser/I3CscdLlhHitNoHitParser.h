/**
 * copyright  (C) 2005
 * the icecube collaboration
 *  $Id$
 *
 * @file I3CscdLlhHitNoHitParser.h
 * @version $Revision: $
 * @date $Date$
 * @author mggreene
 */


#ifndef I3_CSCD_LLH_HIT_NO_HIT_PARSER
#define I3_CSCD_LLH_HIT_NO_HIT_PARSER

/**
 * @brief I3CscdLlhHitNoHitParser extracts parameters from cscd-llh steering files.
 */

#include "cscd-llh/parser/I3CscdLlhAbsParser.h"
//#include "icetray/services/I3Logging.h"

//class I3ModuleImpl;

class I3CscdLlhHitNoHitParser : public I3CscdLlhAbsParser 
{
  public:

    // Constructor
  //  I3CscdLlhHitNoHitParser(I3ModuleImplPtr impl) : I3CscdLlhAbsParser(impl) {}
    I3CscdLlhHitNoHitParser(I3Configuration& config) :
      I3CscdLlhAbsParser(config) {}
    
    // Destructor
    virtual ~I3CscdLlhHitNoHitParser() {}

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

   // Hit/No-hit options
    double optHitNoHitNorm_;
    double optHitNoHitLambdaAttn_;
    double optHitNoHitNoise_;
    double optHitNoHitDistCutoff_;
    double optHitNoHitDead_;
    double optHitNoHitSmallProb_;

  SET_LOGGER("I3CscdLlhHitNoHitParser");
};

typedef boost::shared_ptr<I3CscdLlhHitNoHitParser> I3CscdLlhHitNoHitParserPtr;
#endif
