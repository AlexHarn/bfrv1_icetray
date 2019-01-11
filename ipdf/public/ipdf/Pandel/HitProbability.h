#ifndef IPDF_PANDEL_HitProbability_H_INCLUDED
#define IPDF_PANDEL_HitProbability_H_INCLUDED

/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @file HitProbability.h
    @version $Revision: 1.6 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>
*/

#include "ipdf/Hypotheses/InfiniteMuon.h"
#include "ipdf/Hypotheses/PointCascade.h"
#include "ipdf/Hypotheses/DirectionalCascade.h"
#include "ipdf/Pandel/PandelConstants.h"
#include "ipdf/Utilities/IPdfException.h"
#include <iosfwd>
#include <cmath>

namespace IPDF {
namespace Pandel {

/**
    @brief Pandel function implementation of PEP concept, 
    see template documentation:
    @li HitProbability<EmissionHypothesis,IceModel>
    @li HitProbability<InfiniteMuon,IceModel>
    @li HitProbability<PointCascade,IceModel>

    @todo Write HitProbabilities for PointCascade
    @todo Tidy-up constants
 */
template<typename EmissionHypothesis, typename IceModel>
class HitProbability {
public:
  typedef HitProbability<EmissionHypothesis,IceModel> Self;
  
  /// @brief Calculate the probability of at least one photo-electron hit on an OM
  template<typename OmReceiver>
  static double pHit(const OmReceiver&, const EmissionHypothesis&, const IceModel& =default_ice);
  /// @brief Calculate the probability of of not a single photo-electron hit on an OM
  template<typename OmReceiver>
  static double pNoHit(const OmReceiver&, const EmissionHypothesis&, const IceModel& =default_ice);
  /// @brief Mean number of expected photo-electron hits on an OM
  template<typename OmReceiver>
  static double meanNumPE(const OmReceiver&, const EmissionHypothesis&, const IceModel& =default_ice);

  friend std::ostream& operator<< (
    std::ostream& os, const HitProbability<EmissionHypothesis,IceModel>&) {
      return (os<<"HitProbability<UnknownHypothesis,"
	  <<IceModel::name()
	  <<">"
	  );
    }
  
private:
  static const IceModel	default_ice;
};

template<typename EmissionHypothesis,typename IceModel>
const IceModel HitProbability<EmissionHypothesis,IceModel>::default_ice=IceModel();


template<typename EmissionHypothesis,typename IceModel>
template<typename OmReceiver>
inline double HitProbability<EmissionHypothesis,IceModel>::
  pHit(const OmReceiver& omr, const EmissionHypothesis& eh, const IceModel& ice) {
  return (1.0 - Self::pNoHit(omr, eh, ice));
}

template<typename EmissionHypothesis,typename IceModel>
template<typename OmReceiver>
inline double HitProbability<EmissionHypothesis,IceModel>::
  pNoHit(const OmReceiver&, const EmissionHypothesis&, const IceModel&) {
  log_fatal("%s not implemented", __PRETTY_FUNCTION__);
  return NAN;
}

/********************************************************************/

/// @brief HitProbability<InfiniteMuon> template specialisation, 
/// see also HitProbability<EmissionHypothesis>
template<typename IceModel>
class HitProbability<InfiniteMuon,IceModel> {
public:
  typedef HitProbability<InfiniteMuon,IceModel> Self;
  
  /// Probability of at least one hit on an OM
  template<typename OmReceiver>
  static double pHit(const OmReceiver&, const InfiniteMuon&, const IceModel& =default_ice);
  template<typename OmReceiver>
  static double pNoHit(const OmReceiver&, const InfiniteMuon&, const IceModel& =default_ice);
  template<typename OmReceiver>
  static double meanNumPE(const OmReceiver&, const InfiniteMuon&, const IceModel& =default_ice);

  friend std::ostream& operator<< (
    std::ostream& os, const HitProbability<InfiniteMuon,IceModel>&) {
      return (os<<"HitProbability<InfiniteMuon,"
	  <<IceModel::name()
	  <<">"
	  );
    }
  
private:
  HitProbability();

  static double ph_eps_en(double energy);
  static double ph_eps_ori(double cs_ori, double energy);
  static double lgph_norm(double perp_dist, double energy, const IceModel& ice=default_ice, double depth=0.);
  static double ph_dist(double perp_dist, double energy);

