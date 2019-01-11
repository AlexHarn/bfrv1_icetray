#ifndef IPDF_IceModel_H
#define IPDF_IceModel_H

/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @file IceModel.h
    @version $Revision: 1.5 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>

    @brief Interface to the ice properties
    
    Constants relating to the hole ice type
*/

#include "ipdf/Utilities/IPdfException.h"
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

namespace IPDF {
namespace Pandel {

  /**
   * @brief Interface to the ice properties
   *
   * Constants relating to the hole ice type.
   *
   * The parameter "d" passed to each method is the depth in the 
   * ice and is ignored for these (bulk ice) models.
   */
template<typename IceModel>
struct IceModelTraits {

  /// @brief Pandel parameter relating to light propagation
  double TauScale(double d=0.) const { return IceModel::TD_TAU; }
  /// @brief The Absorption length of light in ice
  double AbsorptionLength(double d=0.) const { return IceModel::TD_ABS; }
  /// @brief The inverse of the absorption length
  double Absorptivity(double d=0.) const { return 1./IceModel::TD_ABS; }
  /// @brief The effective scattering length
  double EffScattLength(double d=0.) const { return IceModel::TD_LAM; }
  /// @brief The inverse of the effective scattering length
  double InvEffScattLength(double d=0.) const { return 1./IceModel::TD_LAM; }

};

struct H2 : public IceModelTraits<H2> {
  static const std::string name() { return "H2"; }
    static const double TD_ABS;
    static const double TD_TAU;
    static const double TD_LAM;
    static const double TD_DIST_P1;
    static const double TD_DIST_P0_CS0;
    static const double TD_DIST_P0_CS1;
    static const double TD_DIST_P0_CS2;
};

struct H0 : public IceModelTraits<H0> {
  static const std::string name() { return "H0"; }
    static const double TD_ABS;
    static const double TD_TAU ;
    static const double TD_LAM ;
    static const double TD_DIST_P1 ;
    static const double TD_DIST_P0_CS0 ;
    static const double TD_DIST_P0_CS1 ;
    static const double TD_DIST_P0_CS2 ;
};

struct H1 : public IceModelTraits<H1> {
  static const std::string name() { return "H1"; }
    static const double TD_ABS;
    static const double TD_TAU ;
    static const double TD_LAM ;
    static const double TD_DIST_P1 ;
    static const double TD_DIST_P0_CS0 ;
    static const double TD_DIST_P0_CS1 ;
    static const double TD_DIST_P0_CS2 ;
};

struct H3 : public IceModelTraits<H3> {
  static const std::string name() { return "H3"; }
    static const double TD_ABS;
    static const double TD_TAU ;
    static const double TD_LAM ;
    static const double TD_DIST_P1 ;
    static const double TD_DIST_P0_CS0 ;
    static const double TD_DIST_P0_CS1 ;
    static const double TD_DIST_P0_CS2 ;
};

struct H4 : public IceModelTraits<H4> {
  static const std::string name() { return "H4"; }
    static const double TD_ABS;
    static const double TD_TAU ;
    static const double TD_LAM ;
    static const double TD_DIST_P1 ;
    static const double TD_DIST_P0_CS0 ;
    static const double TD_DIST_P0_CS1 ;
    static const double TD_DIST_P0_CS2 ;
};

template<typename HoleIce>
struct CascIce0 : public IceModelTraits<CascIce0<HoleIce> > {
  static const std::string name() {
      return std::string("CascIce0<")+HoleIce::name()+std::string(">");
  }
    static const double TD_ABS; 
    static const double TD_TAU;
    static const double TD_LAM;
    static const double TD_DIST_P1;
    static const double TD_DIST_P0_CS0;
    static const double TD_DIST_P0_CS1;
    static const double TD_DIST_P0_CS2;
};


struct UserDefIce {
    UserDefIce():name_("userice"){
        initialize(boost::make_shared<H2>());
    }
    template<typename HoleIce>
    UserDefIce(boost::shared_ptr<HoleIce> h,std::string name="userice"):name_(name){
        initialize(h);
    }
    template<typename HoleIce>
    void initialize(boost::shared_ptr<HoleIce> h){
        TD_ABS         = h->TD_ABS;
        TD_TAU         = h->TD_TAU;
        TD_LAM         = h->TD_LAM;
        TD_DIST_P1     = h->TD_DIST_P1;
        TD_DIST_P0_CS0 = h->TD_DIST_P0_CS0;
        TD_DIST_P0_CS1 = h->TD_DIST_P0_CS1;
        TD_DIST_P0_CS2 = h->TD_DIST_P0_CS2;
    }
    const std::string name() {
        return name_;
    }
    const std::string name_;
    double TauScale(double d=0.) const { return TD_TAU; }
    double AbsorptionLength(double d=0.) const { return TD_ABS; }
    double Absorptivity(double d=0.) const { return 1./TD_ABS; }
    double EffScattLength(double d=0.) const { return TD_LAM; }
    double InvEffScattLength(double d=0.) const { return 1./TD_LAM; }
    double TD_ABS; 
    double TD_TAU;
    double TD_LAM;
    double TD_DIST_P1;
    double TD_DIST_P0_CS0;
    double TD_DIST_P0_CS1;
    double TD_DIST_P0_CS2;
};

} // namespace Pandel
} // namespace IPDF


template<class HoleIce> const double IPDF::Pandel::CascIce0<HoleIce>::TD_ABS         = HoleIce::TD_ABS;
template<class HoleIce> const double IPDF::Pandel::CascIce0<HoleIce>::TD_TAU         = 450.0;
template<class HoleIce> const double IPDF::Pandel::CascIce0<HoleIce>::TD_LAM         = 47.0;
template<class HoleIce> const double IPDF::Pandel::CascIce0<HoleIce>::TD_DIST_P1     = HoleIce::TD_DIST_P1;
template<class HoleIce> const double IPDF::Pandel::CascIce0<HoleIce>::TD_DIST_P0_CS0 = HoleIce::TD_DIST_P0_CS0;
template<class HoleIce> const double IPDF::Pandel::CascIce0<HoleIce>::TD_DIST_P0_CS1 = HoleIce::TD_DIST_P0_CS1;
template<class HoleIce> const double IPDF::Pandel::CascIce0<HoleIce>::TD_DIST_P0_CS2 = HoleIce::TD_DIST_P0_CS2;

#endif // IPDF_IceModel_H
