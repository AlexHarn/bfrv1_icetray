/**
 * copyright  (C) 2005
 * the icecube collaboration
 *  $Id$
 *
 * @file I3CscdLlhPowell.h
 * @version $Revision: $
 * @date $Date$
 * @author mggreene
 */

#ifndef I3_CSCD_LLH_POWELL_H
#define I3_CSCD_LLH_POWELL_H

// header files

#include "cscd-llh/minimizer/I3CscdLlhMinimizer.h"
#include "cscd-llh/minimizer/NRDataTypes.h"

/**
 * @brief The I3CscdLlhPowell class uses the Powell algorithm
 * to minimize a negative log-likelihood.
 */
class I3CscdLlhPowell : public I3CscdLlhMinimizer 
{
  public:
    // Constructors
    I3CscdLlhPowell();
    /**
     * @param maxParams The maximum number of minimization parameters.
     */
    I3CscdLlhPowell(int maxParams);

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
    virtual ~I3CscdLlhPowell();

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
     * @param p A vector containing the results of the powell fit.
     *
     */
    void CopyFitParams(const Vec_DP p);

    /**
     * The fcn is the function to be minimized:  the negative log likelihood.
     *
     * @param p An array of parameters.
     *
     * @return the function value.
     *
     */
    DP Func(Vec_I_DP &p);

    /**
     * Powell minimization.
     * This code is adapted from Numerical Recipes.  
     *
     * @param p A vector containing a point in the parameter space.
     * @param xi A matrix,
     *    each column of which contains a direction vector
     *    in the parameter space.
     * @param fret The function value evaluated at the point p.
     *
     */
    void Powell(Vec_IO_DP &p, Mat_IO_DP &xi, DP &fret);

    /**
     * Find the minumum value along a line in the parameter space.
     * This code is adapted from Numerical Recipes.  
     *
     * @param p A vector containing a point in the parameter space.
     * @param xi A matrix,
     *    each column of which contains a direction vector
     *    in the parameter space.
     * @param fret The function value evaluated at the point p.
     *
     */
    void Linmin(Vec_IO_DP &p, Vec_IO_DP &xi, DP &fret);

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
    DP Brent(const DP ax, const DP bx, const DP cx,
      DP &xmin);

    /**
     * A function of one value, which is the minimization
     *    function along a line in the parameter space.
     * This code is adapted from Numerical Recipes.  
     *
     * @param x The abscissa.
     *
     * @return The minimum value.
     *
     */
    DP F1dim(const DP x);

    inline void Shft3(DP &a, DP &b, DP &c, const DP d) 
    {
      a = b;
      b = c;
      c = d;
    }

    int nCom_;
    Vec_DP *pComP_;
    Vec_DP *xiComP_;

  SET_LOGGER("I3CscdLlhPowell");

}; // class I3CscdLlhPowell

typedef boost::shared_ptr<I3CscdLlhPowell> I3CscdLlhPowellPtr;
#endif 