  static const IceModel	default_ice;
  static const double	td_ph_dist_lge0_;
};

template<typename IceModel>
const double HitProbability<InfiniteMuon,IceModel>::td_ph_dist_lge0_ = log(IPDF::PandelConstants::TD_PH_DIST_E0);

template<typename IceModel>
const IceModel HitProbability<InfiniteMuon,IceModel>::default_ice=IceModel();

template<typename IceModel>
template<typename OmReceiver>
inline double HitProbability<InfiniteMuon,IceModel>::
  pHit(const OmReceiver& omr, const InfiniteMuon& eh, const IceModel& ice) {
  return (1.0 - Self::pNoHit(omr, eh, ice));
}

template<typename IceModel>
template<typename OmReceiver>
inline double HitProbability<InfiniteMuon,IceModel>::
  pNoHit(const OmReceiver& omr, const InfiniteMuon& eh, const IceModel& ice) {

  InfiniteMuon::EmissionGeometry<IceModel> egeom( eh, omr );
  double energy = eh.getEnergy();
  double eps = omr.getRelativeSensitivity() * Self::ph_eps_en(energy) 
      * Self::ph_eps_ori(egeom.cosEmissionAngle(),energy);
  double ph  = exp(Self::lgph_norm(egeom.perpendicularDistance(),energy,ice,omr.getZ()));
  return pow((1.0-ph),eps);
}

template<typename IceModel>
template<typename OmReceiver>
inline double HitProbability<InfiniteMuon,IceModel>::
  meanNumPE(const OmReceiver& omr, const InfiniteMuon& eh, const IceModel& ice) {
    double phit = Self::pHit(omr,eh,ice);
    double pnh = 1. -phit;
    double tpe;
    if (pnh <  PandelConstants::MAX_PNH_FOR_PE ){
      tpe = PandelConstants::MAX_PE_FROM_PNH;
    }else if (pnh >= 1.){
      tpe=0.;
    }else{
      tpe = -log ( pnh );
    }
    return tpe;
  }

template<typename IceModel>
inline double HitProbability<InfiniteMuon,IceModel>::ph_eps_en(double energy) {
  double r;

  r = PandelConstants::TD_PH_EPS_PE0 
    + PandelConstants::TD_PH_EPS_PE1*energy /* /1000. ALREADY IN GEV!! */;

  if (energy < PandelConstants::MIN_ION_ENERGY_GEV) {
    r *= energy/PandelConstants::MIN_ION_ENERGY_GEV;
  }
    
  return r;
}

template<typename IceModel>
inline double HitProbability<InfiniteMuon,IceModel>::
  ph_eps_ori(double cs_ori, double energy) {
  double r;
 
  if (energy < PandelConstants::MIN_ION_ENERGY_GEV)
    r =   1. + PandelConstants::TD_PH_EPS_ORI_N0
	*pow(PandelConstants::MIN_ION_ENERGY_GEV,PandelConstants::TD_PH_EPS_ORI_POW)
       	* cs_ori;
  else
    r =   1. + PandelConstants::TD_PH_EPS_ORI_N0
	*pow(energy/* /1000. already in GeV */, PandelConstants::TD_PH_EPS_ORI_POW) 
	* cs_ori;
  return r;
}

/* same as above just using the Phit lambda ! */
/// @todo Job for the IceModel?
template<typename IceModel>
inline double HitProbability<InfiniteMuon,IceModel>::
  lgph_norm(double perp_dist, double energy, const IceModel& ice, double depth) {

  AssertX(((perp_dist >= 0.)&&(energy > 0.)),
    IPDF::IllegalInputParameter
    );

  double dist = Self::ph_dist(perp_dist, energy);
  double rdist = dist*ice.InvEffScattLength(depth);
  double ldist = dist*ice.Absorptivity(depth);
  return (-ldist -rdist*
    (log(1.+ice.TauScale(depth)*IPdfConstants::C_ICE_G*ice.Absorptivity(depth))));
}

template<typename IceModel>
inline double HitProbability<InfiniteMuon,IceModel>::
  ph_dist(double perp_dist, double energy) {

  double scale = PandelConstants::TD_PH_DIST_A -
    PandelConstants::TD_PH_DIST_B *
    tanh(PandelConstants::TD_PH_DIST_L*(log(energy/* /1000. already in GeV */ )
	  -td_ph_dist_lge0_));

  return (scale * perp_dist * PandelConstants::TD_DIST_PERPTOLON);
}


/********************************************************************/

/// @brief HitProbability<PointCascade> template specialisation, 
/// see also HitProbability<EmissionHypothesis>
template<typename IceModel>
class HitProbability<PointCascade,IceModel> {
public:
  typedef HitProbability<PointCascade,IceModel> Self;
  
