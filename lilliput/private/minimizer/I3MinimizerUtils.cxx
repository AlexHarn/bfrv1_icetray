/**
 * @file I3MinimizerUtils.cxx
 * @brief shared functions for taming troublesome minimizers
 *
 * (c) 2010 the IceCube Collaboration
 *
 * $Id$
 * @version $Revision$
 * @date $LastChangedDate$ ($LastChangedBy$)
 * @author boersma
 *
 * @todo Maybe I3MinimizerUtils should be moved out of lilliput, I think it belongs in gulliver/utilities.
 */

#include "minimizer/I3MinimizerUtils.h"

namespace I3MinimizerUtils {

    void
    CheckMinimum(const I3ServiceBase &self, I3MinimizerResult &result,
        const std::vector<I3FitParameterInitSpecs> &parspecs, I3GulliverBase &llh)
    {
        bool keepchecking = true;
        std::vector<double> testpar = result.par_;
        for ( int nsteps=1; keepchecking && nsteps<=3; nsteps++ ){
            result.converged_ = false; // guilty until proven innocent :-)
            keepchecking = false;
            log_debug( "(%s) minimizer 'converged' back to seed value "
                "=> checking flatness of LLH space", self.GetName().c_str() );
            bool deepbreak = false;
            for (unsigned int i = 0; i < parspecs.size(); i++ ){
                if ( deepbreak ) break;
                // going to test one stepsize on either side of the result
                for ( int pm = -1; pm <= +1; pm += 2 ){
                    if ( deepbreak ) break;
                    testpar[i] += pm * parspecs[i].stepsize_;
                    if ( parspecs[i].minval_ < parspecs[i].maxval_ ){
                        // bounds were set for this parameter
                        double bound = (pm<0) ? parspecs[i].minval_
                                              : parspecs[i].maxval_;
                        if ( pm*(testpar[i] - bound) > 0 ){
                            // Oops, stepped out of bounds.
                            // Try again, retreat half a stepsize.
                            testpar[i] -= 0.5 * pm * parspecs[i].stepsize_;
                            if ( pm*(testpar[i] - bound) > 0 ){
                                // Oops again. Just omit the test on this side of the result.
                                // (We do not want to test arbitrarily close to the result.)
                                testpar[i] = result.par_[i];
                                continue;
                            }
                        }
                    }
                    // OK, let's have a look...
                    double testval = llh( testpar );
                    if ( testval < result.minval_ ){
                        log_debug( "(%s) OOPS: -log(L) value *decreases* by varying "
                            "parameter %d from %f (fit result) to %f (%d stepsize%s away), "
                            "fval(fit)=%g and fval(shifted)=%g: maybe that's OK, maybe not",
                            self.GetName().c_str(),
                            i, result.par_[i], testpar[i],
                            nsteps,((nsteps>1)?"s":""),
                            result.minval_, testval );
                        result.converged_ = false;
                        // start over; but do *not* restore original parameter value
                        // it might be that we will just find a slightly better minimum
                        // (Claudio ran into this use case when making a LLH map with very small stepsizes)
                        result.par_[i] = testpar[i];
                        result.minval_ = testval;
                        keepchecking = true;
                        deepbreak = true;
                        break;
                    }
                    // so far so good...
                    if ( testval > result.minval_ ){
                        log_debug( "(%s) OK (maybe): -log(L) value increases by varying "
                            "parameter %d from %f (fit) to %f (stepsize=%g)",
                            self.GetName().c_str(), i, result.par_[i], testpar[i], parspecs[i].stepsize_ );
                        result.converged_ = true; // acquitted! (maybe, need to check the rest)
                    } else {
                        log_debug( "(%s) varying parameter %d from %f to %f "
                            "did NOT change the -log(L) value %f",
                            self.GetName().c_str(),
                            i, result.par_[i], testpar[i], result.minval_ );
                    }

                    // put back to original value to get ready for next test (if any)
                    testpar[i] = result.par_[i];
                }
            }

        }
        log_debug( "(%s) %s", self.GetName().c_str(),
            result.converged_ ? "OK, NOT FLAT => converged"
                              : "oh, horror, FLAT/SADDLEPOINT => NOT converged" );
    }

    ParameterWithInclusiveBounds::
    ParameterWithInclusiveBounds(double minval, double maxval, double initval){
        assert(initval >= minval);
        assert(initval <= maxval);
        mean_ = 0.5*(maxval + minval);
        span_ = 0.5*(maxval - minval);
        SetBoundedValue( initval ); // also computes & sets free value, of course
    }

    ParameterWithInclusiveBounds::
    ParameterWithInclusiveBounds( const I3FitParameterInitSpecs &spec ){ // output
        assert(spec.initval_ >= spec.minval_);
        assert(spec.initval_ <= spec.maxval_);
        mean_ = 0.5*(spec.maxval_ + spec.minval_);
        span_ = 0.5*(spec.maxval_ - spec.minval_);
        SetBoundedValue( spec.initval_ ); // also computes & sets free value, of course
    }


    // maps any real number to a number between minval and maxval (inclusive)
    // minval=mean-span, maxval=mean+span
    void
    ParameterWithInclusiveBounds::
    SetFreeValue(double xfree){
        xfree_ = xfree;
        xbounded_ = mean_ + span_*sin(xfree);
    }


    // maps any real number between minval and maxval (inclusive)
    // to a number between -pi/2 and +pi/2
    void
    ParameterWithInclusiveBounds::
    SetBoundedValue(double xbounded){
        xbounded_ = xbounded;
        double sinval = (xbounded-mean_)/span_;
        assert(fabs(sinval)<=1); // in Debug mode we crash
        // in Release mode we deal with it
        // Note:
        // this is only used for setting the initial value
        // this value should never be *on* the boundary
        // so maybe we should not be so friendly here
        xfree_= (sinval>+1) ? +M_PI*0.5 :
                (sinval<-1) ? -M_PI*0.5 :
                asin(sinval);
    }

    double
    ParameterWithInclusiveBounds::GetFreeStep(double boundstep){
        double ftmp = xfree_;
        double btmp = xbounded_;
        // todo: check that xbounded +/- boundstep is not OOB
        SetBoundedValue( btmp + (ftmp > 0. ? -1 : +1) * boundstep );
        double freestep = fabs(GetFreeValue() - ftmp);
        xfree_ = ftmp;
        xbounded_ = btmp;
        return freestep;
    }

    // not relevant :-)
    // double GetBoundStep(double freestep);

}
