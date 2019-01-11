/**
 * copyright  (C) 2005
 * the icecube collaboration
 *  $Id$
 *
 * @file I3CscdLlhBrent.h
 * @version $Revision: $
 * @date $Date$
 * @author mggreene
 */

#ifndef I3_CSCD_LLH_BRENT_H
#define I3_CSCD_LLH_BRENT_H

// header files

#include "cscd-llh/minimizer/I3CscdLlhMinimizer.h"
#include "cscd-llh/minimizer/NRDataTypes.h"

/**
 * @brief The I3CscdLlhBrent class uses the Brent algorithm
 * to minimize a negative log-likelihood.  It can only be used
 * for a function of one variable.
 */
class I3CscdLlhBrent : public I3CscdLlhMinimizer 
{

  public:
    // Constructors
    I3CscdLlhBrent();
    /**
     * @param maxParams The maximum number of minimization parameters.
     */
    I3CscdLlhBrent(int maxParams);

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
    virtual ~I3CscdLlhBrent();

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
     * @param xmin The location of the minimum.
     *
     */
    void CopyFitParams(const DP xmin);

    /**
     * The fcn is the function to be minimized:  the negative log likelihood.
     *
     * @param x An array of parameters.
     *
     * @return the function value.
     *
     */
    DP Func(const DP x);

    /**
     * Find a bracketing triplet around the minimum.
     * This code is adapted from Numerical Recipes.  
     *
     * @param ax A seed bracket and a final side bracket.
     * @param bx A seed bracket and a final middle bracket.
     * @param cx A final side bracket.
     * @param fa The function value at the final ax.
     * @param fb The function value at the final bx.
     * @param fc The function value at the final cx.
     *
     */
    void Mnbrak(DP &ax, DP &bx, DP &cx,
      DP &fa, DP &fb, DP &fc);

    /**
     * Brent minimization.
     * This code is adapted from Numerical Recipes.  
     *
     * @param ax Side bracket.
     * @param bx Middle bracket.
     * @param cx Side bracket.
     * @param xmin The location of the minimum.
     *
     * @return The minimum function value.
     *
     */
    DP Brent(const DP ax, const DP bx, const DP cx, DP &xmin);

    inline void Shft3(DP &a, DP &b, DP &c, const DP d) 
    {
      a = b;
      b = c;
      c = d;
    }

    int idxFreeParam_;
    double* fitParams_;

  SET_LOGGER("I3CscdLlhBrent");

}; // class I3CscdLlhBrent

typedef boost::shared_ptr<I3CscdLlhBrent> I3CscdLlhBrentPtr;
#endif 
