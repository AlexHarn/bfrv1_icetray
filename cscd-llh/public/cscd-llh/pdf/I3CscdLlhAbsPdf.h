/**
 * copyright  (C) 2005
 * the icecube collaboration
 *  $Id$
 *
 * @file I3CscdLlhAbsPdf.h
 * @version $Revision: 1.1 $
 * @date $Date$
 * @author mggreene
 */


#ifndef I3_CSCD_LLH_ABS_PDF
#define I3_CSCD_LLH_ABS_PDF

#include "cscd-llh/minimizer/I3CscdLlhHit.h"


/**
 * @brief I3CscdLlhAbsPdf is an abstract base class for PDF's.
 */
class I3CscdLlhAbsPdf 
{

  public:

    // Constructor
    I3CscdLlhAbsPdf() 
    {
      //declare this as false by default, and let the user change this 
      //in the steering file.
      minimizeInLogE_ = false;
  
      //this next one is not to be set or unset by the user, but is to be 
      //overridden in the constructors of the individual pdfs
      useNoHits_ = false;
    }
    
    // Destructor
    virtual ~I3CscdLlhAbsPdf() {}

  public:
    
    /**
     * This method handles any additional configuration that needs to be done,
     * e.g. setting the photonics driver file.
     */
    virtual void Configure()
    {
      log_error("Error! Configure called on pdf when no additional "
        "configuration is needed");
    }

    /**
     * Calculate the PDF.
     * 
     * @param hit The OM hit data.
     * @param param Pointer to an array containing the minimization parameters.
     * @param value The result.
     *
     */
    virtual void Evaluate(const I3CscdLlhHitPtr& hit, const double* param,
      double& value) const = 0;

    /**
     * Calculate the gradient of the PDF with respect to 
     * the vertex parameters.
     * 
     * @param hit The OM hit data.
     * @param param Pointer to an array containing
     * the cascade vertex time, position, direction, and energy.
     * param[i] is (t, x, y, z, theta, phi, energy)
     * for i = (0, 1, 2, 3, 4, 5, 6).
     * @param gradient is the result, that is,
     * gradient[i] is the derivative of the PDF with respect to param[i].
     */
    virtual void CalculateGradient(const I3CscdLlhHitPtr& hit, 
      const double* param, double* gradient) const = 0;

    /**
     * Get the number of minimization parameters (fixed and free.)
     *
     * @return the number of paramters.
     */
    virtual int GetNumParams() const = 0;

    /**
     * Get the index of a minimization parameter.
     *
     * @param name The name of the parameter.
     *
     * @return the index of the parameter in the parameter array,
     * if name is not valid, return -1.
     */
    virtual int GetParamIndex(const std::string name) const = 0;

    /**
     * Get the name of a minimization parameter.
     *
     * @param index The parameter index.
     *
     * @return the name of the parameter,
     * if index is not valid, return an empty string.
     */
    virtual std::string GetParamName(const int index) const = 0;

    /**
     * Set a constant value.
     * @param id A predefined identifier for the constant.
     * @param value The value to be assigned to the constant.
     * 
     * @return true if successful.
     */
    virtual bool SetConstant(int id, double value) = 0;
    virtual bool SetConstant(int id, int value) = 0;
    virtual bool SetConstant(int id, bool value) = 0;
    virtual bool SetConstant(int id, std::string value) = 0;

    void SetMinimizeInLogE(bool minimizeInLogE)
    {
      minimizeInLogE_ = minimizeInLogE; /* BUG FIX: remove extra _ from the RHS variable */
    }//end SetMinimizeInLogE
 
    bool GetUsesNoHits()
    {
      return useNoHits_;
    }

  protected:
    bool minimizeInLogE_;

    //This is NOT to be set by the user. It will be used by the pdfs to
    //switch on behaviour depending on wether the pdf uses unhit OMs.
    //An example of how this is used is in the fitter, when the reduced
    //llh is calculated. This will be set as false by default in the
    //constructor for this base class, but will be overridden by the
    //constructors of the individual pdfs.
    bool useNoHits_;

};

typedef boost::shared_ptr<I3CscdLlhAbsPdf> I3CscdLlhAbsPdfPtr;
#endif
