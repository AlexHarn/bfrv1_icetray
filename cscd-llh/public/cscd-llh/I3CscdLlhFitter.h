/**
 * copyright  (C) 2005
 * the icecube collaboration
 *  $Id$
 *
 * @file I3CscdLlhFitter.h
 * @version $Revision: 1.1 $
 * @date $Date$
 * @author mggreene
 */

#ifndef I3_CSCD_LLH_FITTER_H
#define I3_CSCD_LLH_FITTER_H

// header files
#include "recclasses/I3CscdLlhFitParams.h"
#include "cscd-llh/minimizer/I3CscdLlhMinimizer.h"
#include "cscd-llh/parser/I3CscdLlhAbsParser.h"

#include "icetray/OMKey.h"
#include "icetray/I3Logging.h"
#include "dataclasses/physics/I3Particle.h"

#include <list>

struct I3CscdLlhHit;

/**
 * @brief The I3CscdLlhFitter class performs log likelihood reconstruction of cascades.
 * To use: Create I3CscdLlhFitter object, set parameters,
 * add hits with addHit(), call fit(),
 * then get results with getCascade().  Call reset() to
 * use the fitter more than once.
 */
class I3CscdLlhFitter 
{

  public:
    static const int MINIMIZER_NULL;
    static const int MINIMIZER_BRENT;
    static const int MINIMIZER_POWELL;
    static const int MINIMIZER_SIMPLEX;
    static const int MINIMIZER_NAG;
    static const int MINIMIZER_GRAPHICAL;

    static const int PDF_NULL;
    static const int PDF_UPANDEL;
    static const int PDF_UPANDEL_MPE;
    static const int PDF_HIT_NO_HIT;
    static const int PDF_HIT_NO_HIT_MPE;
    static const int PDF_PNDL_HNH;
    static const int PDF_HNH_DIR;
    static const int PDF_PHOTOREC;
    static const int PDF_FULL_E_RECO_PHOTONICS;

  public:
    I3CscdLlhFitter();
    
    // Destructor
    virtual ~I3CscdLlhFitter()
    {}

    /**
     * Remove hits and clear values of all member variables.
     *
     */
    void Clear();

    /**
     * Set the Minimizer.
     *
     * @param code A defined constant that specifies the minimizer.
     *
     * @return true if successful.
     */
    bool SetMinimizer(int code);
 
    /**
     * Set the PDF.
     *
     * @param code A defined constant
     * that specifies the PDF to be used by the minimizer.
     *
     * @return true if successful.
     */
    bool SetPdf(int code);

    /**
     * Configure steering file parameters.
     *
     * @param parser An class that gets parameters from a steering file.
     *
     * @return true iff successful.
     */
    bool Configure(I3CscdLlhAbsParserPtr parser);

    /**
     * Set the maximum number of function calls after which the
     * calculation will be stopped even if it has not yet converged.
     * 
     * @param maxCalls The maximum number of function calls.
     */
    void SetMaxCalls(int maxCalls);

    /**
     * Set the required tolerance on the function value 
     * at the minumum.
     * 
     * @param tolerance The tolerance.
     */
    void SetTolerance(double tolerance);

    /**
     * Set a constant value used by the PDF.
     *
     * @param id A predefined identifier for the constant.
     * @param value The value to be assigned to the constant.
     *
     * @return true if successful.
     */
    bool SetPdfConstant(int id, bool value)
    {return pdf_->SetConstant(id, value);}
    bool SetPdfConstant(int id, int value)
    {return pdf_->SetConstant(id, value);}
    bool SetPdfConstant(int id, double value)
    {return pdf_->SetConstant(id, value);}
    bool SetPdfConstant(int id, std::string value)
    {return pdf_->SetConstant(id, value);}

    /**
     * Initialize a parameter.
     *
     * @param paramName The parameter name.
     * @param stepSize Starting step size or uncertainty.
     * @param lowerLimit Lower physical limit on the parameter.
     * @param upperLimit Upper physical limit on the parameter.
     * @param fix Fix the parameter if true, leave it free if false..
     * 
     * @return true if successful.
     */
    bool InitParam(std::string paramName, double stepSize,
      double lowerLimit, double upperLimit, bool fix);

    /**
     * Set the initial value for a parameter.
     *
     * @param paramName The parameter name.
     * @param seed The seed value.
     *
     * @return true if successful.
     */
    bool SetSeed(std::string paramName, double seed);

    /**
     * Add an OM hit that contributes to the reconstruction.
     * Invalid hits will not be added.
     *
     * @param hit
     * @return true if hit is valid.
     */
    bool AddHit(I3CscdLlhHitPtr& hit);

    /**
     * Set the number of hit OM's.
     *
     * @param count The number of hit OM's.
     */
    void SetHitOmCount(int count) 
    {fitParams_->SetHitOmCount(count);}

    /**
     * Set the number of unhit OM's .
     *
     * @param count The number of OM's that were <I>not</I> hit.
     */
    void SetUnhitOmCount(int count) 
    {fitParams_->SetUnhitOmCount(count);}

    /**
     * Perform the actual fitting algorithm.
     *
     * @return true if algorithm succeeded.
     */
    bool Fit();

    /**
     * Get the results of the fit.
     *
     * @return The cascade.
     */
    I3ParticlePtr GetCascade();

    /**
     * Get the Fit Parameters of the fit.
     *
     * @return the fit params
     */
    I3CscdLlhFitParamsPtr GetFitParams();

   /**
    * Initialize the result with the results of a prior reconstruction,
    * as determined by a prior reconstruction of some sort. This module
    * must pass through the results of any prior reconstruction. 
    *
    * @param priorRecoResult the result of a 
    * previous reconstruction, that is used as a seed for the current fit.
    */
   void InitializeResult(const I3Particle& priorRecoResult);
   
   /**
    * Under certain circumstances, it is best to minimize the energy on
    * a logarithmic scale, rather than in a linear one. This is done if
    * the function is in a slowly varying region about the seed, for 
    * example.
    */ 
   void MinimizeInLogE();
  private:

    std::list<I3CscdLlhHitPtr> hits_;

    I3CscdLlhMinimizerPtr minimizer_;
    I3CscdLlhAbsPdfPtr pdf_;
    I3ParticlePtr resultingCascade_;
    I3CscdLlhFitParamsPtr fitParams_;
    bool minimizeInLogE_;

  SET_LOGGER("I3CscdLlhFitter");

}; // class I3CscdLlhFitter

typedef boost::shared_ptr<I3CscdLlhFitter> I3CscdLlhFitterPtr;
#endif 
