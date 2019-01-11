/**
 *  copyright  (C) 2004
 *  the icecube collaboration
 *  $Id$
 *
 *  @version $Revision$
 *  @date $Date$
 *  @author David Boersma <boersma@icecube.wisc.edu>
 *
 */

#ifndef GULLIVER_I3LOGLIKELIHOODFITPARAMS_H_INCLUDED
#define GULLIVER_I3LOGLIKELIHOODFITPARAMS_H_INCLUDED

// shared pointers
#include <boost/shared_ptr.hpp>
#include "dataclasses/Utility.h"
#include "icetray/I3FrameObject.h"
#include "icetray/I3Logging.h"
#include "dataclasses/I3Vector.h"

// for NAN definition
#include <cmath>

/**
 * @class I3LogLikelihoodFitParams
 * @brief Likelihood result and fitting diagnostics
 *
 * This (base) class holds some generic parameters which are relevant
 * for most or all loglikelihood reconstructions.  The datamembers can
 * be set by the Gulliver fit; for a particular sophisticated fitter
 * (e.g. paraboloid) you might want to derive a fitparams from this,
 * or just define a separate result class.
 *
 * @todo Think of a way to include fitting errors
 * @todo Maybe the minimizer algorithm should create this object
 */

class I3LogLikelihoodFitParams : public I3FrameObject {

public:

    /// -log(total event likelihood) (sometimes named "chi^2").
    double logl_;

    /**
     * Reduced -log(event likelihood) (sometimes named "rchi^2").
     * Defined as logl divided by the number of degrees of freedom.
     *
     * The number of degrees of freedom is usually the number of
     * selected "hits" -- or pulses, waveform fragments -- minus the
     * number of fit parameters.
     */
    double rlogl_;

    /**
     * the number of degrees of freedom for the fit
     * zero if fit failed
     * negative if this is for some reason ill-defined
     */
    int ndof_;

    /**
     * the number of times that the likelihood function was evaluated by the minimizer
     * useful diagnostic (variable for NN?), depends also on minimizer algorithm though
     */
    int nmini_;


    /// constructor
    I3LogLikelihoodFitParams( double l=NAN, double r=NAN,
                              int dof=-1, int mini=-1 ):
            I3FrameObject(),
            logl_(l), rlogl_(r),
            ndof_(dof), nmini_(mini){
        log_debug( "hello" );
    }

    /// destructor
    virtual ~I3LogLikelihoodFitParams(){}
  
    std::ostream& Print(std::ostream&) const override;

    /// set logl_ and rlogl_ to NAN, ndof_ and nmini_ to -1
    virtual void Reset();

protected:

    friend class icecube::serialization::access;

    template <class Archive>
    void serialize(Archive& ar, unsigned version);

    SET_LOGGER( "I3LogLikelihoodFitParams" );

};

inline bool operator==(const I3LogLikelihoodFitParams &lhs, const I3LogLikelihoodFitParams &rhs)
{
	return (lhs.logl_==rhs.logl_) && (lhs.rlogl_==rhs.rlogl_)
	    && (lhs.ndof_==rhs.ndof_) && (lhs.nmini_==rhs.nmini_);
}

std::ostream& operator<<(std::ostream& oss, const I3LogLikelihoodFitParams& d);

I3_POINTER_TYPEDEFS( I3LogLikelihoodFitParams );
typedef I3Vector<I3LogLikelihoodFitParams> I3LogLikelihoodFitParamsVect;
I3_POINTER_TYPEDEFS(I3LogLikelihoodFitParamsVect);

#endif /* GULLIVER_I3LOGLIKELIHOODFITPARAMS_H_INCLUDED */
