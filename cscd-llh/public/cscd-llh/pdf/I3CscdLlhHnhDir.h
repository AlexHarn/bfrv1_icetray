/**
 * copyright (C) 2006
 * the IceCube collaboration
 * $Id$
 *
 * @file I3CscdLlhHnhDir.h
 * @version
 * @author Doug Rutledge
 * @date 1Feb2006
 */

#ifndef I3_CSCD_LLH_HNH_DIR_H
#define I3_CSCD_LLH_HNH_DIR_H

//#include "cscd-llh/pdf/I3CscdLlhHitNoHit.h"
#include "cscd-llh/pdf/I3CscdLlhAbsPdf.h"
#include <cmath>

class I3CscdLlhHnhDir : public I3CscdLlhAbsPdf//public I3CscdLlhHitNoHit
{
  public:
    I3CscdLlhHnhDir();
    virtual ~I3CscdLlhHnhDir(){}
  public:
    static const int MINIMIZATION_PARAMS;
    static const int PARAM_INDEX_ENERGY;
    static const int PARAM_INDEX_AZIMUTH;
    static const int PARAM_INDEX_ZENITH;
    static const int PARAM_INDEX_X;
    static const int PARAM_INDEX_Y;
    static const int PARAM_INDEX_Z;

    static const double DEFAULT_LEGENDRE_COEFF_0;
    static const double DEFAULT_LEGENDRE_COEFF_1;
    static const double DEFAULT_LEGENDRE_COEFF_2;
    
    //Hit-no-hit defaults
    static const double DEFAULT_NORM;
    static const double DEFAULT_LAMBDA_ATTN;
    static const double DEFAULT_NOISE;
    static const double DEFAULT_DIST_CUTOFF;
    static const double DEFAULT_DEAD;
    static const double DEFAULT_SMALL_PROB;

    static const int CONST_L_POLY_0;
    static const int CONST_L_POLY_1;
    static const int CONST_L_POLY_2;
    
    // Hit/No-hit constants
    static const int CONST_HNH_NORM;
    static const int CONST_HNH_LAMBDA_ATTN;
    static const int CONST_HNH_NOISE;
    static const int CONST_HNH_DIST_CUTOFF;
    static const int CONST_HNH_DEAD;
    static const int CONST_HNH_SMALL_PROB;

  private:
    void Evaluate(const I3CscdLlhHitPtr& hit,
      const double* param, double& value) const;
    double CalculateMu(const double dist, const double energy) const;
    int GetParamIndex(const std::string name) const;
    std::string GetParamName(const int index) const;
    double CalculateLegendrePoly(const double angle) const;
    double CalculateAngle(const double hitX, const double hitY, 
      const double hitZ, const double vertexX, const double vertexY, 
      const double vertexZ, const double hypothesisAzimuth, 
      const double hypothesisZenith) const ;

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

    bool SetConstant(int id, double value);
    bool SetConstant(int id, int value);
    bool SetConstant(int id, bool value);
    bool SetConstant(int id, std::string value);

    /**
     * Calculate the probability of not getting a hit from the cascade. 
     * @param mu The expected number of photoelectrons.
     * @return The result.
     */
    inline double ProbNoHitFromCasc(const double mu) const 
    {
      return std::isinf(mu) ? 0.0 : exp(-mu);
    }

    /**
     * Calculate the probability of getting a hit from the cascade. 
     * @param mu The expected number of photoelectrons.
     * @return The result.
     */
    inline double ProbHitFromCasc(const double mu) const 
    {
      return 1.0 - ProbNoHitFromCasc(mu);
    }

    /**
     * Calculate the probability of not getting a hit from the 
     * cascade or noise. 
     * @param mu The expected number of photoelectrons.
     * @return The result.
     */
    double ProbNoHit(const double mu) const;

    /**
     * Calculate the probability of getting a hit from the cascade or noise.
     * @param mu The expected number of photoelectrons.
     * @return The result.
   */
    double ProbHit(const double mu) const;

  private:
    double a0_;
    double a1_;
    double a2_;
    double norm_;
    double lambdaAttn_;
    double noise_;
    double distCutoff_;
    double dead_;
    double smallProb_;

  SET_LOGGER("I3CscdLlhHnhDir"); 
};

typedef boost::shared_ptr<I3CscdLlhHnhDir> I3CscdLlhHnhDirPtr;
#endif
