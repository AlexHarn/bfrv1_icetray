/**
 * copyright  (C) 2005
 * the icecube collaboration
 *  $Id$
 *
 * @file I3CscdLlhModule.h
 * @version $Revision: 1.1 $
 * @date $Date$
 * @author mggreene
 * @author Doug Rutledge (maintenance and DC V2 Conversion)
 */
#ifndef I3_CSCD_LLH_MODULE_H
#define I3_CSCD_LLH_MODULE_H


#include "cscd-llh/I3CscdLlhFitter.h"
#include "cscd-llh/parser/I3CscdLlhGeneralParser.h"
#include "cscd-llh/parser/I3CscdLlhHitNoHitParser.h"
#include "cscd-llh/parser/I3CscdLlhPndlHnhParser.h"
#include "cscd-llh/parser/I3CscdLlhUPandelParser.h"
#include "cscd-llh/parser/I3CscdLlhHnhDirParser.h"

#include "icetray/I3ConditionalModule.h"

#include "icetray/OMKey.h"

/**
 * @brief IceTray module for executing a LLH reconstruction.
 */
class I3CscdLlhModule : public I3ConditionalModule
{
  public:

    // constructors and destructor

    /**
     * @param ctx The context with which this module is built.
     */
    I3CscdLlhModule(const I3Context& ctx);
  
    ~I3CscdLlhModule();

    // transitions
    /**
     * Load the configuration parameters.
     */
    void Configure();

    /**
     * The actual reconstruction is performed in the Physics method.
     */
    void Physics(I3FramePtr frame);

  private:

    /**
     * Tell the fitter to use a Brent minimizer.
     *
     * @return true if successful.
     */
    bool SetBrent();

    /**
     * Tell the fitter to use a Powell minimizer.
     *
     * @return true if successful.
     */
    bool SetPowell();

    /**
     * Tell the fitter to use a Simplex minimizer.
     *
     * @return true if successful.
     */
    bool SetSimplex();

    /**
     * Tell the fitter to use a UPandel PDF.
     *
     * @param mpe Set to true for a multi-photoelectron PDF.
     *
     * @return true if successful.
     */
    bool SetUPandel(bool mpe);

    /**
     * Tell the fitter to use a Hit/No-hit PDF.
     *
     * @param mpe Set to true for a multi-photoelectron PDF.
     *
     * @return true if successful.
     */
    bool SetHitNoHit(bool mpe);

    /**
     * Tell the fitter to use a PndlHnh PDF
     * (combined UPandelMpe and HitNoHitMpe).
     *
     * @return true if successful.
     */
    bool SetPndlHnh();

    /**
     * Tell the fitter to use a direction-reconstructing version of the
     * HitNoHit PDF.
     *
     * @return true if successful
     */
    bool SetHnhDir();

    bool SetUPandelPhx();
    /**
     * Get the first guess vertex from the ResultDict and pass it to the fitter.
     *
     * @return true if able to set the seed.
     */
    bool SetSeed(I3FramePtr frame);

    /**
     *
     * @ return 
     */
    bool AddRecoPulses(I3FramePtr frame);

    /**
     * Get RecoHitSeries hits from the OMSelectionDict,
     * using I3Analog2Hits,
     * and add them to the fitter.
     *
     * @return true if the hits were successfully added.
     */
    bool AddRecoHits(I3FramePtr frame);

    /**
     * Get a list of unhit OM's, and add them to the fitter
     * as hits with a zero hit count.
     *
     * @return true if the non-hits were successfully added.
     */
    bool AddNoHits(I3FramePtr frame);

    /**
     * Calculate the weight according to the AmpWeightPower.
     *
     * @return the weight.
     */
    double CalculateWeight(double amplitude);

    /**
     * Finish the analysis.
     */
    void Finish();

    /**
     * Convenience method to check if the current OM being considered is 
     * part of the vector of excluded OMs.
     *
     * @param omKey the OMKey to be considered.
     * @return true if the OM is in the user-defined vector of excluded OMKeys
     */
    bool SkipThisOM(const OMKey& omKey);

  private:

    // Unless noted otherwise, all units are meters, nanoseconds, GeV.
  
    std::string optInputType_;
    std::string optRecoSeries_;
    bool optFirstLE_;
    bool optSeedWithOrigin_;
    std::string optSeedKey_;
    int optMinHits_;
    double optAmpWeightPower_;
    std::string optResultName_;
    double optEnergySeed_;
    std::string optMinimizer_;
    std::string optPdf_;

    I3CscdLlhGeneralParserPtr parserGeneral_;
    I3CscdLlhHitNoHitParserPtr parserHitNoHit_;
    I3CscdLlhPndlHnhParserPtr parserPndlHnh_;
    I3CscdLlhUPandelParserPtr parserUPandel_;
    I3CscdLlhHnhDirParserPtr parserHnhDir_;
    I3CscdLlhFitter fitter_;

    std::vector<OMKey> excludedOMs_;

    int countEvents_;
    int countRecords_;
    int countAllOk_;

    // Add unhit OM's as hits with a zero hit count.
    bool addNoHits_;

  SET_LOGGER("I3CscdLlhModule");

};

#endif 
