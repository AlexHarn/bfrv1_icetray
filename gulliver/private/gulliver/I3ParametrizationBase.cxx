/**
 * copyright  (C) 2007
 * the icecube collaboration
 * $Id$
 *
 * @file I3ParametrizationBase.cxx
 * @version $Revision$
 * @date $Date$
 * @author David Boersma <boersma@icecube.wisc.edu>
 *
 */

#include "gulliver/I3ParametrizationBase.h"

void
I3ParametrizationBase::SetHypothesisPtr( I3EventHypothesisPtr eh ){
    assert( par_.size() );
    assert( par_.size() == parspecs_.size() );
    assert( eh );
    hypothesis_ = eh;
    UpdateParameters();
}

I3EventHypothesisConstPtr
I3ParametrizationBase::GetHypothesisPtr(const vector<double> &par){
    assert(par_.size()>0);
    assert(par.size()==par_.size());
    // TODO: check against boundaries
    copy(par.begin(),par.end(),par_.begin());
    UpdatePhysicsVariables();
    return hypothesis_;
}

const std::vector<I3FitParameterInitSpecs>&
I3ParametrizationBase::GetParInitSpecs( I3EventHypothesisPtr eh ){
    if ( eh ){
        hypothesis_ = eh;
        UpdateParameters();
    }
    assert( par_.size() );
    assert( par_.size() == parspecs_.size() );
    if ( ! hypothesis_ ){
        return parspecs_; //LCOV_EXCL_LINE it should not be possible to have null pointer
    }
    std::vector<I3FitParameterInitSpecs>::iterator ispec;
    int ipar = 0;
    for ( ispec = parspecs_.begin(); ispec != parspecs_.end(); ++ispec ){
        ispec->initval_  = par_[ipar++];
    }
    return parspecs_;
}

void I3ParametrizationBase::GetGradient(vector<double> &grad ) {
    ApplyChainRule();
    assert( par_.size() == par_gradient_.size());
    assert( grad.size() == par_gradient_.size());
    copy(par_gradient_.begin(),par_gradient_.end(),grad.begin());
    /*
    ostringstream oss;
    oss << "checkcheck" << endl;
    oss << "par=";
    copy(par_.begin(),par_.end(),ostream_iterator<double>(oss,","));
    oss << endl << "par_grad=";
    copy(par_gradient_.begin(),par_gradient_.end(),ostream_iterator<double>(oss,","));
    oss << endl << "grad=";
    copy(grad.begin(),grad.end(),ostream_iterator<double>(oss,","));
    log_trace("%s",oss.str().c_str());
    */
    return;
}
