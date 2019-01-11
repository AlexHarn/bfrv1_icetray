/**
 * copyright  (C) 2005
 * the icecube collaboration
 *  $Id$
 *
 * @file I3CscdLlhSimplex.h
 * @version $Revision: $
 * @date $Date$
 * @author mggreene
 */

#ifndef I3_CSCD_LLH_SIMPLEX_H
#define I3_CSCD_LLH_SIMPLEX_H

// header files

#include "cscd-llh/minimizer/I3CscdLlhMinimizer.h"
#include "cscd-llh/minimizer/NRDataTypes.h"

/**
 * @brief The I3CscdLlhSimplex class uses the Simplex algorithm
 * to minimize a negative log-likelihood.
 */
class I3CscdLlhSimplex : public I3CscdLlhMinimizer 
{

  public:
    // Constructors
    I3CscdLlhSimplex();
    /**
     * @param maxParams The maximum number of minimization parameters.
     */
    I3CscdLlhSimplex(int maxParams);

  protected:
    /**
     * Initialize.  Called by the constructors.
     *
     * @param maxParams The maximum number of minimization parameters.
     *
     * @return true if successful.
     */
    virtual void Init(int maxParams);

  public:
    // Destructor
    virtual ~I3CscdLlhSimplex();

  public:

    /**
     * Perform the actual minimization.
     *
     * @param hits The data: a list of OM hits.
     * @param pdf The PDF.
     *
     * @return true if successful.
     */
    virtual bool Minimize(std::list<I3CscdLlhHitPtr>* hits, I3CscdLlhAbsPdfPtr pdf);

  private:

    /**
     * Copy the results of the fit to the I3CscdLlhResult.
     *
     * @param p A matrix containing the results of the simplex fit.
     * The first row contains the best vertex.
     *
     */
    void CopyFitParams(const Mat_DP p);

    /**
     * The fcn is the function to be minimized:  the negative log likelihood.
     *
     * @param x An array of parameters.
     *
     * @return the function value.
     *
     */
    DP Func(Vec_I_DP &x);

    /**
     * Simplex minimization.
     * This code is adapted from Numerical Recipes.  
     *
     * @param p A matrix containing the simplex.
     * Each row has a simplex vertex.
     * The number of colummns is equal to the dimension of the parameter space.
     * @param y A vector containing the function values
     *    at each of the simplex vertices.
     *
     */
    void Amoeba(Mat_IO_DP &p, Vec_IO_DP &y);

   /**
     * Find the sum of simplex vertices.
     * This code is adapted from Numerical Recipes.  
     *
     * @param p A matrix containing the simplex.
     * @param psum The resulting sum.
     *
     */
    inline void Get_psum(Mat_I_DP &p, Vec_O_DP &psum);

    /**
     * Evaluate the function with a trial simplex vertex.
     * This code is adapted from Numerical Recipes.  
     *
     * @param p A matrix containing the simplex.
     * @param y The result (the function value.)
     * @param psum The sum of simplex vertices.
     * @param ihi The index (row) of the worst vertex that
     *     will be replaced if the trial vertex is better.
     * @param fac A factor used for finding the position of the trial vertex.
     *
     */
    DP Amotry(Mat_IO_DP &p, Vec_O_DP &y, Vec_IO_DP &psum,
      const int ihi, const DP fac);


  SET_LOGGER("I3CscdLlhSimplex");

}; // class I3CscdLlhSimplex

typedef boost::shared_ptr<I3CscdLlhSimplex> I3CscdLlhSimplexPtr;
#endif 
