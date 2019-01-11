/**
 * copyright  (C) 2005
 * the icecube collaboration
 *  $Id$
 *
 * @file I3CscdLlhHitNoHit.h
 * @version $Revision: $
 * @date $Date$
 * @author mggreene
 */

#ifndef I3_CSCD_LLH_HIT_NO_HIT
#define I3_CSCD_LLH_HIT_NO_HIT

#include "cscd-llh/pdf/I3CscdLlhAbsPdf.h"
#include "icetray/I3Logging.h"

/**
 * @brief I3CscdLlhHitNoHit calculates the probability that an OM,
 * a given distance from the cascade vertex, will be hit by a photoelectron.
 */


// The terminology used in the code, e.g., "lambdaAttn", is described here.
/**

The hit/no-hit probability is discussed in

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

The probability of getting no hits from the cascade is

\f[
   P^{\mathrm{casc}}_{\mathrm{nohit}} = e^{-\mu} \ ,
\f]
and the probability of getting one or more hits from the cascade is
\f[
   P^{\mathrm{casc}}_{\mathrm{hit}} = 1 - e^{-\mu} \ .
\f]

Suppose the probability of registering a noise hit is given by \f$P_{\mathrm{noise}}\f$, and the probability that an OM will not respond, regardless of the light intensity, is given by \f$P_{\mathrm{dead}}\f$.  Then the probability of getting no hits (from the cascade or noise) is
\f[
   P_{\mathrm{nohit}} = P_{\mathrm{dead}} \, P^{\mathrm{casc}}_{\mathrm{hit}} +
           (1 - P_{\mathrm{noise}}) \, P^{\mathrm{casc}}_{\mathrm{nohit}} \ ,
\f]
and the probability of getting one or more hits (from the cascade or noise) is
\f[
   P_{\mathrm{hit}} = (1 - P_{\mathrm{dead}}) \, P^{\mathrm{casc}}_{\mathrm{hit}} +
           P_{\mathrm{noise}} \, P^{\mathrm{casc}}_{\mathrm{nohit}} \ .
\f]
Note that other definitions of \f$P_{\mathrm{noise}}\f$ and \f$P_{\mathrm{dead}}\f$ are possible.


The hit/no-hit probability, that is, the probability of getting the observed hit pattern, is
\f[
    \mathcal{L} = \Pi_{\mathrm{all hit OM's}} \, P_{\mathrm{hit} (E, d)} \ 
        \Pi_{\mathrm{all unhit OM's}} \, P_{\mathrm{no-hit} (E, d)} \ .  
\f]

*/

class I3CscdLlhHitNoHit : public I3CscdLlhAbsPdf 
{

  public:
    I3CscdLlhHitNoHit();
    virtual ~I3CscdLlhHitNoHit() {}

  protected:
    static const int MINIMIZATION_PARAMS;
    static const int PARAM_INDEX_X;
    static const int PARAM_INDEX_Y;
    static const int PARAM_INDEX_Z;
    static const int PARAM_INDEX_ENERGY;

  public:
    static const int CONST_NULL;
    static const int CONST_NORM;
    static const int CONST_LAMBDA_ATTN;
    static const int CONST_NOISE;
    static const int CONST_DIST_CUTOFF;
    static const int CONST_DEAD;
    static const int CONST_SMALL_PROB;

    static const double DEFAULT_NORM;
    static const double DEFAULT_LAMBDA_ATTN;
    static const double DEFAULT_NOISE;
    static const double DEFAULT_DIST_CUTOFF;
    static const double DEFAULT_DEAD;
    static const double DEFAULT_SMALL_PROB;

  public:
  
    /**
     * Calculate the Hit/No-hit PDF.
     * 
     * @param hit The OM hit data.
     * @param param Pointer to an array containing
     * the cascade vertex position and energy.
     * param[i] is (x, y, z, energy)
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
     * the cascade vertex position and energy.
     * param[i] is (x, y, z, energy)
     * for i = (0, 1, 2, 3).
     * @param gradient is the result, that is,
     * gradient[i] is the derivative of the PDF with respect to param[i].
     */
    virtual void CalculateGradient(const I3CscdLlhHitPtr& hit, 
      const double* param, double* gradient) const;

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

    /**
     * Calculate the expected number of photoelectrons. 
     * @param dist The distance from the cascade vertex to the OM.
     * @param energy The cascade energy.
     * @return The result.
     */
    double CalculateMu(double dist, double energy) const;

    /**
     * Calculate the probability of not getting a hit from the cascade. 
     * @param mu The expected number of photoelectrons.
     * @return The result.
     */
    inline double ProbNoHitFromCasc(double mu) const 
    {
      return std::isinf(mu) ? 0.0 : exp(-mu);
    }

    /**
     * Calculate the probability of getting a hit from the cascade. 
     * @param mu The expected number of photoelectrons.
     * @return The result.
     */
    inline double ProbHitFromCasc(double mu) const 
    {
      return 1.0 - ProbNoHitFromCasc(mu);
    }

    /**
     * Calculate the probability of not getting a hit from the 
     * cascade or noise. 
     * @param mu The expected number of photoelectrons.
     * @return The result.
     */
    double ProbNoHit(double mu) const;

    /**
     * Calculate the probability of getting a hit from the cascade or noise.
     * @param mu The expected number of photoelectrons.
     * @return The result.
   */
    double ProbHit(double mu) const;

  protected:
    double norm_;
    double lambdaAttn_;
    double noise_;
    double distCutoff_;
    double dead_;
    double smallProb_;

  SET_LOGGER("I3CscdLlhHitNoHit");
};

typedef boost::shared_ptr<I3CscdLlhHitNoHit> I3CscdLlhHitNoHitPtr;
#endif
