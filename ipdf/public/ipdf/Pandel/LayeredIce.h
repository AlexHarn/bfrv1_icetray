#ifndef IPDF_LayeredIce_H
#define IPDF_LayeredIce_H

/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @file LayeredIce.h
    @version $Revision: 1.5 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>
*/

#include "ipdf/I3/IceTableInterpolator.h"
#include "ipdf/Pandel/IceModel.h"
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>


namespace IPDF {
namespace Pandel {

  /*
   * @brief Implementation of IceModel for depth dependent ice properties
   *
    This class is an interface to I3Medium, which provides the 
    depth dependent ice properties.  Currently The TauScale() and 
    propogationDistance() are provided by the HoleIce (template 
    parameter).

    Implicit copy ctor, op= and dtor are correct (boost::shared_ptr).
   */
template<typename HoleIce=IPDF::Pandel::H2>
class LayeredIce {
public:
  typedef LayeredIce<HoleIce> Self;
  typedef boost::shared_ptr<Self> pointer_type;
  
  explicit LayeredIce(const std::string& ice_file_path);

  static const std::string name() { return (std::string("LayeredIce<")+HoleIce::name()+">"); }

  /// @brief Pandel parameter relating to light propagation
  static double TauScale(double d=0.) { return HoleIce::TD_TAU; }
  /// @brief The Absorption length of light in ice
  double AbsorptionLength(double d=0.) const { return i3medium_->AbsorptionLength(d); }
  /// @brief The inverse of the absorption length
  double Absorptivity(double d=0.) const { return i3medium_->Absorptivity(d); }
  /// @brief The effective scattering length
  double EffScattLength(double d=0.) const { return i3medium_->EffScattLength(d); }
  /// @brief The inverse of the effective scattering length
  double InvEffScattLength(double d=0.) const { return i3medium_->InvEffScattLength(d); }

  /// @brief Pandel parameter relating to light propagation
  static double TauScale(double d1, double d2) { return HoleIce::TD_TAU; }
  /// @brief The Absorption length of light in ice
  ///
  /// This is the version accounting for receiver and emitter depth
  double AbsorptionLength(double d1, double d2) const
  { return i3medium_->AveragedAbsorptionLength(d1,d2); }
  /// @brief The inverse of the absorption length
  ///
  /// This is the version accounting for receiver and emitter depth
  double Absorptivity(double d1, double d2) const
  { return i3medium_->AveragedAbsorptivity(d1,d2); }
  /// @brief The effective scattering length
  ///
  /// This is the version accounting for receiver and emitter depth
  double EffScattLength(double d1, double d2) const
  { return i3medium_->AveragedEffScattLength(d1,d2); }
  /// @brief The inverse of the effective scattering length
  ///
  /// This is the version accounting for receiver and emitter depth
  double InvEffScattLength(double d1, double d2) const
  { return i3medium_->AveragedInvEffScattLength(d1,d2); }

  // aliases for python bindings convenience

  /// @brief The Absorption length of light in ice
  double AveragedAbsorptionLength(double d1, double d2) const { return i3medium_->AveragedAbsorptionLength(d1,d2); }
  /// @brief The inverse of the absorption length
  double AveragedAbsorptivity(double d1, double d2) const { return i3medium_->AveragedAbsorptivity(d1,d2); }
  /// @brief The effective scattering length
  double AveragedEffScattLength(double d1, double d2) const { return i3medium_->AveragedEffScattLength(d1,d2); }
  /// @brief The inverse of the effective scattering length
  double AveragedInvEffScattLength(double d1, double d2) const { return i3medium_->AveragedInvEffScattLength(d1,d2); }
  /// @brief The Absorption length of light in ice
  double LocalAbsorptionLength(double d) const { return i3medium_->AbsorptionLength(d); }
  /// @brief The inverse of the absorption length
  double LocalAbsorptivity(double d) const { return i3medium_->Absorptivity(d); }
  /// @brief The effective scattering length
  double LocalEffScattLength(double d) const { return i3medium_->EffScattLength(d); }
  /// @brief The inverse of the effective scattering length
  double LocalInvEffScattLength(double d) const { return i3medium_->InvEffScattLength(d); }

  static const double TD_DIST_P1;
  static const double TD_DIST_P0_CS0;
  static const double TD_DIST_P0_CS1;
  static const double TD_DIST_P0_CS2;

private:
  boost::shared_ptr<IceTableInterpolator> i3medium_;   /// Need boost::shared_ptr in order to support copy semantics
};

template<typename HoleIce>
LayeredIce<HoleIce>::LayeredIce(const std::string& ice_file)
 : i3medium_(boost::make_shared<IceTableInterpolator>(ice_file))
{
}

} // namespace Pandel
} // namespace IPDF


template<class HoleIce> const double IPDF::Pandel::LayeredIce<HoleIce>::TD_DIST_P1 = HoleIce::TD_DIST_P1;
template<class HoleIce> const double IPDF::Pandel::LayeredIce<HoleIce>::TD_DIST_P0_CS0 = HoleIce::TD_DIST_P0_CS0;
template<class HoleIce> const double IPDF::Pandel::LayeredIce<HoleIce>::TD_DIST_P0_CS1 = HoleIce::TD_DIST_P0_CS1;
template<class HoleIce> const double IPDF::Pandel::LayeredIce<HoleIce>::TD_DIST_P0_CS2 = HoleIce::TD_DIST_P0_CS2;


#endif // IPDF_LayeredIce_H
