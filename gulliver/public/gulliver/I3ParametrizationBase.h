/**
 * copyright  (C) 2004
 * the icecube collaboration
 * $Id$
 *
 * @version $Revision$
 * @date $Date$
 * @author David Boersma <boersma@icecube.wisc.edu>
 *
 * This is a base class for classes which holds some track and its
 * parametrization. The template parameters are the track type and the
 * number of parameters. The parametrization should also work for classes
 * derived from the track type.
 */

#ifndef GULLIVER_I3PARAMETRIZATION_INCLUDED
#define GULLIVER_I3PARAMETRIZATION_INCLUDED

// standard library stuff
#include <cassert>
#include <vector>
using std::vector;

// for the covariance matrix
#include <boost/numeric/ublas/symmetric.hpp>
#include <boost/numeric/ublas/io.hpp>

// smart pointers
#include "icetray/IcetrayFwd.h"

// specs
#include "gulliver/I3FitParameterInitSpecs.h"
#include "gulliver/I3EventHypothesis.h"

/**
 * @class I3ParametrizationBase
 *
 * This base class defines the methods of parametrized particles which are
 * relevant for the event loglikelihood "functor" classes (which mediate
 * between the physics likelihood function and the non-phyisics minimizer).
 *
 * The physics of the "emission hypothesis" is given in an I3EventHypothesis, which
 * is just struct with an I3ParticlePtr with regular particle information and
 * an I3FrameObjectPtr datamember to accommodate any non-standard fitvariables.
 *
 * If you define a new parametrization, you only need to define the two
 * methods which translate from physics variables to minimizer parameters
 * and vice versa, and in the constructor you should initialize the
 * parameters (par_) and parameter specs (parspecs_). Oh yes, you should
 * also implement the trivial GetName() method.
 *
 * If you (implementer of new parametrization) actually use the
 * non-standard part of the I3EventHypothesis, then it is up to you to
 * check that the "nonstd" I3FrameObjectPtr is indeed of the type for
 * which your parametrization is intended.
 *
 * @todo Maybe implement a Test() method which parametrizes back and forth
 *       and checks that the same result is obtained.
 *
 * @sa I3Gulliver, I3GulliverBase, I3EventHypothesis
 */
class I3ParametrizationBase {

protected:

    /**
     * This should calculate physics datamembers of the particle
     * (in I3Particle and maybe I3FrameObject) from the values in the
     * parametrization (par_).
     */
    virtual void UpdatePhysicsVariables() = 0;

    /**
     * This should calculate the values in par_ from datamembers of the particle
     * If relevant it should also update stepsizes.
     */
    virtual void UpdateParameters() = 0;

    /**
     * This should calculate the values in par_gradient_, using the current
     * gradient_ and hypothesis_.
     */
    virtual void ApplyChainRule(){
        log_fatal( "(%s) chain rule not implemented",
                   this->GetName().c_str() );
    }

    /// the physics variables
    I3EventHypothesisPtr hypothesis_;

    /// the gradient in physics variables (dlogl/dhypothesis)
    I3EventHypothesisPtr gradient_;

    /// parametrization info for the minimizer: stepsizes etcetera.
    std::vector<I3FitParameterInitSpecs> parspecs_;

    /// the parametrization of the free physics variables
    std::vector<double> par_;

    /// the gradient in parameters (dlogl/dpar)
    std::vector<double> par_gradient_;

public:

    /// just return a name, useful for diagnostic blurbs
    virtual const std::string GetName() const = 0;

    /**
     * @brief Initilialize gradient capability
     *
     * If wantgradient is false, then no gradients are desired (until further
     * notice) and any operations for applying the chain rule can be skipped.
     * Return value is ignored in this case.
     *
     * If wantgradient is true, then gradients are desired and any
     * operations needed for applying the chain rule should be performed.
     * In particular,  the gradient_ data member should be initialized such
     * that all datamembers of gradient_.particle (and gradient.nonstd_)
     * that will be used in the chain rule calculation are set to zero.
     *
     * If the parametrization subclass indeed provides the ApplyChainRule
     * method (possibly depending on user configuration), then the return value
     * should be true. Otherwise it should return false.
     *
     * @note I3Gulliver calls this init method for every minimizer function call.
     */
    virtual bool InitChainRule(bool wantgradient){
        return false;
    }

