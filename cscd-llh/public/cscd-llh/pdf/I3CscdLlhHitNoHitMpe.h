/**
 * copyright  (C) 2005
 * the icecube collaboration
 *  $Id$
 *
 * @file I3CscdLlhHitNoHitMpe.h
 * @version $Revision: $
 * @date $Date$
 * @author mggreene
 */


#ifndef I3_CSCD_LLH_HIT_NO_HIT_MPE
#define I3_CSCD_LLH_HIT_NO_HIT_MPE

#include "cscd-llh/pdf/I3CscdLlhHitNoHit.h"
#include "icetray/I3Logging.h"
#include <math.h>
#include <gsl/gsl_randist.h>

/**
 * @brief I3CscdLlhHitNoHitMpe calculates the probability that an OM,
 * a given distance \f$d\f$ from the cascade vertex, will be hit by \f$n\f$ photoelectrons.
 */

/**

The MPE hit/no-hit probability is a generalization of the SPE hit/no-hit probability discussed in

  J. Ahrens, <I> et al.</I>  [AMANDA Collaboration],
  "Muon Track Reconstruction and Data Selection Techniques in AMANDA,"
  Nucl. Instrum. Meth. A <B> 524</B>, 169 (2004)
  [arXiv:astro-ph/0407044],

and

  M. Kowalski, "Search for Neutrino-Induced Cascades with the AMANDA-II Detector,"
  Dissertation, Humboldt-Universit&auml;t zu Berlin, 2004.

The expected number of photoelectrons that hit an OM a distance \f$d\f$ from the cascade vertex, for distances greater than the effective scattering length, can be shown to be

\f[ \mu \approx \frac{I_0 \, E}{d} \, e^{-d/\lambda_{\mathrm{attn}}} \ ,
\f] 
where \f$E\f$ is the cascade energy, \f$\lambda_{\mathrm{attn}}\f$ is the attenuation length, and \f$I_0\f$ is a normalization constant that can be obtained from Monte Carlo studies.  Note that \f$\mu \propto 1/d \f$, not \f$1/d^2\f$, as one might naively expect.  The \f$1/d\f$ dependence can be derived from a random-walk argument, see

  P. Askebjer, <I> et al.</I>  [AMANDA Collaboration],
  "Optical Properties of Deep Ice at the South Pole: Absorption,"
  Applied Optics <B>36</B> No.~18, 4168 (1997) [arXiv:physics/9701025].

The divergence at \f$d = 0\f$ is handled by using a distance cut-off, \f$d_{\mathrm{cut-off}}\f$:
\f[ \mu \approx \frac{I_0 \, E}{d_{\mathrm{cut-off}} + d} \, e^{-d/\lambda_{\mathrm{attn}}} \ .
\f] 

The probability of getting \f$n\f$ hits from the cascade is given by a Poisson distribution:

\f[
   P^{\mathrm{casc}}(n) = \frac{\mu^n \, e^{-\mu}}{n!} \ .
\f]

The probability of registering \f$n\f$ noise hits also follows a Poisson distribution, but we will assume that noise hit probabilities are small, so we can write

\f[
    P_{\mathrm{noise}} \equiv \sum_{i=1}^\infty \, P_{\mathrm{noise}}(i) \approx
    P_{\mathrm{noise}}(1) \ . 
\f]

The probability of getting \f$n\f$ hits (from the cascade or noise) is

\f[ 
   P(n) = \left\{ \begin{array}{ll}
         P^{\mathrm{casc}}(0) \, (1 - P_{\mathrm{noise}})
               & \mathrm{for} \ n = 0 ;\\
         P^{\mathrm{casc}}(n) \, (1 - P_{\mathrm{noise}}) \, + \,
         P^{\mathrm{casc}}(n-1) \, P_{\mathrm{noise}}
               & \mathrm{for} \ n > 0 .\end{array} \right. 
\f] 

The hit/no-hit probability, that is, the probability of getting the observed hit pattern, is
\f[
    \mathcal{L} = \Pi_{\mathrm{all OM's}} \, 
      P(n; E, d) \ .  
\f]
*/

class I3CscdLlhHitNoHitMpe : public I3CscdLlhHitNoHit 
{

  public:
    I3CscdLlhHitNoHitMpe();
    virtual ~I3CscdLlhHitNoHitMpe() 
    {
    }

  public:
  
    /**
     * Calculate the MPE Hit/No-hit PDF.
     * 
     * @param hit The OM hit data.
     * @param param Pointer to an array containing
     * the cascade vertex position and energy.
     * param[i] is (x, y, z, energy)
     * for i = (1, 2, 3, 4).
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
     * the cascade vertex position and energy.
     * param[i] is (x, y, z, energy)
     * for i = (1, 2, 3, 4).
     * @param gradient is the result, that is,
     * gradient[i] is the derivative of the PDF with respect to param[i].
     */
    virtual void CalculateGradient(const I3CscdLlhHitPtr& hit, 
      const double* param, double* gradient) const;

  protected:

    /**
     * Calculate the probability of getting some number of hits from the 
     * cascade. 
     * @param n The observed number of photoelectrons.
     * @param mu The expected number of photoelectrons.
     * @return The result.
     */
    inline double ProbHitsFromCascade(int n, double mu) const
    {
      return gsl_ran_poisson_pdf(n, mu);
    }

    /**
     * Calculate the probability of getting some number of hits from 
     * the cascade and noise. 
     * @param n The observed number of photoelectrons.
     * @param mu The expected number of photoelectrons.
     * @return The result.
     */
    double ProbHits(int n, double mu) const;

  SET_LOGGER("I3CscdLlhHitNoHitMpe");
};

typedef boost::shared_ptr<I3CscdLlhHitNoHitMpe> I3CscdLlhHitNoHitMpePtr;
#endif
