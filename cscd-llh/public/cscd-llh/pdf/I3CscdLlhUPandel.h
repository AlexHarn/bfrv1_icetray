/**
 * copyright  (C) 2005
 * the icecube collaboration
 *  $Id$
 *
 * @file I3CscdLlhUPandel.h
 * @version $Revision: 1.3 $
 * @date $Date$
 * @author mggreene
 * @author Doug Rutledge 
 */

#ifndef I3_CSCD_LLH_U_PANDEL
#define I3_CSCD_LLH_U_PANDEL

#include "cscd-llh/pdf/I3CscdLlhAbsPdf.h"
#include "icetray/I3Logging.h"

/**
 * @brief I3CscdLlhUPandel Calculates the UPandel function.
 */


// The terminology used in the code, e.g., "lambdaA", is described here.
/**

The Patched Pandel, or UPandel, function is briefly discussed in

  J. Ahrens, <I> et al.</I>  [AMANDA Collaboration],
  "Muon Track Reconstruction and Data Selection Techniques in AMANDA,"
  Nucl. Instrum. Meth. A <B> 524</B>, 169 (2004)
  [arXiv:astro-ph/0407044].

Some useful information can also be found in 

  "Photon arrival time distribution convoluted to a gaussian time measurement uncertainty", G. Japaridze and M. Ribordy, AMANDA-IR/20031201, 2003,

but this article is mostly concerned with the Convoluted Pandel function.

The UPandel takes a different form in three domains:

\f[ P_U(d, t) = \left\{ \begin{array}{ll}
         P_1 & \mathrm{for} \ t < 0 ;\\
         P_2 & \mathrm{for} \ 0 \leq t < t_1 ;\\
         P_3 & \mathrm{for} \ t_1 \leq t .\end{array} \right. 
\f] 

In the above formula, t is the <I>time residual</I>, that is, the time it takes light to travel from the cascade vertex to the OM, minus the time that a direct hit would take.  A direct hit is a hit that is induced by a photon that travels directly from the cascade vertex to the OM, without scattering.   \f$ t_1\f$ is the <I>patch time</I>.  The functions \f$ P_1, P_2, P_3 \f$ are a Gaussian:
 
\f[
   P_1 = A \, \frac{1}{\sqrt{2 \, \pi} \, \sigma} \, 
   \exp(-t^2 / 2 \sigma^2) \ ,
\f]
a third-order polynomial:

\f[
   P_2 = c_0 + c_1 t + c_2 t^2 + c_3 t^3 \ ,
\f]
and an unpatched Pandel function:

\f[
   P_3 = \frac{1}{\Gamma(\xi) \, \rho^\xi \, t^{\xi - 1} \, e^{- \rho t}} \ ,
\f]
where \f$ \xi \equiv \frac{d}{\lambda}\f$ and \f$\rho \equiv \frac{1}{\tau} + \frac{c}{\lambda_a}\f$.   \f$\ \ d\f$ is the distance from the cascade vertex to the OM.  The constants \f$ \tau \f$ and \f$ \lambda\f$ are empirically determined parameters with units of time and distance, respectively; \f$ \lambda_a\f$ is the photon absorption length.  \f$\ \ \sigma\f$ is the jitter time of the phototubes.

The parameters \f$A\f$, \f$c_0\f$, \f$c_1\f$, \f$c_2\f$, and \f$c_3\f$ are determined by requiring that the UPandel function is continuous and differentiable at \f$t=0\f$ and \f$t=t_1\f$, the slope is zero at \f$t=0\f$, and the integral over time from \f$ t = - \infty\f$ to \f$ t = \infty\f$ equals 1.  Also, the patch time \f$t_1\f$ is set to \f$\sqrt{2 \, \pi} \sigma\f$.

A small number \f$p_0\f$ is added to \f$P_U\f$ to model uncorrelated noise hits.  This also helps prevent \f$P_U\f$ from becoming negative due to rounding errors.
*/

class I3CscdLlhUPandel : public I3CscdLlhAbsPdf 
{

  public:
    I3CscdLlhUPandel();
    virtual ~I3CscdLlhUPandel() {}

  protected:
    static const int MINIMIZATION_PARAMS ;
    static const int PARAM_INDEX_T;
    static const int PARAM_INDEX_X;
    static const int PARAM_INDEX_Y;
    static const int PARAM_INDEX_Z;

  public:
    static const int CONST_NULL;
    static const int CONST_C_ICE;
    static const int CONST_SMALL_PROB;
    static const int CONST_TAU;
    static const int CONST_LAMBDA;
    static const int CONST_LAMBDA_A;
    static const int CONST_SIGMA;
    static const int CONST_MAX_DIST;

    static const double DEFAULT_C_ICE;
    static const double DEFAULT_SMALL_PROB;
    static const double DEFAULT_TAU;
    static const double DEFAULT_LAMBDA;
    static const double DEFAULT_LAMBDA_A;
    static const double DEFAULT_SIGMA;
    static const double DEFAULT_MAX_DIST;

  public:
  
    /**
     * Calculate the UPandel PDF.
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
     * Evaluate the Gaussian.
     *
     * @param dist Distance from the vertex.
     * @param tResidual  The time residual.
     *
     * @return the probability.
     */ 
    double EvaluateGaussian(double dist, double tResidual) const;

    /**
     * Evaluate the spline.
     *
     * @param dist Distance from the vertex.
     * @param tResidual  The time residual.
     *
     * @return the probability.
     */ 
    double EvaluateSpline(double dist, double tResidual) const;

    /**
     * Evaluate the (unpatched) Pandel function.
     *
     * @param dist Distance from the vertex.
     * @param tResidual  The time residual.
     *
     * @return the probability.
     */ 
    double EvaluatePandel(double dist, double tResidual) const;
  
    /**
     * Differentiate the PDF with respect to tResidual.
     *
     * @param dist Distance from the vertex.
     * @param tResidual  The time residual.
     *
     * @return the derivative of the PDF with respect to time.
     */
    double DifferentiatePandel(double dist, double tResidual) const;

    /**
    * Integrate the Pandel function from 0.0 to tResidual.
    */
    double IntegratePandel(double dist, double tResidual) const;

    /**
     * The spline coefficients.
     *
     */
    double CalculateC0(double dist) const;
    double CalculateC2(double dist) const;
    double CalculateC3(double dist) const;

  private:
    void SetRho() 
    {
      rho_ = (1.0/tau_) + (cIce_/lambdaA_);
    }
    void SetPatchTime() 
    {
      patchTime_ = sqrt(2.0 * M_PI) * sigma_;
    }

  protected:
    double cIce_;
    double smallProb_;
    double tau_;
    double lambda_;
    double lambdaA_;
    double sigma_;
    double maxDist_;
    double rho_;
    double patchTime_;

  SET_LOGGER("I3CscdLlhUPandel");
};

typedef boost::shared_ptr<I3CscdLlhUPandel> I3CscdLlhUPandelPtr;
#endif
