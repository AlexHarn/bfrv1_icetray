/**
 * copyright (c) 2006
 * the IceCube collaboration
 * $Id$
 *
 * @file I3CscdLlhHnhDirParser.h
 * @version
 * @author Doug Rutledge
 * @date 08Feb2006
 */

#ifndef I3_CSCD_LLH_HNH_DIR_PARSER_H
#define I3_CSCD_LLH_HNH_DIR_PARSER_H

#include "cscd-llh/parser/I3CscdLlhAbsParser.h"
//#include "icetray/services/I3Logging.h"

class I3CscdLlhHnhDirParser : public I3CscdLlhAbsParser
{
  public:
/*    I3CscdLlhHnhDirParser(I3ModuleImplPtr& implementation) :
      I3CscdLlhAbsParser(implementation){}*/
    
    I3CscdLlhHnhDirParser(I3Configuration& config) :
      I3CscdLlhAbsParser(config){}
    virtual ~I3CscdLlhHnhDirParser(){}
  public:
    /*
     * Add parameters.
     */
    virtual void AddParameters();
  
    /*
     * Set the parameters of the probability density function that 
     * this is meant to configure
     */
    virtual bool Configure();

   private:
     double optLegendrePolyCoeff0_;
     double optLegendrePolyCoeff1_;
     double optLegendrePolyCoeff2_;

     // Hit/No-hit options
     double optHitNoHitNorm_;
     double optHitNoHitLambdaAttn_;
     double optHitNoHitNoise_;
     double optHitNoHitDistCutoff_;
     double optHitNoHitDead_;
     double optHitNoHitSmallProb_;

  SET_LOGGER("I3CscdLlhHnhDirParser");
};

typedef boost::shared_ptr<I3CscdLlhHnhDirParser> I3CscdLlhHnhDirParserPtr;
#endif