    /**
     * Some parametrizations may want to access frame information
     * at the start of an event. E.g. in order to align a parametrization
     * to another fit or to the COG of the pulses.
     */
    virtual void SetEvent(const I3Frame& f){
        log_trace( "%s SetEvent(f) not implemented.",
                   this->GetName().c_str());
    }

    /**
     * Get any extra information that might be interesting for the analysis.
     * This is very specific to the particulars of the parametrization.
     * Gulliver modules will get this frame object through the fit result pointer
     * from the I3Gulliver object, and if it is indeed defined (pointer not NULL)
     * then they should store it into the frame.
     */
    virtual I3FrameObjectPtr GetDiagnostics( const I3EventHypothesis &fitresult ){
        return I3FrameObjectPtr();
    }

    /**
     * This is called by Gulliver after the minimization has finished to provide
     * the covariance matrix. Support in Gulliver is tentative at this point,
     * you might get only NaNs, or only the diagonal elements. For backward
     * compatibility, the default implementation of this function does nothing.
     * To do something useful with the passed covariance matrix, overwrite this
     * function in your derived class.
     */
    virtual void PassCovariance(const boost::numeric::ublas::symmetric_matrix<double>& cov)
    { /* I do nothing by default, overwrite me in Derived */ }

    /**
     * constructor: an already existing I3EventHypothesis object is parametrized.
     */
    I3ParametrizationBase( I3EventHypothesisPtr eh ):
        hypothesis_(eh){}

/////////////////////////////////////////////////////////////////////////
// Everything below this comment is for implementation of I3Gulliver
// You don't need to worry about this for any new parametrization.
/////////////////////////////////////////////////////////////////////////

    /// destructor
    virtual ~I3ParametrizationBase(){}

    /**
     * Tie the parametrization to a different fit object.
     * @param[in] eh pointer of new hypothesis (NULL pointers are forbidden)
     * Note that the pointer itself is used, no copy is made.
     * The pointee will be modified by the parametrization service.
     */
    void SetHypothesisPtr( I3EventHypothesisPtr eh );

    /// Get pointer to current event hypothesis
    I3EventHypothesisPtr GetGradientPtr() {
        return gradient_;
    }

    /// Get pointer to current event hypothesis
    I3EventHypothesisPtr GetHypothesisPtr() {
        return hypothesis_;
    }

    /// this calculates the physics particle from the parameter vector and returns it
    I3EventHypothesisConstPtr GetHypothesisPtr(const vector<double> &par);

    /// gradient (in par) is computed and copied into grad.
    void GetGradient( vector<double> &grad );

    /// Get number of free parameters
    unsigned int GetNPar() const {
        assert( par_.size() );
        assert( par_.size() == parspecs_.size() );
        return par_.size();
    }

    /**
     * update the parameters according to the current event hypothesis
     * then return a I3FitParameterInitSpecs object which contains
     * besides the parameter values also the ranges, stepsizes etc.
     */
    const std::vector<I3FitParameterInitSpecs>& GetParInitSpecs( I3EventHypothesisPtr eh = I3EventHypothesisPtr() );

    /// macro for log_message system
    SET_LOGGER("I3ParametrizationBase");


private:

    /**
     * Prohibit the default constructor by making it private.
     * A parameterization should refer to an already existing object, so
     * it makes no sense to have a constructor without particle argument.
     */
    I3ParametrizationBase();

};

I3_POINTER_TYPEDEFS(I3ParametrizationBase);

#endif /* GULLIVER_I3PARAMETRIZATIOn_INCLUDED */
