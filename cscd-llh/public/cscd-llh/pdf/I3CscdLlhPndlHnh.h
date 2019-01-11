/**
 * copyright  (C) 2005
 * the icecube collaboration
 *  $Id$
 *
 * @file I3CscdLlhPndlHnh.h
 * @version $Revision: $
 * @date $Date$
 * @author mggreene
 */

#ifndef I3_CSCD_LLH_PNDL_HNH
#define I3_CSCD_LLH_PNDL_HNH

#include "cscd-llh/pdf/I3CscdLlhAbsPdf.h"
#include "cscd-llh/pdf/I3CscdLlhUPandel.h"
#include "cscd-llh/pdf/I3CscdLlhHitNoHitMpe.h"
#include "icetray/I3Logging.h"

/**
 * @brief I3CscdLlhPndlHnh calculates a combined likelihood
 *   consisting of a single photoelectron (SPE) UPandel (patched Pandel) PDF
 *   and a multi-photoelectron (MPE) Hit/No-hit PDF.
 */


/**
The I3CscdLlhPndlHnh calculates a combined likelihood as

\f[
    \mathcal{L} = \mathcal{L}_{\mathrm{UPandel}} \, \cdot \, 
     \left(\mathcal{L_{\mathrm{hit/no-hit}}}\right)^w \ ,
\f]
where \f$w\f$ is a weight factor.
The UPandel PDF is single photoelectron (SPE);
the Hit/No-hit is multi-photoelectron (MPE).
*/

class I3CscdLlhPndlHnh : public I3CscdLlhAbsPdf 
{

  public:
    I3CscdLlhPndlHnh();
    virtual ~I3CscdLlhPndlHnh() 
    {
    }

  protected:
    static const int MINIMIZATION_PARAMS ;
    static const int PARAM_INDEX_T;
    static const int PARAM_INDEX_X;
    static const int PARAM_INDEX_Y;
    static const int PARAM_INDEX_Z;
    static const int PARAM_INDEX_ENERGY;

  public:
    static const int CONST_NULL;
    static const int CONST_WEIGHT;

    // UPandel constants
    static const int CONST_PNDL_C_ICE;
    static const int CONST_PNDL_SMALL_PROB;
    static const int CONST_PNDL_TAU;
    static const int CONST_PNDL_LAMBDA;
    static const int CONST_PNDL_LAMBDA_A;
    static const int CONST_PNDL_SIGMA;
    static const int CONST_PNDL_MAX_DIST;

    // Hit/No-hit constants
    static const int CONST_HNH_NORM;
    static const int CONST_HNH_LAMBDA_ATTN;
    static const int CONST_HNH_NOISE;
    static const int CONST_HNH_DIST_CUTOFF;
    static const int CONST_HNH_DEAD;
    static const int CONST_HNH_SMALL_PROB;

    static const double DEFAULT_WEIGHT;

  public:
  
    /**
     * Calculate the PndlHnh PDF.
     * 
     * @param hit The OM hit data.
     * @param param Pointer to an array containing
     * the cascade vertex time, position, and energy.
     * param[i] is (t, x, y, z, energy)
     * for i = (0, 1, 2, 3, 4).
     * @param value The result.
     *
     */
    virtual void Evaluate(const I3CscdLlhHitPtr& hit, const double* param,
          double& value) const;

    /**
     * Calculate the gradient of the PDF with respect to 
     * the vertex parameters.
     *  
     * @param hit The OM hit data.
     * @param param Pointer to an array containing
     * the cascade vertex time, position, and energy.
     * param[i] is (t, x, y, z, energy)
     * for i = (0, 1, 2, 3, 4).
     * @param gradient is the result, that is,
     * gradient[i] is the derivative of the PDF with respect to param[i].
     */
    virtual void CalculateGradient(const I3CscdLlhHitPtr& hit, 
      const double* param,double* gradient) const;

    /**
     * Get the number of minimization parameters (fixed and free.)
     *
     * @return the number of paramters.
     */
    virtual int GetNumParams() const 
    {
      return MINIMIZATION_PARAMS;
    }

    /**
     * Get the index of a minimization parameter.
     *
     * @param name The name of the parameter.
     *
     * @return the index of the parameter in the parameter array.
     */
    virtual int GetParamIndex(const std::string name) const;

    /**
     * Get the name of a minimization parameter.
     *
     * @param index The parameter index.
     *
     * @return the name of the parameter.
     */
    virtual std::string GetParamName(const int index) const;

    /**
     * Set a constant value.
     * @param id A predefined identifier for the constant.
     * @param value The value to be assigned to the constant.
     * 
     * @return true if successful.
     */
    virtual bool SetConstant(int id, double value);
    virtual bool SetConstant(int id, int value);
    virtual bool SetConstant(int id, bool value);
    virtual bool SetConstant(int id, std::string value);

  protected:

    I3CscdLlhUPandel pdfPndl_;
    I3CscdLlhHitNoHitMpe pdfHnh_;

    double weight_;

  SET_LOGGER("I3CscdLlhPndlHnh");
};

typedef boost::shared_ptr<I3CscdLlhPndlHnh> I3CscdLlhPndlHnhPtr;
#endif