  /// Probability of at least one hit on an OM
  template<typename OmReceiver>
  static double pHit(const OmReceiver&, const PointCascade&, const IceModel& =default_ice);
  template<typename OmReceiver>
  static double pNoHit(const OmReceiver&, const PointCascade&, const IceModel& =default_ice);
  template<typename OmReceiver>
  static double meanNumPE(const OmReceiver&, const PointCascade&, const IceModel& =default_ice);

  friend std::ostream& operator<< (
    std::ostream& os, const HitProbability<PointCascade,IceModel>&) {
      return (os<<"HitProbability<PointCascade,"
	  <<IceModel::name()
	  <<">"
	  );
    }
  
private:
  HitProbability();

  static const IceModel	default_ice;
  static const double	norm_marek_;
  static const double	lambda_marek_;
  static const double	min_dist_;
};

template<typename IceModel>
const IceModel HitProbability<PointCascade,IceModel>::default_ice=IceModel();


template<typename IceModel>
const double HitProbability<PointCascade,IceModel>::norm_marek_ = 1.4; // [m/GeV], taken from siegmund/recoos.

template<typename IceModel>
const double HitProbability<PointCascade,IceModel>::lambda_marek_ = 29.0; // [m], taken from siegmund/recoos.

template<typename IceModel>
const double HitProbability<PointCascade,IceModel>::min_dist_ = 0.1; // [.], protect against underflow / div by zero


// NOTE:
// this is actually the meanNumPE for a non-directional cascade (Marek's
// formula). In order to apply Ignacio's formula I guess we need to
// introduce a DirectionalCascade class, which would be almost identical
// to the PointCascade class, only the egeom.cosEmissionAngle() method
// needs to be refined.
template<typename IceModel>
template<typename OmReceiver>
inline double HitProbability<PointCascade,IceModel>::
  meanNumPE(const OmReceiver& omr, const PointCascade& eh, const IceModel& ice) {

  PointCascade::EmissionGeometry<IceModel> egeom( eh, omr );
  double dist = egeom.propagationDistance()+Self::min_dist_;
  double mean = (eh.getEnergy()*Self::norm_marek_/dist)*exp(-dist/Self::lambda_marek_);

  if ( ! std::isfinite(mean) ){
      log_fatal("PointCascade meanNumPE: mean=%g dist=%g energy=%g",
                mean, dist, eh.getEnergy()
              );
  }

  // // For Directional cascades (not yet supported):
  // 
  // // This correction takes into account that a cascade emits the
  // // light not entirely isotropically, but depends on the angle
  // // w.r.t. cascade axis, so it corrects for the position of the
  // // OM w.r.t. cascade axis.
  //
  // // It does NOT correct for the orientation of the OM w.r.t. the
  // // light source, which might possibly irrelevant thanks to the
  // // milkiness of the hole ice.
  //
  // // In order to use it, in the calculation of mean you should use also
  // // norm_ignacio and lambda_ignacio.
  //
  // // Unfortunately egeom.cosEmissionAngle() gives the wrong answer!
  // // It gives the cosine of the angle under which an unscattered photo
  // // would hit the optical module. What we need is the cosine of the angle
  // // between the direction of the cascade and the difference vector of
  // // OM position and cascade vertex.
  // // double csaxis = egeom.cosEmissionAngle();
  //
  // double rlen = egeom::propagationDistance();
  // double csaxis = (rlen>0.) ? ((rx*eh.dx_ + ry*eh.dy_ + rz*eh.dz_)/rlen) : (0.);
  // mean *= csaxis * csaxis + p1_ignacio_ * csaxis + p0_ignacio_;

  return mean;
}

template<typename IceModel>
template<typename OmReceiver>
inline double HitProbability<PointCascade,IceModel>::
  pHit(const OmReceiver& omr, const PointCascade& eh, const IceModel& ice) {
  return (1.0 - Self::pNoHit(omr, eh, ice));
}

template<typename IceModel>
template<typename OmReceiver>
inline double HitProbability<PointCascade,IceModel>::
  pNoHit(const OmReceiver& omr, const PointCascade& eh, const IceModel& ice) {

  double mean = Self::meanNumPE(omr,eh,ice);
  if (mean > PandelConstants::MAX_PE_FROM_PNH ){
    return PandelConstants::MAX_PNH_FOR_PE;
  }

  double pnh = exp(-mean);
  return pnh;
}


/********************************************************************/

/// @brief HitProbability<DirectionalCascade> template specialisation, 
/// see also HitProbability<EmissionHypothesis>
template<typename IceModel>
class HitProbability<DirectionalCascade,IceModel> {
public:
  typedef HitProbability<DirectionalCascade,IceModel> Self;
  
