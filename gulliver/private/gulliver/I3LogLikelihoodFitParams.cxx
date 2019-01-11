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

#include "icetray/serialization.h"
#include "gulliver/I3LogLikelihoodFitParams.h"

template <class Archive>
void I3LogLikelihoodFitParams::serialize(Archive& ar, unsigned version){
    ar & make_nvp("I3FrameObject", base_object<I3FrameObject>(*this));
    ar & make_nvp("LogL", logl_ ); // or "chi2"
    ar & make_nvp("rLogL", rlogl_ ); // or "rchi2"
    ar & make_nvp("Ndof", ndof_ );
    ar & make_nvp("Nmini", nmini_ );
    if (version > 0){
        log_warn( "too high version number" );
    }
}

void I3LogLikelihoodFitParams::Reset(){
    logl_ = NAN;
    rlogl_ = NAN;
    ndof_ = -1;
    nmini_ = -1;
}

std::ostream& operator<<(std::ostream& oss, const I3LogLikelihoodFitParams& p){
    return(p.Print(oss));
}

std::ostream& I3LogLikelihoodFitParams::Print(std::ostream& oss) const{
    oss << "[ I3LogLikelihoodFitParams logL : " << logl_ << std::endl
        << "                          rlogL : " << rlogl_ << std::endl
        << "                           Ndof : " << ndof_  << std::endl
        << "                          Nmini : " << nmini_  << std::endl
        << "]" ;
    return oss;
}

I3_CLASS_VERSION(I3LogLikelihoodFitParams, 0);
I3_SERIALIZABLE(I3LogLikelihoodFitParams);
I3_SERIALIZABLE(I3LogLikelihoodFitParamsVect);
