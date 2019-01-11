/**
 * copyright  (C) 2005
 * the icecube collaboration
 *  $Id$
 *
 * @file I3CscdLlhMinimizer.h
 * @version $Revision: 1.1 $
 * @date $Date$
 * @author mggreene
 */

#ifndef I3_CSCD_LLH_MINIMIZER_H
#define I3_CSCD_LLH_MINIMIZER_H

// header files

#include "cscd-llh/minimizer/I3CscdLlhHit.h"
#include "cscd-llh/minimizer/I3CscdLlhResult.h"
#include "cscd-llh/pdf/I3CscdLlhAbsPdf.h"
#include "icetray/I3Logging.h"

#include <list>


/**
 * @brief The I3CscdLlhMinimizer class is a base class for classes that
 *            implement function minization algorithms.
 */
class I3CscdLlhMinimizer 
{
  public:
    static const int MAX_MINIMIZATION_PARAMS;
    static const int DEFAULT_MAX_CALLS;
    static const double DEFAULT_TOLERANCE;

    static const int STATUS_NULL;
    static const int STATUS_SUCCESS;
    static const int STATUS_MAX_FUNCTION_CALLS;

  public:
    // Constructors
    I3CscdLlhMinimizer();
    /**
     * @param maxParams The maximum number of minimization parameters.
     */
    I3CscdLlhMinimizer(int maxParams);

  protected:
    /**
     * Initialize.  Called by the constructors.
     *
     * @param maxParams The maximum number of minimization parameters.
     * @return true if successful.
     */
    virtual void Init(int maxParams);

  public:
    // Destructor
    virtual ~I3CscdLlhMinimizer();

    /**
     * Get the maximum number minimization parameters.
     *
     * @return the maximum number of free parameters.
     */
    virtual int GetMaxParams() 
    { 
      return maxParams_;
    }

    /**
     * Get the number of (fixed + free) minimization parameters.
     *
     * @return the number of parameters.
     */
    virtual int GetNumParams() 
    {
      return numParams_;
    }

    /**
     * Get the number of free minimization parameters.
     *
     * @return the number of free parameters.
     */
    virtual int GetNumFreeParams() 
    {
      return numFreeParams_;
    }

    /**
     * Set the maximum number of function calls after which the
     * calculation will be stopped even if it has not yet converged.
     * 
     * @param maxCalls The maximum number of function calls.
     */
    void SetMaxCalls(int maxCalls) 
    {
      maxCalls_ = maxCalls;
    }

    /**
     * Set the required tolerance on the function value 
     * at the minumum.
     * 
     * @param tolerance The tolerance.
     */
    void SetTolerance(double tolerance) 
    {
      tolerance_ = tolerance;
    }

    /**
     * Initialize a parameter.
     * 
     * @param idx The parameter index.
     * @param stepSize Starting step size or uncertainty.
     * @param lowerLimit Lower physical limit on the parameter.
     * @param upperLimit Upper physical limit on the parameter.
     * @param fix Fix the parameter if true, leave it free if false.
     * 
     * @return true if successful.
     */
    virtual bool InitParam(int idx, double stepSize,
      double lowerLimit, double upperLimit, bool fix);

    /**
     * Set a seed value for a parameter.
     *
     * @param idx Parameter index.
     * @param name Parameter name.
     * @param seed Starting value.
     *
     * @return true if successful.
     */
    virtual bool SetSeed(int idx, std::string name, double seed);

    /**
     * Perform the actual minimization.
     *
     * @param hits The data: a list of OM hits.
     * @param pdf The PDF.
     *
     * @return true if successful.
     */
    virtual bool Minimize(std::list<I3CscdLlhHitPtr>* hits, I3CscdLlhAbsPdfPtr pdf);

    /**
     * @return the number of function calls.
     */
    virtual int GetFunctionCalls() 
    {
      return functionCalls_;
    }

    /**
     * @return the status.
     * 0: Minimization succeeded.
     * 1: Exceeded max function calls.
     */
    virtual int GetStatus() 
    {
      return status_;
    }

    /**
     * Get the result of the minimization..
     *
     * @return the result
     */
    virtual I3CscdLlhResultPtr GetResult()
    {
      return result_;
    }

  protected:
    static std::list<I3CscdLlhHitPtr>* hits_;
    static I3CscdLlhAbsPdfPtr pdf_;

    I3CscdLlhResultPtr result_;

    int maxParams_;
    int numParams_;
    int numFreeParams_;
    int maxCalls_;
    double tolerance_;

    double* stepSize_;
    double* lowerLimit_;
    double* upperLimit_;
    bool* fixParam_;
    double* seed_;

    int functionCalls_;
    int status_;

  SET_LOGGER("I3CscdLlhMinimizer");

}; // class I3CscdLlhMinimizer

typedef boost::shared_ptr<I3CscdLlhMinimizer> I3CscdLlhMinimizerPtr;
#endif 
