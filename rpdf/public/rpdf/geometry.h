#ifndef INCLUDE_RPDF_GEOMETRY_H
#define INCLUDE_RPDF_GEOMETRY_H

/**
 * @brief functions for calculating geometrical parameters for muon reconstructions
 *
 * @copyright (C) 2018 The Icecube Collaboration
 *
 * @file geometry.h
 * @author Kevin Meagher
 * @date January 2018
 *
 */

#include <utility>
#include "dataclasses/I3Constants.h"
class I3Position;
class I3Particle;

namespace rpdf{

  namespace constants{
    ///Speed of light in vacuum, less precision than what is found in I3Constants
    ///because this is what ipdf did
    const double C_VACUUM=2.997924e-1;
    ///Group index of refraction of ice
    const double N_ICE_G=I3Constants::n_ice_group;
    ///group velocity in ice
    const double C_ICE_G = C_VACUUM/N_ICE_G;
    ///the sine of the Cherenkov angle
    const double SIN_CHERENKOV=sin(I3Constants::theta_cherenkov);
    ///the tangent of the Cherenkov angle
    const double TAN_CHERENKOV=tan(I3Constants::theta_cherenkov);
    ///a grouping of constants that is always the same in the calculation.
    ///It is called the effective tangent of the Cherenkov angle because
    ///It is that if the group and phase angles are the same
    const double EFF_TAN_CHERENKOV = N_ICE_G/SIN_CHERENKOV-1/TAN_CHERENKOV;
  }

  /**
   * This structure holds all the relevant parameters to describe the optical
   * properties of ice used for reconstruction
   */
  struct IceModel{
    ///The distance a photon can go before being absorbed on average
    double absorption_length;
    ///The pandel tau parameter, represents the non-linear behavior of
    ///multiple photon scatters
    double tau_scale;
    ///The distance a photon can go on average before being absorbed
    double scattering_length;
    /// The coefficient in front of the distance when calculating effective distance
    double P1;
    /// The constant coefficient when calculating effective distance
    double P0_CS0;
    /// The coefficient in front of cos\eta when calculating effective distance
    double P0_CS1;
    /// The coefficient in front of cos^2\eta when calculating effective distance
    double P0_CS2;
    /// rho is a combination of the other optical properties, it is the scale
    /// parameter in the Pandel function
    double rho;

    /**
     * IceModel is mostly a dumb struct, however the constructor computes
     * rho from the other parameters
     *
     * @param absorption_length The distance a photon can go before being
     *        absorbed on average
     * @param tau_scale The pandel tau parameter, represents the non-linear
     *        behavior of multiple photon scatters
     * @param scattering_length The distance a photon can go on average
     *        before being absorbed
     * @param P1 The coefficient in front of the distance when calculating
     *        effective distance
     * @param P0_CS0 The constant coefficient when calculating effective
     *        distance
     * @param P0_CS1 The coefficient in front of cos\eta when calculating
     *        effective distance
     * @param P0_CS2 The coefficient in front of cos^2\eta when calculating
     *        effective distance
     */
  IceModel(const double a,const double t, const double s,
           const double p1,const double p0c0,
           const double p0c1, const double p0c2):
    absorption_length(a),tau_scale(t),  scattering_length(s),
      P1(p1), P0_CS0(p0c0), P0_CS1(p0c1), P0_CS2(p0c2),
      rho(1/t+constants::C_ICE_G/a) {}
  };

  //forward declare the 5 standard ice models defined by AMANDA
  extern IceModel H0,H1,H2,H3,H4;

  /**
   * @brief Calculate the effective distance between an emitter and a DOM due to scattering.
   * This is a small correction for DOMs orientated away from the source.
   *
   * @param distance the actual distance traveled by the photon
   * @param cos_eta the cosine of the angle between the photon's trajectory and
   * the direction the DOM is pointing
   * @param ice_model the object holding the description of the ice
   *
   * @returns the equivalent distance that would have experienced the same amount of
   * scattering if the DOM was orientated toward the photon path
   */
  double effective_distance(const double distance,
                              const double cos_eta,
                              const IceModel& ice_model);

  /**
   * @brief Calculate the time Cherenkov photon arrives at a position d_track
   * along the track, to a DOM located d_approach from the track
   *
   * @param d_track The distance along the track from the vertex to the point of
   * closest approach
   * @param d_approach The distance of closest approach between the DOM and track
   *
   * @returns the time at which the Cherenkov photon arrives at the DOM
   */
  double cherenkov_time(const double d_track,const double d_approach);

  /**
   * @brief This calculates the two geometrical parameters needed by the pandel
   * function to calculate the PDF.
   *
   * Because the calculations relating to these function are very tightly
   * coupled they are both calculated in a single function
   *
   * @param om the position of the optical module being hit
   * @param track the muon track hypothesis
   * @param ice_model the model used to describe the optical properties of the ice
   *
   * @returns t_hit the difference in time between the hit and the expected time
   * of a direct photon
   * @returns effective_distance the effective distance between the track hypothesis
   * and the OM including scattering needed to hit an OM facing the other way
   */
  std::pair<double,double> muon_geometry(const I3Position& om,
                                         const I3Particle& track,
                                         const IceModel& ice_model);

}
#endif
