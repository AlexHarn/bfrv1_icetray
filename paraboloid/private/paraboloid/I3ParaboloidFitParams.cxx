/**
 * copyright (C) 2006 the icecube collaboration
 * $Id$
 *
 * @file I3ParaboloidFitParams.cxx
 * @version $Revision$
 * @date $Date$
 * @author David Boersma <boersma@icecube.wisc.edu>
 */

#include "icetray/serialization.h"
#include "paraboloid/I3ParaboloidFitParams.h"

template <class Archive>
void I3ParaboloidFitParams::serialize(Archive& ar, unsigned version){
    ar & make_nvp("I3LogLikelihoodFitParams", base_object<I3LogLikelihoodFitParams>(*this));

    ar & make_nvp( "pbf_zen", pbfZen_ );               // theta-angle of paraboloid fit (global coordinates)
    ar & make_nvp( "pbf_azi", pbfAzi_ );               // phi-angle of paraboloid fit (global coordinates)
    ar & make_nvp( "pbf_err_1", pbfErr1_ );            // parameter 1 of error ellipse (using diagonalized hesse matrix)
    ar & make_nvp( "pbf_err_2", pbfErr2_ );            // parameter 2 of error ellipse (using diagonalized hesse matrix)
    ar & make_nvp( "pbf_rotang", pbfRotAng_ );         // rotation angle of error ellipse (using diagonalized hesse matri
    ar & make_nvp( "pbf_center_llh", pbfCenterLlh_ );  // llh-value of grid center
    ar & make_nvp( "pbf_llh", pbfLlh_ );               // llh-value of paraboloid fit
    ar & make_nvp( "pbf_zen_off", pbfZenOff_ );        // theta offset of paraboloid fit (local coordinates)
    ar & make_nvp( "pbf_azi_off", pbfAziOff_ );        // phi-offset of paraboloid fit (local coordinates)
    ar & make_nvp( "pbf_curv_11", pbfCurv11_ );        // (theta) curvature-value of paraboloid fit
    ar & make_nvp( "pbf_curv_12", pbfCurv12_ );        // (1/cov) curvature-value of paraboloid fit
    ar & make_nvp( "pbf_curv_22", pbfCurv22_ );        // (phi)   curvature-value of paraboloid fit
    ar & make_nvp( "pbf_chi_2", pbfChi2_ );            // chi^2 of parabola fit
    ar & make_nvp( "pbf_detcurvm", pbfDetCurvM_ );     // determinant of curvature matrix of parabola fit
    ar & make_nvp( "pbf_sigma_zen", pbfSigmaZen_ );    // theta error of fit using inverted curvature matrix
    ar & make_nvp( "pbf_sigma_azi", pbfSigmaAzi_ );    // phi error of fit using inverted curvature matrix
    ar & make_nvp( "pbf_covar", pbfCovar_ );           // covariance from inverted curvature matrix

    if (version > 0){
        ar & make_nvp( "pbf_tr_off_zen", pbfTrOffZen_ );    // true theta in paraboloid local system (w.r.t. fit)
        ar & make_nvp( "pbf_tr_off_azi", pbfTrOffAzi_ );    // true phi in paraboloid local system (w.r.t. fit)
        ar & make_nvp( "pbf_tr_zen", pbfTrZen_ );           // true theta
        ar & make_nvp( "pbf_tr_azi", pbfTrAzi_ );           // true phi
    }
    if (version > 1){
        ar & make_nvp( "pbf_status", pbfStatus_ );          // status index (zero=OK, nonzer=error)
    }
    if (version > 2){
        log_warn( "too high version number" );
    }
}

std::ostream& I3ParaboloidFitParams::Print(std::ostream& os) const{
    os << "I3ParaboloidFitParams:\n"
       << "     Status: " << pbfStatus_ << '\n'
       << "     Zenith: " << pbfZen_ << '\n'
       << "    Azimuth: " << pbfAzi_ << '\n'
       << "       Err1: " << pbfErr1_ << '\n'
       << "       Err2: " << pbfErr2_ << '\n'
       << "     RotAng: " << pbfRotAng_ << '\n'
       << "  CenterLLH: " << pbfCenterLlh_ << '\n'
       << "        LLH: " << pbfLlh_ << '\n'
       << "     ZenOff: " << pbfZenOff_ << '\n'
       << "     AziOff: " << pbfAziOff_ << '\n'
       << "     Curv11: " << pbfCurv11_ << '\n'
       << "     Curv12: " << pbfCurv12_ << '\n'
       << "     Curv22: " << pbfCurv22_ << '\n'
       << "       Chi2: " << pbfChi2_ << '\n'
       << "    CurvDet: " << pbfDetCurvM_ << '\n'
       << "   SigmaZen: " << pbfSigmaZen_ << '\n'
       << "   SigmaAzi: " << pbfSigmaAzi_ << '\n'
       << "      Covar: " << pbfCovar_ << '\n'
       << "   ZenTrOff: " << pbfTrOffZen_ << '\n'
       << "   AziTrOff: " << pbfTrOffAzi_ << '\n'
       << "      ZenTr: " << pbfTrZen_ << '\n'
       << "      AziTr: " << pbfTrAzi_ << '\n';
    return os;
}

void I3ParaboloidFitParams::Reset(){
    this->I3LogLikelihoodFitParams::Reset();

    pbfZen_       =
    pbfAzi_       =
    pbfErr1_      =
    pbfErr2_      =
    pbfRotAng_    =
    pbfCenterLlh_ =
    pbfLlh_       =
    pbfZenOff_    =
    pbfAziOff_    =
    pbfCurv11_    =
    pbfCurv12_    =
    pbfCurv22_    =
    pbfChi2_      =
    pbfDetCurvM_  =
    pbfSigmaZen_  =
    pbfSigmaAzi_  =
    pbfCovar_     =
    pbfTrOffZen_  =
    pbfTrOffAzi_  =
    pbfTrZen_     =
    pbfTrAzi_     = -1.0;

    pbfStatus_ = I3ParaboloidFitParams::PBF_UNDEFINED;
}

std::ostream& operator<<(std::ostream& os, const I3ParaboloidFitParams& p){
    return(p.Print(os));
}

I3_SERIALIZABLE(I3ParaboloidFitParams);
