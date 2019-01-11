/**
 * copyright  (C) 2005
 * the icecube collaboration
 *  $Id$
 *
 * @file I3CscdLlhUPandelMpe.h
 * @version $Revision: $
 * @date $Date$
 * @author mggreene
 */


#ifndef I3_CSCD_LLH_U_PANDEL_MPE
#define I3_CSCD_LLH_U_PANDEL_MPE

#include "cscd-llh/pdf/I3CscdLlhUPandel.h"
#include "icetray/I3Logging.h"

/**
 * @brief I3CscdLlhUPandelMpe calculates the multi-photoelectron 
 * likelihood based on the patched Pandel (UPandel) function.
 */

/**

The MPE time-likelihood approach is briefly discussed in

  J. Ahrens, <I> et al.</I>  [AMANDA Collaboration],
  "Muon Track Reconstruction and Data Selection Techniques in AMANDA,"
  Nucl. Instrum. Meth. A <B> 524</B>, 169 (2004)
  [arXiv:astro-ph/0407044].

The MPE UPandel PDF gives the probability that, for an OM with \f$n\f$ hits, the time residuals will all be greater than \f$t\f$:

\f[
    P_{\mathrm{MPE}}(d, t) = n \, \left[\, P_U(d, t) + p_0 \, \right] \,
    \left[\,
    \left(\, \int_t^\infty \, dt^\prime \, P_U(d, t^\prime) \, \right)
    + p_0 \, \right]^{(n-1)}
    \ ,
\f] 

where \f$ P_U \f$ is the unpatched Pandel function.


A small number \f$p_0\f$ is used to model uncorrelated noise hits.  This also helps prevent \f$P_{\mathrm{MPE}}\f$ from becoming negative due to rounding errors.  To keep things simple, the same \f$p_0\f$ is used for all \f$n\f$ factors.

*/

class I3CscdLlhUPandelMpe : public I3CscdLlhUPandel 
{

  public:
    I3CscdLlhUPandelMpe();
    virtual ~I3CscdLlhUPandelMpe() 
    {
    }

  public:
  
    /**
     * Calculate the MPE UPandel PDF.
     * 
     * @param hit The OM hit data.
     * @param param Pointer to an array containing
     * the cascade vertex time and position.
     * param[i] is (t, x, y, z)
     * for i = (0, 1, 2, 3).
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
     * the cascade vertex time and position.
     * param[i] is (t, x, y, z)
     * for i = (0, 1, 2, 3).
     * @param gradient is the result, that is,
     * gradient[i] is the derivative of the PDF with respect to param[i].
     */
    virtual void CalculateGradient(const I3CscdLlhHitPtr& hit, 
      const double* param, double* gradient) const;

  protected:
    /**
     * Integrate the UPandel function from tResidual to infinity.
     */
    double IntegrateUPandel(double dist, double tResidual) const;

    /**
     * Integrate the Gaussian from negative infinity to tResidual.
     */
    double IntegrateGaussian(double dist, double tResidual) const;

    /**
     * Integrate the spline from zero to tResidual.
     */
    double IntegrateSpline(double dist, double tResidual) const;

  SET_LOGGER("I3CscdLlhUPandelMpe");
};

typedef boost::shared_ptr<I3CscdLlhUPandelMpe> I3CscdLlhUPandelMpePtr;
#endif