  /// Probability of at least one hit on an OM
  template<typename OmReceiver>
  static double pHit(const OmReceiver&, const DirectionalCascade&, const IceModel& =default_ice);
  template<typename OmReceiver>
  static double pNoHit(const OmReceiver&, const DirectionalCascade&, const IceModel& =default_ice);
  template<typename OmReceiver>
  static double meanNumPE(const OmReceiver&, const DirectionalCascade&, const IceModel& =default_ice);

  friend std::ostream& operator<< (
    std::ostream& os, const HitProbability<DirectionalCascade,IceModel>&) {
      return (os<<"HitProbability<DirectionalCascade,"
	  <<IceModel::name()
	  <<">"
	  );
    }
  
private:
  HitProbability();

  static const IceModel	default_ice;
  static const double	norm_ignacio_;
  static const double	lambda_ignacio_;
  static const double	p0_ignacio_;
  static const double	p1_ignacio_;
  static const double	min_dist_;
};

template<typename IceModel>
const IceModel HitProbability<DirectionalCascade,IceModel>::default_ice=IceModel();


template<typename IceModel>
const double HitProbability<DirectionalCascade,IceModel>::norm_ignacio_ = 0.45; // [m/GeV], taken from siegmund/recoos.

template<typename IceModel>
const double HitProbability<DirectionalCascade,IceModel>::lambda_ignacio_ = 28.08; // [m], taken from siegmund/recoos.

template<typename IceModel>
const double HitProbability<DirectionalCascade,IceModel>::p0_ignacio_ = 2.566; // [.], taken from siegmund/sieglinde

template<typename IceModel>
const double HitProbability<DirectionalCascade,IceModel>::p1_ignacio_ = 3.364; // [.], taken from siegmund/sieglinde

template<typename IceModel>
const double HitProbability<DirectionalCascade,IceModel>::min_dist_ = 0.1; // [.], protect against underflow / div by zero


// NOTE:
// this is actually the meanNumPE for a non-directional cascade (Marek's
// formula). In order to apply Ignacio's formula I guess we need to
// introduce a DirectionalCascade class, which would be almost identical
// to the DirectionalCascade class, only the egeom.cosEmissionAngle() method
// needs to be refined.
template<typename IceModel>
template<typename OmReceiver>
inline double HitProbability<DirectionalCascade,IceModel>::
  meanNumPE(const OmReceiver& omr, const DirectionalCascade& eh, const IceModel& ice) {

  DirectionalCascade::EmissionGeometry<IceModel> egeom( eh, omr );
  double dist = egeom.propagationDistance()+Self::min_dist_;
  double mean = (eh.getEnergy()*Self::norm_ignacio_/dist)*exp(-dist/Self::lambda_ignacio_);

  // This correction takes into account that a cascade emits the
  // light not entirely isotropically, but depends on the angle
  // w.r.t. cascade axis, so it corrects for the position of the
  // OM w.r.t. cascade axis.
  //
  // It does NOT correct for the orientation of the OM w.r.t. the
  // light source, which might possibly irrelevant thanks to the
  // milkiness of the hole ice.

  double csaxis = egeom.cosEmissionAngle();
  mean *= csaxis * csaxis + p1_ignacio_ * csaxis + p0_ignacio_;

  if ( ! std::isfinite(mean) ){
      log_fatal("DirectionalCascade meanNumPE: "
                "mean=%g dist=%g energy=%g csaxis=%g",
                mean, dist, eh.getEnergy(), csaxis );
  }

  return mean;
}

template<typename IceModel>
template<typename OmReceiver>
inline double HitProbability<DirectionalCascade,IceModel>::
  pHit(const OmReceiver& omr, const DirectionalCascade& eh, const IceModel& ice) {
  return (1.0 - Self::pNoHit(omr, eh, ice));
}

template<typename IceModel>
template<typename OmReceiver>
inline double HitProbability<DirectionalCascade,IceModel>::
  pNoHit(const OmReceiver& omr, const DirectionalCascade& eh, const IceModel& ice) {

  double mean = Self::meanNumPE(omr,eh,ice);
  if (mean > PandelConstants::MAX_PE_FROM_PNH ){
    return PandelConstants::MAX_PNH_FOR_PE;
  }

  double pnh = exp(-mean);
  return pnh;
}


} // namespace Pandel
} // namespace IPDF

#endif // IPDF_PANDEL_HitProbability_H_INCLUDED
